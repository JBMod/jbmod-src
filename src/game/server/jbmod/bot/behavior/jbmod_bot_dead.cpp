//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "jbmod_player.h"
#include "jbmod_gamerules.h"
#include "bot/jbmod_bot.h"
#include "bot/behavior/jbmod_bot_dead.h"
#include "bot/behavior/jbmod_bot_behavior.h"

#include "nav_mesh.h"

extern void respawn( CBaseEntity* pEdict, bool fCopyCorpse );

//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotDead::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	m_deadTimer.Start();

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotDead::Update( CJBModBot *me, float interval )
{
	if ( me->IsAlive() )
	{
		// how did this happen?
		return ChangeTo( new CJBModBotMainAction, "This should not happen!" );
	}

	if ( m_deadTimer.IsGreaterThen( 5.0f ) )
	{
		if ( me->HasAttribute( CJBModBot::REMOVE_ON_DEATH ) )
		{
			// remove dead bots
			engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", me->GetUserID() ) );
		}
		else if ( me->HasAttribute( CJBModBot::BECOME_SPECTATOR_ON_DEATH ) )
		{
			me->ChangeTeam( TEAM_SPECTATOR );
			return Done();
		}
	}

	// I want to respawn!
	// Some gentle massaging for bots to get unstuck if they are holding keys, etc.
	if ( me->m_lifeState == LIFE_DEAD || me->m_lifeState == LIFE_RESPAWNABLE )
	{
		if ( g_pGameRules->FPlayerCanRespawn( me ) )
		{
			respawn( me, !me->IsObserver() );
		}
	}

	return Continue();
}

