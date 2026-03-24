//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "fmtstr.h"

#include "jbmod_gamerules.h"
#include "jbmod/weapon_slam.h"
#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"

#include "bot/jbmod_bot.h"
#include "bot/jbmod_bot_manager.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_destroy_entity.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_move_to.h"
#include "bot/behavior/nav_entities/jbmod_bot_nav_ent_wait.h"
#include "bot/behavior/jbmod_bot_tactical_monitor.h"
#include "bot/behavior/jbmod_bot_retreat_to_cover.h"
#include "bot/behavior/jbmod_bot_get_health.h"
#include "bot/behavior/jbmod_bot_get_ammo.h"

#include "bot/behavior/jbmod_bot_attack.h"
#include "bot/behavior/jbmod_bot_seek_and_destroy.h"

#include "bot/behavior/jbmod_bot_scenario_monitor.h"


extern ConVar jbmod_bot_health_ok_ratio;
extern ConVar jbmod_bot_health_critical_ratio;


//-----------------------------------------------------------------------------------------
// Returns the initial Action we will run concurrently as a child to us
Action< CJBModBot > *CJBModBotScenarioMonitor::InitialContainedAction( CJBModBot *me )
{
	if ( me->IsInASquad() )
	{
		if ( me->GetSquad()->IsLeader( me ) )
		{
			// I'm the leader of this Squad, so I can do what I want and the other Squaddies will support me
			return DesiredScenarioAndClassAction( me );
		}

		// I'm in a Squad but not the leader, do "escort and support" Squad behavior
		// until the Squad disbands, and then do my normal thing
		//
		// TODO: Implement this if we ever want squads in HL2MP.
		// It's like, an MVM thing, not really useful for us.
		//return new CJBModBotEscortSquadLeader( DesiredScenarioAndClassAction( me ) );
	}

	return DesiredScenarioAndClassAction( me );
}


//-----------------------------------------------------------------------------------------
// Returns Action specific to the scenario and my class
Action< CJBModBot > *CJBModBotScenarioMonitor::DesiredScenarioAndClassAction( CJBModBot *me )
{
	return new CJBModBotSeekAndDestroy;
}


//-----------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotScenarioMonitor::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	m_ignoreLostFlagTimer.Start( 20.0f );
	m_lostFlagTimer.Invalidate();
	return Continue();
}


ConVar jbmod_bot_fetch_lost_flag_time( "jbmod_bot_fetch_lost_flag_time", "10", FCVAR_CHEAT, "How long busy JBModBots will ignore the dropped flag before they give up what they are doing and go after it" );
ConVar jbmod_bot_flag_kill_on_touch( "jbmod_bot_flag_kill_on_touch", "0", FCVAR_CHEAT, "If nonzero, any bot that picks up the flag dies. For testing." );


//-----------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotScenarioMonitor::Update( CJBModBot *me, float interval )
{
	return Continue();
}

