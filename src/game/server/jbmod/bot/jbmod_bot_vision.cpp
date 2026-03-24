//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "vprof.h"

#include "jbmod_bot.h"
#include "jbmod_bot_vision.h"
#include "jbmod_player.h"
#include "jbmod_gamerules.h"

ConVar jbmod_bot_choose_target_interval( "jbmod_bot_choose_target_interval", "0.3f", FCVAR_CHEAT, "How often, in seconds, a JBModBot can reselect his target" );
ConVar jbmod_bot_sniper_choose_target_interval( "jbmod_bot_sniper_choose_target_interval", "3.0f", FCVAR_CHEAT, "How often, in seconds, a zoomed-in Sniper can reselect his target" );

extern ConVar jbmod_bot_ignore_real_players;

//------------------------------------------------------------------------------------------
void CJBModBotVision::CollectPotentiallyVisibleEntities( CUtlVector< CBaseEntity* >* potentiallyVisible )
{
	VPROF_BUDGET( "CJBModBotVision::CollectPotentiallyVisibleEntities", "NextBot" );

	potentiallyVisible->RemoveAll();

	// include all players
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer* player = UTIL_PlayerByIndex( i );

		if ( player == NULL )
			continue;

		if ( FNullEnt( player->edict() ) )
			continue;

		if ( !player->IsPlayer() )
			continue;

		if ( !player->IsConnected() )
			continue;

		if ( !player->IsAlive() )
			continue;

		if ( jbmod_bot_ignore_real_players.GetBool() )
		{
			if ( !player->IsBot() )
				continue;
		}

		potentiallyVisible->AddToTail( player );
	}

	// include sentry guns
	UpdatePotentiallyVisibleNPCVector();

	FOR_EACH_VEC( m_potentiallyVisibleNPCVector, it )
	{
		potentiallyVisible->AddToTail( m_potentiallyVisibleNPCVector[it] );
	}
}


//------------------------------------------------------------------------------------------
void CJBModBotVision::UpdatePotentiallyVisibleNPCVector( void )
{
	if ( m_potentiallyVisibleUpdateTimer.IsElapsed() )
	{
		m_potentiallyVisibleUpdateTimer.Start( RandomFloat( 3.0f, 4.0f ) );

		// collect list of active buildings
		m_potentiallyVisibleNPCVector.RemoveAll();

		CUtlVector< INextBot* > botVector;
		TheNextBots().CollectAllBots( &botVector );
		for ( int i = 0; i < botVector.Count(); ++i )
		{
			CBaseCombatCharacter* botEntity = botVector[i]->GetEntity();
			if ( botEntity && !botEntity->IsPlayer() )
			{
				// NPC
				m_potentiallyVisibleNPCVector.AddToTail( botEntity );
			}
		}
	}
}


//------------------------------------------------------------------------------------------
/**
 * Return true to completely ignore this entity.
 * This is mostly for enemy spies.  If we don't ignore them, we will look at them.
 */
bool CJBModBotVision::IsIgnored( CBaseEntity* subject ) const
{
	CJBModBot* me = ( CJBModBot* )GetBot()->GetEntity();

	if ( me->IsAttentionFocused() )
	{
		// our attention is restricted to certain subjects
		if ( !me->IsAttentionFocusedOn( subject ) )
		{
			return false;
		}
	}

	if ( !me->IsEnemy( subject ) )
	{
		// don't ignore friends
		return false;
	}

	if ( subject->IsEffectActive( EF_NODRAW ) )
	{
		return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
// Return VISUAL reaction time
float CJBModBotVision::GetMinRecognizeTime( void ) const
{
	CJBModBot* me = ( CJBModBot* )GetBot();

	switch ( me->GetDifficulty() )
	{
	case CJBModBot::EASY:	return 1.0f;
	case CJBModBot::NORMAL:	return 0.5f;
	case CJBModBot::HARD:	return 0.3f;
	case CJBModBot::EXPERT:	return 0.2f;
	}

	return 1.0f;
}



//------------------------------------------------------------------------------------------
float CJBModBotVision::GetMaxVisionRange( void ) const
{
	CJBModBot *me = (CJBModBot *)GetBot();

	if ( me->GetMaxVisionRangeOverride() > 0.0f )
	{
		// designer specified vision range
		return me->GetMaxVisionRangeOverride();
	}

	// long range, particularly for snipers
	return 6000.0f;
}
