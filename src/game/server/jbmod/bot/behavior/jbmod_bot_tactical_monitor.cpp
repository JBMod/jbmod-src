//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "fmtstr.h"

#include "jbmod_gamerules.h"
#include "jbmod/weapon_slam.h"
#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"

#include "bot/jbmod_bot.h"
#include "bot/jbmod_bot_manager.h"

#include "bot/behavior/jbmod_bot_tactical_monitor.h"
#include "bot/behavior/jbmod_bot_scenario_monitor.h"

#include "bot/behavior/jbmod_bot_seek_and_destroy.h"
#include "bot/behavior/jbmod_bot_retreat_to_cover.h"
#include "bot/behavior/jbmod_bot_get_health.h"
#include "bot/behavior/jbmod_bot_get_ammo.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_destroy_entity.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_move_to.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_wait.h"

extern ConVar jbmod_bot_health_ok_ratio;
extern ConVar jbmod_bot_health_critical_ratio;

ConVar jbmod_bot_force_jump( "jbmod_bot_force_jump", "0", FCVAR_CHEAT, "Force bots to continuously jump" );

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Attempts to kick/despawn the bot in the Update()

class CJBModDespawn : public Action< CJBModBot >
{
public:
	virtual ActionResult< CJBModBot >	Update( CJBModBot* me, float interval );
	virtual const char* GetName( void ) const { return "Despawn"; };
};


ActionResult< CJBModBot > CJBModDespawn::Update( CJBModBot* me, float interval )
{
	// players need to be kicked, not deleted
	if ( me->GetEntity()->IsPlayer() )
	{
		CBasePlayer* player = dynamic_cast< CBasePlayer* >( me->GetEntity() );
		engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", player->GetUserID() ) );
	}
	else
	{
		UTIL_Remove( me->GetEntity() );
	}
	return Continue();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Action< CJBModBot > *CJBModBotTacticalMonitor::InitialContainedAction( CJBModBot *me )
{
	return new CJBModBotScenarioMonitor;
}


//-----------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotTacticalMonitor::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	return Continue();
}


//-----------------------------------------------------------------------------------------
void CJBModBotTacticalMonitor::MonitorArmedStickyBombs( CJBModBot *me )
{
	if ( m_stickyBombCheckTimer.IsElapsed() )
	{
		m_stickyBombCheckTimer.Start( RandomFloat( 0.3f, 1.0f ) );

		// are there any enemies on/near my sticky bombs?
		CWeapon_SLAM *slam = dynamic_cast< CWeapon_SLAM* >( me->Weapon_OwnsThisType( "weapon_slam" ) );
		if ( slam )
		{
			const CUtlVector< CBaseEntity* > &satchelVector = slam->GetSatchelVector();

			if ( satchelVector.Count() > 0 )
			{
				CUtlVector< CKnownEntity > knownVector;
				me->GetVisionInterface()->CollectKnownEntities( &knownVector );

				for( int p=0; p< satchelVector.Count(); ++p )
				{
					CBaseEntity *satchel = satchelVector[p];
					if ( !satchel )
					{
						continue;
					}

					for( int k=0; k<knownVector.Count(); ++k )
					{
						if ( knownVector[k].IsObsolete() )
						{
							continue;
						}

						if ( knownVector[k].GetEntity()->IsBaseObject() )
						{
							// we want to put several stickies on a sentry and det at once
							continue;
						}

						if ( satchel->GetTeamNumber() != GetEnemyTeam( knownVector[k].GetEntity()->GetTeamNumber() ) )
						{
							// "known" is either a spectator, or on our team
							continue;
						}

						const float closeRange = 150.0f;
						if ( ( knownVector[k].GetLastKnownPosition() - satchel->GetAbsOrigin() ).IsLengthLessThan( closeRange ) )
						{
							// they are close - blow it!
							me->PressFireButton();
							return;
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------
void CJBModBotTacticalMonitor::AvoidBumpingEnemies( CJBModBot *me )
{
	if ( me->GetDifficulty() < CJBModBot::HARD )
		return;

	const float avoidRange = 200.0f;

	CUtlVector< CJBMod_Player * > enemyVector;
	CollectPlayers( &enemyVector, GetEnemyTeam( me->GetTeamNumber() ), COLLECT_ONLY_LIVING_PLAYERS );

	CJBMod_Player *closestEnemy = NULL;
	float closestRangeSq = avoidRange * avoidRange;

	for( int i=0; i<enemyVector.Count(); ++i )
	{
		CJBMod_Player *enemy = enemyVector[i];

		float rangeSq = ( enemy->GetAbsOrigin() - me->GetAbsOrigin() ).LengthSqr();
		if ( rangeSq < closestRangeSq )
		{
			closestEnemy = enemy;
			closestRangeSq = rangeSq;
		}
	}

	if ( !closestEnemy )
		return;

	// avoid unless hindrance returns a definitive "no"
	if ( me->GetIntentionInterface()->IsHindrance( me, closestEnemy ) == ANSWER_UNDEFINED )
	{
		me->ReleaseForwardButton();
		me->ReleaseLeftButton();
		me->ReleaseRightButton();
		me->ReleaseBackwardButton();

		Vector away = me->GetAbsOrigin() - closestEnemy->GetAbsOrigin();

		me->GetLocomotionInterface()->Approach( me->GetLocomotionInterface()->GetFeet() + away );
	}
}


//-----------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotTacticalMonitor::Update( CJBModBot *me, float interval )
{
	if ( jbmod_bot_force_jump.GetBool() )
	{
		if ( !me->GetLocomotionInterface()->IsClimbingOrJumping() )
		{
			me->GetLocomotionInterface()->Jump();
		}
	}

	const CKnownEntity* threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	me->EquipBestWeaponForThreat( threat );

	Action< CJBModBot > *result = me->OpportunisticallyUseWeaponAbilities();
	if ( result )
	{
		return SuspendFor( result, "Opportunistically using buff item" );
	}

	// check if we need to get to cover
	QueryResultType shouldRetreat = me->GetIntentionInterface()->ShouldRetreat( me );

	if ( shouldRetreat == ANSWER_YES )
	{
		return SuspendFor( new CJBModBotRetreatToCover, "Backing off" );
	}
	else if ( shouldRetreat != ANSWER_NO )
	{
		if ( !me->IsPropFreak() )
		{
			// retreat if we need to do a full reload (ie: soldiers shot all their rockets)
			if ( me->IsDifficulty( CJBModBot::HARD ) || me->IsDifficulty( CJBModBot::EXPERT ) )
			{
				CBaseJBModCombatWeapon *weapon = ( CBaseJBModCombatWeapon *) me->GetActiveWeapon();
				if ( weapon && me->GetAmmoCount( weapon->GetPrimaryAmmoType() ) > 0 && me->IsBarrageAndReloadWeapon( weapon ) )
				{
					if ( weapon->Clip1() <= 1 )
					{
						return SuspendFor( new CJBModBotRetreatToCover, "Moving to cover to reload" );
					}
				}
			}
		}
	}

	bool isAvailable = ( me->GetIntentionInterface()->ShouldHurry( me ) != ANSWER_YES );

	// collect ammo and health kits, unless we're in a big hurry
	if ( isAvailable )
	{
		if ( m_maintainTimer.IsElapsed() )
		{
			m_maintainTimer.Start( RandomFloat( 0.3f, 0.5f ) );

			bool isHurt = ( me->GetFlags() & FL_ONFIRE ) || ( ( float )me->GetHealth() / ( float )me->GetMaxHealth() ) < jbmod_bot_health_ok_ratio.GetFloat();

			if ( isHurt && CJBModBotGetHealth::IsPossible( me ) )
			{
				return SuspendFor( new CJBModBotGetHealth, "Grabbing nearby health" );
			}

			// Prop freaks go for props instead of ammo.
			if ( !me->IsPropFreak() )
			{
				if ( me->IsAmmoLow() && CJBModBotGetAmmo::IsPossible( me ) )
				{
					return SuspendFor( new CJBModBotGetAmmo, "Grabbing nearby ammo" );
				}
			}
		}
	}

	// detonate sticky bomb traps when victims are near
	MonitorArmedStickyBombs( me );

	me->UpdateDelayedThreatNotices();

	return Continue();
}


//-----------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotTacticalMonitor::OnOtherKilled( CJBModBot *me, CBaseCombatCharacter *victim, const CTakeDamageInfo &info )
{
	return TryContinue();
}


//-----------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotTacticalMonitor::OnNavAreaChanged( CJBModBot *me, CNavArea *newArea, CNavArea *oldArea )
{
	return TryContinue();
}

//-----------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotTacticalMonitor::OnCommandString( CJBModBot *me, const char *command )
{
	if ( FStrEq( command, "despawn" ) )
	{
		return TrySuspendFor( new CJBModDespawn(), RESULT_CRITICAL, "Received command to go to de-spawn" );
	}

	return TryContinue();
}
