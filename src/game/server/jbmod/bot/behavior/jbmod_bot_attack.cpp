//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "jbmod_player.h"
#include "jbmod_gamerules.h"
#include "team_control_point_master.h"
#include "bot/jbmod_bot.h"
#include "bot/behavior/jbmod_bot_attack.h"

#include "nav_mesh.h"

extern ConVar jbmod_bot_path_lookahead_range;
extern ConVar jbmod_bot_offense_must_push_time;

ConVar jbmod_bot_aggressive( "jbmod_bot_aggressive", "0", FCVAR_NONE );

//---------------------------------------------------------------------------------------------
CJBModBotAttack::CJBModBotAttack( void ) : m_chasePath( ChasePath::LEAD_SUBJECT )
{
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotAttack::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
// head aiming and weapon firing is handled elsewhere - we just need to get into position to fight
ActionResult< CJBModBot >	CJBModBotAttack::Update( CJBModBot *me, float interval )
{
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	me->EquipBestWeaponForThreat( threat );

	if ( threat == NULL || threat->IsObsolete() || !me->GetIntentionInterface()->ShouldAttack( me, threat ) )
	{
		return Done( "No threat" );
	}

	if ( me->IsPropFreak() && me->Physcannon_GetHeldProp() == NULL )
	{
		// No prop? Oh no
		return Done( "Prop freak with no prop to throw!" );
	}

	CBaseJBModCombatWeapon* myWeapon = dynamic_cast< CBaseJBModCombatWeapon* >( me->GetActiveWeapon() );
	bool isUsingCloseRangeWeapon = me->IsCloseRange( myWeapon );
	if ( isUsingCloseRangeWeapon && threat->IsVisibleRecently() && me->IsRangeLessThan( threat->GetLastKnownPosition(), 1.1f * me->GetDesiredAttackRange() ) )
	{
		// circle around our victim
		if ( me->TransientlyConsistentRandomValue( 3.0f ) < 0.5f )
		{
			me->PressLeftButton();
		}
		else
		{
			me->PressRightButton();
		}
	}

	bool bHasRangedWeapon = me->IsRanged( myWeapon );

	// Go after them!
	bool bAggressive = jbmod_bot_aggressive.GetBool() &&
					   !bHasRangedWeapon &&
					   me->GetDifficulty() > CJBModBot::EASY;

	// pursue the threat. if not visible, go to the last known position
	if ( bAggressive ||
	     !threat->IsVisibleRecently() || 
		 me->IsRangeGreaterThan( threat->GetEntity()->GetAbsOrigin(), me->GetDesiredAttackRange() ) || 
		 !me->IsLineOfFireClear( threat->GetEntity()->EyePosition() ) )
	{
		if ( threat->IsVisibleRecently() )
		{
			if ( isUsingCloseRangeWeapon )
			{
				CJBModBotPathCost cost( me, FASTEST_ROUTE );
				m_chasePath.Update( me, threat->GetEntity(), cost );
			}
			else
			{
				CJBModBotPathCost cost( me, DEFAULT_ROUTE );
				m_chasePath.Update( me, threat->GetEntity(), cost );
			}
		}
		else
		{
			// if we're at the threat's last known position and he's still not visible, we lost him
			m_chasePath.Invalidate();

			if ( me->IsRangeLessThan( threat->GetLastKnownPosition(), 20.0f ) )
			{
				me->GetVisionInterface()->ForgetEntity( threat->GetEntity() );
				return Done( "I lost my target!" );
			}

			// look where we last saw him as we approach
			if ( me->IsRangeLessThan( threat->GetLastKnownPosition(), me->GetMaxAttackRange() ) )
			{
				me->GetBodyInterface()->AimHeadTowards( threat->GetLastKnownPosition() + Vector( 0, 0, HumanEyeHeight ), IBody::IMPORTANT, 0.2f, NULL, "Looking towards where we lost sight of our victim" );
			}

			m_path.Update( me );

			if ( m_repathTimer.IsElapsed() )
			{
				//m_repathTimer.Start( RandomFloat( 0.3f, 0.5f ) );
				m_repathTimer.Start( RandomFloat( 3.0f, 5.0f ) );

				if ( isUsingCloseRangeWeapon )
				{
					CJBModBotPathCost cost( me, FASTEST_ROUTE );
					m_path.Compute( me, threat->GetLastKnownPosition(), cost );
				}
				else
				{
					CJBModBotPathCost cost( me, DEFAULT_ROUTE );
					m_path.Compute( me, threat->GetLastKnownPosition(), cost );
				}
			}
		}
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotAttack::OnStuck( CJBModBot *me )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotAttack::OnMoveToSuccess( CJBModBot *me, const Path *path )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotAttack::OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
QueryResultType	CJBModBotAttack::ShouldRetreat( const INextBot *me ) const
{
	return ANSWER_UNDEFINED;
}


//---------------------------------------------------------------------------------------------
QueryResultType CJBModBotAttack::ShouldHurry( const INextBot *me ) const
{
	return ANSWER_UNDEFINED;
}

