//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "jbmod_gamerules.h"
#include "bot/jbmod_bot.h"
#include "bot/behavior/jbmod_bot_get_prop.h"
#include "jbmod/weapon_physcannon.h"
#include "props.h"

extern ConVar jbmod_bot_path_lookahead_range;

ConVar jbmod_bot_prop_search_range( "jbmod_bot_prop_search_range", "2000", FCVAR_CHEAT, "How far prop freak bots will search to find props around them" );
ConVar jbmod_bot_prop_search_important_range( "jbmod_bot_prop_search_important_range", "400", FCVAR_CHEAT, "How far prop freak bots will search to find and prefer important props around them (like explosive barrels)" );
ConVar jbmod_bot_prop_search_min_mass( "jbmod_bot_prop_search_min_mass", "30", FCVAR_CHEAT, "Ignore props under this mass" );
ConVar jbmod_bot_prop_search_only_important( "jbmod_bot_prop_search_only_important", "0", FCVAR_CHEAT );
ConVar jbmod_bot_debug_prop_scavenging( "jbmod_bot_debug_prop_scavenging", "0", FCVAR_CHEAT );
ConVar jbmod_bot_disable_get_prop( "jbmod_bot_disable_get_prop", "0", FCVAR_CHEAT );


//---------------------------------------------------------------------------------------------
CJBModBotGetProp::CJBModBotGetProp( void )
{
	m_path.Invalidate();
	m_prop = NULL;
}

CJBModBotGetProp::~CJBModBotGetProp( void )
{
	Assert( !m_pushedPhyscannon );
}


//---------------------------------------------------------------------------------------------
template <bool bOnlyImportant>
class CPropFilter : public INextBotFilter
{
public:
	CPropFilter( CJBModBot *me, CWeaponPhysCannon *pPhyscannon )
	{
		m_me = me;
		m_physcannon = pPhyscannon;
	}

	bool IsSelected( const CBaseEntity *constCandidate ) const
	{
		if ( !constCandidate )
			return false;

		CBaseEntity *candidate = const_cast< CBaseEntity * >( constCandidate );

		if ( !bOnlyImportant )
		{
			CClosestJBModPlayer close( candidate );
			ForEachPlayer( close );

			// if the closest player to this candidate object is an enemy, don't use it
			if ( close.m_closePlayer && m_me->IsEnemy( close.m_closePlayer ) )
				return false;
		}

		// ignore non-existent props to ensure we collect nearby existing props
		if ( candidate->IsEffectActive( EF_NODRAW ) )
			return false;

		CPhysicsProp* pProp = dynamic_cast< CPhysicsProp* >( candidate );
		if ( !pProp )
			return false;

		bool bIsImportant = pProp->GetExplosiveDamage() > 0 || pProp->GetExplosiveRadius() > 0;

		if ( bOnlyImportant )
		{
			// Ignore non-explosive props.
			if ( !bIsImportant )
			{
				return false;
			}
		}
		else
		{
			// Not worth it.
			if ( pProp->GetMass() < jbmod_bot_prop_search_min_mass.GetFloat() && !bIsImportant )
				return false;
		}

		if ( !bIsImportant && jbmod_bot_prop_search_only_important.GetBool() )
			return false;

		if ( !m_physcannon->CanPickupObject( pProp ) )
			return false;

		return true;
	}

	CJBModBot *m_me;
	CWeaponPhysCannon *m_physcannon;
};


//---------------------------------------------------------------------------------------------
static CJBModBot *s_possibleBot = NULL;
static CHandle< CBaseEntity > s_possibleProp = NULL;
static int s_possibleFrame = 0;


//---------------------------------------------------------------------------------------------
/**
 * Return true if this Action has what it needs to perform right now
 */
bool CJBModBotGetProp::IsPossible( CJBModBot *me )
{
	VPROF_BUDGET( "CJBModBotGetProp::IsPossible", "NextBot" );

	if ( jbmod_bot_disable_get_prop.GetBool() )
		return false;

	CWeaponPhysCannon* pPhyscannon = dynamic_cast< CWeaponPhysCannon* >( me->Weapon_OwnsThisType( "weapon_physcannon" ) );
	if ( !pPhyscannon )
		return false;

	if ( me->Physcannon_GetHeldProp() != NULL )
		return false;

	const CKnownEntity* threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	// Scared!
	if ( threat && threat->WasEverVisible() && threat->GetTimeSinceLastSeen() > ( me->IsPropFreak() ? 2.0f : 5.0f ) )
		return false;

	CBaseEntity* prop = NULL;
	CUtlVector< CHandle< CBaseEntity > > hPhysProps;
	while( ( prop = gEntList.FindEntityByClassname( prop, "prop_physics*" ) ) != NULL )
	{
		hPhysProps.AddToTail( prop );
	}

	CPropFilter<false> propFilter( me, pPhyscannon );
	CUtlVector< CHandle< CBaseEntity > > hReachablePhysProps;
	me->SelectReachableObjects( hPhysProps, &hReachablePhysProps, propFilter, me->GetLastKnownArea(), jbmod_bot_prop_search_range.GetFloat() );

	CPropFilter<true> importantFilter( me, pPhyscannon );
	CUtlVector< CHandle< CBaseEntity > > hReachableImportantProps;
	me->SelectReachableObjects( hPhysProps, &hReachableImportantProps, importantFilter, me->GetLastKnownArea(), jbmod_bot_prop_search_important_range.GetFloat() );

	CBaseEntity* closestProp = hReachablePhysProps.Size() > 0 ? hReachablePhysProps[0] : NULL;

	if ( hReachableImportantProps.Size() )
	{
		closestProp = hReachableImportantProps[0];
	}

	if ( !closestProp )
	{
		if ( me->IsDebugging( NEXTBOT_BEHAVIOR ) )
		{
			Warning( "%3.2f: No prop nearby\n", gpGlobals->curtime );
		}
		return false;
	}

	PathFollower path;
	CJBModBotPathCost cost( me, FASTEST_ROUTE );
	path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );
	if ( !path.Compute( me, closestProp->WorldSpaceCenter(), cost ) )
	{
		Warning( "%3.2f: We found a 'reachable' prop that wasn't reachable O_O\n", gpGlobals->curtime );
		return false;
	}

	if ( jbmod_bot_debug_prop_scavenging.GetBool() )
	{
		NDebugOverlay::Cross3D( closestProp->WorldSpaceCenter(), 5.0f, 255, 255, 0, true, 999.9 );
	}

	s_possibleBot = me;
	s_possibleProp = closestProp;
	s_possibleFrame = gpGlobals->framecount;

	return true;
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotGetProp::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	VPROF_BUDGET( "CJBModBotGetProp::OnStart", "NextBot" );

	if ( jbmod_bot_disable_get_prop.GetBool() )
		return Done( "Disabled." );

	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	// if IsPossible() has already been called, use its cached data
	if ( s_possibleFrame != gpGlobals->framecount || s_possibleBot != me )
	{
		if ( !IsPossible( me ) || s_possibleProp == NULL )
		{
			return Done( "Can't get prop" );
		}
	}

	m_prop = s_possibleProp;

	CJBModBotPathCost cost( me, FASTEST_ROUTE );
	if ( !m_path.Compute( me, m_prop->WorldSpaceCenter(), cost ) || !m_path.IsValid() || m_path.GetResult() != Path::COMPLETE_PATH )
	{
		return Done( "No path to prop!" );
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotGetProp::Update( CJBModBot *me, float interval )
{
	const CKnownEntity* threat = me->GetVisionInterface()->GetPrimaryKnownThreat();

	if ( jbmod_bot_disable_get_prop.GetBool() )
		return Done( "Disabled." );

	if ( me->Physcannon_GetHeldProp() != NULL )
	{
		me->MarkPhyscannonPickupTime();
		return Done( "I picked up a prop! :D" );
	}

	// If we have a threat, nope out of here.
	if ( threat && threat->WasEverVisible() && threat->GetTimeSinceLastSeen() > ( me->IsPropFreak() ? 2.0f : 5.0f ) )
	{
		return Done( "I'm scared!" );
	}

	CWeaponPhysCannon* pPhyscannon = dynamic_cast< CWeaponPhysCannon* >( me->Weapon_OwnsThisType( "weapon_physcannon" ) );
	if ( pPhyscannon == NULL )
	{
		return Done( "I don't have a physcannon anymore!" );
	}

	if ( m_prop == NULL )
	{
		return Done( "Prop I was going for is gone!" );
	}

	if ( !pPhyscannon->CanPickupObject( m_prop ) )
	{
		return Done( "I can't pick up this prop anymore!" );
	}

	if ( !m_path.IsValid() )
	{
		return Done( "My path became invalid" );
	}

	if ( me->IsRangeLessThan( m_prop, 100.0f ) )
	{
		// misyl: Push physcannon onto the required weapon stack when getting props.
		if ( !m_pushedPhyscannon )
		{
			me->PushRequiredWeapon( pPhyscannon );
			m_pushedPhyscannon = true;
		}

		// may need to switch weapons to pick up a prop.
		me->EquipBestWeaponForThreat( threat );
	}

	m_path.Update( me );

	if ( me->IsRangeLessThan( m_prop, 80.0f ) )
	{
		if ( me->GetGroundEntity() == m_prop )
		{
			// Jump if we are on top of it so we can pick it up!
			me->PressJumpButton();
		}

		me->GetBodyInterface()->AimHeadTowards( m_prop, IBody::MANDATORY, 0.1f, NULL, "Looking towards our desired prop" );

		me->PressAltFireButton( 1.0f );
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotGetProp::OnSuspend( CJBModBot* me, Action< CJBModBot >* interruptingAction )
{
	if ( m_pushedPhyscannon )
	{
		me->PopRequiredWeapon();
		m_pushedPhyscannon = false;

		me->ReleaseAltFireButton();

		const CKnownEntity* threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
		// may need to switch weapons from physcannon
		me->EquipBestWeaponForThreat( threat );
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotGetProp::OnResume( CJBModBot* me, Action< CJBModBot >* interruptingAction )
{
	return Continue();
}


//---------------------------------------------------------------------------------------------
void CJBModBotGetProp::OnEnd( CJBModBot* me, Action< CJBModBot >* nextAction )
{
	if ( m_pushedPhyscannon )
	{
		me->PopRequiredWeapon();
		m_pushedPhyscannon = false;

		me->ReleaseAltFireButton();

		const CKnownEntity* threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
		// may need to switch weapons from physcannon
		me->EquipBestWeaponForThreat( threat );
	}
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetProp::OnContact( CJBModBot *me, CBaseEntity *other, CGameTrace *result )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetProp::OnStuck( CJBModBot *me )
{
	return TryDone( RESULT_CRITICAL, "Stuck trying to reach prop" );
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetProp::OnMoveToSuccess( CJBModBot *me, const Path *path )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CJBModBot > CJBModBotGetProp::OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason )
{
	return TryDone( RESULT_CRITICAL, "Failed to reach prop" );
}


//---------------------------------------------------------------------------------------------
QueryResultType CJBModBotGetProp::ShouldHurry( const INextBot *me ) const
{
	// hustle over to the nearest prop
	return ANSWER_YES;
}
