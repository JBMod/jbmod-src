//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "jbmod_gamerules.h"
#include "bot/jbmod_bot.h"
#include "item_healthkit.h"
#include "bot/behavior/jbmod_bot_get_health.h"

extern ConVar jbmod_bot_path_lookahead_range;

ConVar jbmod_bot_health_critical_ratio( "jbmod_bot_health_critical_ratio", "0.3", FCVAR_CHEAT );
ConVar jbmod_bot_health_ok_ratio( "jbmod_bot_health_ok_ratio", "0.65", FCVAR_CHEAT );
ConVar jbmod_bot_health_search_near_range( "jbmod_bot_health_search_near_range", "1000", FCVAR_CHEAT );
ConVar jbmod_bot_health_search_far_range( "jbmod_bot_health_search_far_range", "2000", FCVAR_CHEAT );

ConVar jbmod_bot_debug_health_scavenging( "jbmod_bot_debug_ammo_scavenging", "0", FCVAR_CHEAT );

//---------------------------------------------------------------------------------------------
class CHealthFilter : public INextBotFilter
{
public:
	CHealthFilter( CJBModBot *me )
	{
		m_me = me;
	}

	bool IsSelected( const CBaseEntity *constCandidate ) const
	{
		if ( !constCandidate )
			return false;

		CBaseEntity *candidate = const_cast< CBaseEntity * >( constCandidate );

		CClosestJBModPlayer close( candidate );
		ForEachPlayer( close );

		// if the closest player to this candidate object is an enemy, don't use it
		if ( close.m_closePlayer && m_me->IsEnemy( close.m_closePlayer ) )
			return false;

		// ignore non-existent ammo to ensure we collect nearby existing ammo
		if ( candidate->IsEffectActive( EF_NODRAW ) )
			return false;

		if ( candidate->ClassMatches( "item_healthkit" ) )
			return true;

		if ( candidate->ClassMatches( "item_healthvial" ) )
			return true;

		if ( candidate->ClassMatches( "item_healthcharger" ) )
			return true;

		if ( candidate->ClassMatches( "func_healthcharger" ) )
			return true;

		return false;
	}

	CJBModBot *m_me;
};


//---------------------------------------------------------------------------------------------
static CJBModBot *s_possibleBot = NULL;
static CHandle< CBaseEntity > s_possibleHealth = NULL;
static int s_possibleFrame = 0;


//---------------------------------------------------------------------------------------------
/** 
 * Return true if this Action has what it needs to perform right now
 */
bool CJBModBotGetHealth::IsPossible( CJBModBot *me )
{
	VPROF_BUDGET( "CJBModBotGetHealth::IsPossible", "NextBot" );

	float healthRatio = (float)me->GetHealth() / (float)me->GetMaxHealth();

	float t = ( healthRatio - jbmod_bot_health_critical_ratio.GetFloat() ) / ( jbmod_bot_health_ok_ratio.GetFloat() - jbmod_bot_health_critical_ratio.GetFloat() );
	t = clamp( t, 0.0f, 1.0f );

	if ( me->GetFlags() & FL_ONFIRE )
	{
		// on fire - get health now
		t = 0.0f;
	}

	// the more we are hurt, the farther we'll travel to get health
	float searchRange = jbmod_bot_health_search_far_range.GetFloat() + t * ( jbmod_bot_health_search_near_range.GetFloat() - jbmod_bot_health_search_far_range.GetFloat() );

	CBaseEntity* healthkit = NULL;
	CUtlVector< CHandle< CBaseEntity > > hHealthKits;
	while ( ( healthkit = gEntList.FindEntityByClassname( healthkit, "*_health*" ) ) != NULL )
	{
		hHealthKits.AddToTail( healthkit );
	}

	CHealthFilter healthFilter( me );
	CUtlVector< CHandle< CBaseEntity > > hReachableHealthKits;
	me->SelectReachableObjects( hHealthKits, &hReachableHealthKits, healthFilter, me->GetLastKnownArea(), searchRange );

	CBaseEntity* closestHealth = hReachableHealthKits.Size() > 0 ? hReachableHealthKits[0] : NULL;

	if ( !closestHealth )
	{
		if ( me->IsDebugging( NEXTBOT_BEHAVIOR ) )
		{
			Warning( "%3.2f: No health nearby\n", gpGlobals->curtime );
		}
		return false;
	}

	CJBModBotPathCost cost( me, FASTEST_ROUTE );
	PathFollower path;
	if ( !path.Compute( me, closestHealth->WorldSpaceCenter(), cost ) || !path.IsValid() || path.GetResult() != Path::COMPLETE_PATH )
	{
		if ( me->IsDebugging( NEXTBOT_BEHAVIOR ) )
		{
			Warning( "%3.2f: No path to health!\n", gpGlobals->curtime );
		}
		return false;
	}

	s_possibleBot = me;
	s_possibleHealth = closestHealth;
	s_possibleFrame = gpGlobals->framecount;

	return true;
}

//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotGetHealth::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	VPROF_BUDGET( "CJBModBotGetHealth::OnStart", "NextBot" );

	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	// if IsPossible() has already been called, use its cached data
	if ( s_possibleFrame != gpGlobals->framecount || s_possibleBot != me )
	{
		if ( !IsPossible( me ) || s_possibleHealth == NULL )
		{
			return Done( "Can't get health" );
		}
	}

	m_healthKit = s_possibleHealth;
	m_isGoalCharger = m_healthKit->ClassMatches( "*charger*" );

	CJBModBotPathCost cost( me, SAFEST_ROUTE );
	if ( !m_path.Compute( me, m_healthKit->WorldSpaceCenter(), cost ) )
	{
		return Done( "No path to health!" );
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotGetHealth::Update( CJBModBot *me, float interval )
{
	if ( m_healthKit == NULL || ( m_healthKit->IsEffectActive( EF_NODRAW ) ) )
	{
		return Done( "Health kit I was going for has been taken" );
	}

	if ( me->GetHealth() >= me->GetMaxHealth() )
	{
		return Done( "I've been healed" );
	}

	if ( JBModRules()->IsTeamplay() )
	{
		// if the closest player to the item we're after is an enemy, give up

		CClosestJBModPlayer close( m_healthKit );
		ForEachPlayer( close );
		if ( close.m_closePlayer && me->IsEnemy( close.m_closePlayer ) )
			return Done( "An enemy is closer to it" );
	}

	if ( m_isGoalCharger )
	{
		// we need to get near and wait, not try to run over
		const float nearRange = 50.0f;
		if ( ( me->GetAbsOrigin() - m_healthKit->GetAbsOrigin() ).IsLengthLessThan( nearRange ) )
		{
			if ( me->GetVisionInterface()->IsLineOfSightClearToEntity( m_healthKit ) )
			{
				if ( me->GetHealth() == me->GetMaxHealth() )
				{
					return Done( "Health refilled by the Charger" );
				}

				CNewWallHealth* pNewWallHealth = dynamic_cast< CNewWallHealth* >( m_healthKit.Get() );
				if ( pNewWallHealth )
				{
					pNewWallHealth->Use( me, me, USE_ON, 0.0f );

					if ( pNewWallHealth->GetJuice() == 0 )
						return Done( "Charger is out of juice!" );
				}

				CWallHealth* pWallHealth = dynamic_cast< CWallHealth* >( m_healthKit.Get() );
				if ( pWallHealth )
				{
					pWallHealth->Use( me, me, USE_ON, 0.0f );

					if ( pWallHealth->GetJuice() == 0 )
						return Done( "Charger is out of juice!" );
				}

				float healthRatio = ( float )me->GetHealth() / ( float )me->GetMaxHealth();
				bool bLowHealth = healthRatio > jbmod_bot_health_critical_ratio.GetFloat();

				// don't wait if I'm in combat
				if ( !bLowHealth && me->GetVisionInterface()->GetPrimaryKnownThreat() )
				{
					return Done( "No time to wait for more health, I must fight" );
				}

				// wait until the charger refills us
				return Continue();
			}
		}
	}

	if ( !m_path.IsValid() )
	{
		// this can occur if we overshoot the health kit's location
		// because it is momentarily gone
		CJBModBotPathCost cost( me, SAFEST_ROUTE );
		if ( !m_path.Compute( me, m_healthKit->WorldSpaceCenter(), cost ) )
		{
			return Done( "No path to health!" );
		}
	}

	m_path.Update( me );

	// may need to switch weapons (ie: engineer holding toolbox now needs to heal and defend himself)
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	me->EquipBestWeaponForThreat( threat );

	return Continue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetHealth::OnStuck( CJBModBot *me )
{
	return TryDone( RESULT_CRITICAL, "Stuck trying to reach health kit" );
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetHealth::OnMoveToSuccess( CJBModBot *me, const Path *path )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetHealth::OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason )
{
	return TryDone( RESULT_CRITICAL, "Failed to reach health kit" );
}


//---------------------------------------------------------------------------------------------
// We are always hurrying if we need to collect health
QueryResultType CJBModBotGetHealth::ShouldHurry( const INextBot *me ) const
{
	return ANSWER_YES;
}
