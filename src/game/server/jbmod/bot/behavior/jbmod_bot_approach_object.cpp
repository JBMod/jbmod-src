//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "nav_mesh.h"
#include "jbmod_player.h"
#include "bot/jbmod_bot.h"
#include "bot/behavior/jbmod_bot_approach_object.h"

extern ConVar jbmod_bot_path_lookahead_range;

//---------------------------------------------------------------------------------------------
CJBModBotApproachObject::CJBModBotApproachObject( CBaseEntity *loot, float range )
{
	m_loot = loot;
	m_range = range;
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotApproachObject::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotApproachObject::Update( CJBModBot *me, float interval )
{
	if ( m_loot == NULL )
	{
		return Done( "Object is NULL" );
	}

	if ( m_loot->IsEffectActive( EF_NODRAW ) )
	{
		return Done( "Object is NODRAW" );
	}

	if ( me->GetLocomotionInterface()->GetGround() == m_loot )
	{
		return Done( "I'm standing on the object" );
	}

	if ( me->IsDistanceBetweenLessThan( m_loot->GetAbsOrigin(), m_range ) )
	{
		// in case we can't pick up the loot for some reason
		return Done( "Reached object" );
	}

	if ( m_repathTimer.IsElapsed() )
	{
		m_repathTimer.Start( RandomFloat( 1.0f, 2.0f ) );

		CJBModBotPathCost cost( me, FASTEST_ROUTE );
		m_path.Compute( me, m_loot->GetAbsOrigin(), cost );
	}

	// move to the loot
	m_path.Update( me );

	return Continue();
}


