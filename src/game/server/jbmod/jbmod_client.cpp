//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "jbmod_player.h"
#include "jbmod_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "entitylist.h"
#include "physics.h"
#include "game.h"
#include "player_resource.h"
#include "engine/IEngineSound.h"
#include "team.h"
#include "viewport_panel_names.h"
#include "vscript_shared.h"
#include "filesystem.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Host_Say( edict_t *pEdict, bool teamonly );

extern CBaseEntity*	FindPickerEntityClass( CBasePlayer *pPlayer, char *classname );
extern bool			g_fGameOver;

void FinishClientPutInServer( CJBMod_Player *pPlayer )
{
	pPlayer->InitialSpawn();
	pPlayer->Spawn();

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerConnect" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL, pPlayer->GetScriptInstance() );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}
}

/*
===========
ClientPutInServer

called each time a player is spawned into the game
============
*/
void ClientPutInServer( edict_t *pEdict, const char *playername )
{
	CJBMod_Player *pPlayer = CJBMod_Player::CreatePlayer( "player", pEdict );
	pPlayer->SetPlayerName( playername );
}


void ClientActive( edict_t *pEdict, bool bLoadGame )
{
	// Can't load games!
	Assert( !bLoadGame );

	CJBMod_Player *pPlayer = ToJBModPlayer( CBaseEntity::Instance( pEdict ) );
	FinishClientPutInServer( pPlayer );
}


/*
===============
const char *GetGameDescription()

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "JBMod";
}

//-----------------------------------------------------------------------------
// Purpose: Given a player and optional name returns the entity of that 
//			classname that the player is nearest facing
//			
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseEntity* FindEntity( edict_t *pEdict, char *classname)
{
	// If no name was given set bits based on the picked
	if (FStrEq(classname,"")) 
	{
		return (FindPickerEntityClass( static_cast<CBasePlayer*>(GetContainingEntity(pEdict)), classname ));
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
void ClientGamePrecache( void )
{
	const char *pFilename = "scripts/client_precache.txt";
	KeyValues *pValues = new KeyValues( "ClientPrecache" );

	if ( !pValues->LoadFromFile( filesystem, pFilename, "GAME" ) )
	{
		Error( "Can't open %s for client precache info.", pFilename );
		pValues->deleteThis();
		return;
	}

	for ( KeyValues *pData = pValues->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey() )
	{
		const char *pszType = pData->GetName();
		const char *pszFile = pData->GetString();

		if ( Q_strlen( pszType ) > 0 && Q_strlen( pszFile ) > 0 )
		{
			if ( !Q_stricmp( pData->GetName(), "model" ) )
			{
				CBaseEntity::PrecacheModel( pszFile );
			}
			else if ( !Q_stricmp( pData->GetName(), "scriptsound" ) )
			{
				CBaseEntity::PrecacheScriptSound( pszFile );
			}
			else if ( !Q_stricmp( pData->GetName(), "particle" ) )
			{
				PrecacheParticleSystem( pszFile );
			}
		}
	}

	pValues->deleteThis();
}


// called by ClientKill and DeadThink
void respawn( CBaseEntity *pEdict, bool fCopyCorpse )
{
	CJBMod_Player *pPlayer = ToJBModPlayer( pEdict );

	if ( pPlayer )
	{
		if ( g_pScriptVM )
		{
			HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerRespawn" );
			if ( hFunction )
			{
				ScriptVariant_t result;
				g_pScriptVM->Call( hFunction, NULL, true, &result, pPlayer->GetScriptInstance() );
				g_pScriptVM->ReleaseFunction( hFunction );

				if ( result.GetType() == FIELD_BOOLEAN )
				{
					if ( (bool)result )
						pPlayer->Spawn();
					else
						pPlayer->SetNextThink( gpGlobals->curtime + 0.1f );
					return;
				}
			}
		}
	}
}

void GameStartFrame( void )
{
	VPROF("GameStartFrame()");
	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = (teamplay.GetInt() != 0);
}

//=========================================================
// instantiate the proper game rules object
//=========================================================
void InstallGameRules()
{
	CreateGameRulesObject( "CJBModRules" );
}

