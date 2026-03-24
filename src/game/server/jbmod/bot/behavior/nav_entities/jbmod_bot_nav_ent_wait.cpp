//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "nav_mesh.h"
#include "jbmod_player.h"
#include "bot/jbmod_bot.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_wait.h"

extern ConVar jbmod_bot_path_lookahead_range;

//---------------------------------------------------------------------------------------------
CJBModBotNavEntWait::CJBModBotNavEntWait( const CFuncNavPrerequisite *prereq )
{
	m_prereq = prereq;
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot > CJBModBotNavEntWait::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	if ( m_prereq == NULL )
	{
		return Done( "Prerequisite has been removed before we started" );
	}

	m_timer.Start( m_prereq->GetTaskValue() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot > CJBModBotNavEntWait::Update( CJBModBot *me, float interval )
{
	if ( m_prereq == NULL )
	{
		return Done( "Prerequisite has been removed" );
	}

	if ( !m_prereq->IsEnabled() )
	{
		return Done( "Prerequisite has been disabled" );
	}

	if ( m_timer.IsElapsed() )
	{
		return Done( "Wait time elapsed" );
	}

	return Continue();
}


