//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "jbmod_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"

#ifdef CLIENT_DLL
	#include "c_jbmod_player.h"
#else

	#include "nav_mesh.h"
	#include "eventqueue.h"
	#include "player.h"
	#include "gamerules.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "voice_gamemgr.h"
	#include "iscorer.h"
	#include "jbmod_player.h"
	#include "weapon_jbmodbasehlmpcombatweapon.h"
	#include "team.h"
	#include "voice_gamemgr.h"
	#include "jbmod_gameinterface.h"
	#include "jbmod_cvars.h"

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

extern bool FindInList( const char **pStrings, const char *pToFind );

ConVar sv_jbmod_weapon_respawn_time( "sv_jbmod_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_jbmod_item_respawn_time( "sv_jbmod_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_report_client_settings("sv_report_client_settings", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_fov_min( "sv_fov_min", "75", FCVAR_GAMEDLL, "Minimum allowed FOV for players." );
ConVar sv_fov_max( "sv_fov_max", "90", FCVAR_GAMEDLL, "Maximum allowed FOV for players." );

extern ConVar mp_chattime;

#define WEAPON_MAX_DISTANCE_FROM_SPAWN 64

#endif


REGISTER_GAMERULES_CLASS( CJBModRules );

BEGIN_NETWORK_TABLE_NOBASE( CJBModRules, DT_JBModRules )

	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bTeamPlayEnabled ) ),
	#else
		SendPropBool( SENDINFO( m_bTeamPlayEnabled ) ),
	#endif

END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( jbmod_gamerules, CJBModGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( JBModGameRulesProxy, DT_JBModGameRulesProxy )

static JBModViewVectors g_JBModViewVectors(
	Vector( 0, 0, 64 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

#ifndef CLIENT_DLL

static const char *s_PreserveEnts[] =
{
	"ai_network",
	"ai_hint",
	"jbmod_gamerules",
	"jbmod_logic_gamemode",
	"team_manager",
	"player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_buyzone",
	"func_illusionary",
	"infodecal",
	"info_projecteddecal",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_deathmatch",
	"info_map_parameters",
	"keyframe_rope",
	"move_rope",
	"info_ladder",
	"player",
	"point_viewcontrol",
	"scene_manager",
	"shadow_control",
	"sky_camera",
	"soundent",
	"trigger_soundscape",
	"viewmodel",
	"predicted_viewmodel",
	"worldspawn",
	"point_devshot_camera",
	"", // END Marker
};

static CUtlVector<CUtlString> s_ExtraPreserveEnts;

static bool IsPreservedEntity( const char *pClassname )
{
	if ( FindInList( s_PreserveEnts, pClassname ) )
		return true;

	for ( int i = 0; i < s_ExtraPreserveEnts.Count(); i++ )
	{
		if ( !Q_stricmp( s_ExtraPreserveEnts[i].Get(), pClassname ) )
			return true;
	}
	return false;
}

#endif // CLIENT_DLL

#ifdef CLIENT_DLL
	void RecvProxy_JBModRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CJBModRules *pRules = JBModRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CJBModGameRulesProxy, DT_JBModGameRulesProxy )
		RecvPropDataTable( "jbmod_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_JBModRules ), RecvProxy_JBModRules )
	END_RECV_TABLE()
#else
	void* SendProxy_JBModRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CJBModRules *pRules = JBModRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CJBModGameRulesProxy, DT_JBModGameRulesProxy )
		SendPropDataTable( "jbmod_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_JBModRules ), SendProxy_JBModRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL

	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
		{
			return ( pListener->GetTeamNumber() == pTalker->GetTeamNumber() );
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

#endif

// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
char *sTeamNames[] =
{
	"Unassigned",
	"Spectator",
	"Combine",
	"Rebels",
};

CJBModRules::CJBModRules()
{
#ifndef CLIENT_DLL
	// Create the team managers
	for ( int i = 0; i < ARRAYSIZE( sTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "team_manager" ));
		pTeam->Init( sTeamNames[i], i );

		g_Teams.AddToTail( pTeam );
	}

	m_bTeamPlayEnabled = teamplay.GetBool();
	m_flIntermissionEndTime = 0.0f;
	m_flGameStartTime = 0;

	m_hRespawnableItemsAndWeapons.RemoveAll();
	m_tmNextPeriodicThink = 0;
	m_flRestartGameTime = 0;
	m_bCompleteReset = false;
	m_bHeardAllPlayersReady = false;
	m_bAwaitingReadyRestart = false;
	m_bChangelevelDone = false;

	m_szGameMode[0] = 0;
	m_szCustomGameDescription[0] = 0;

#endif
}

const CViewVectors* CJBModRules::GetViewVectors()const
{
	return &g_JBModViewVectors;
}

const JBModViewVectors* CJBModRules::GetJBModViewVectors()const
{
	return &g_JBModViewVectors;
}
	
CJBModRules::~CJBModRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
#endif
}

void CJBModRules::CreateStandardEntities( void )
{

#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

#ifdef DBGFLAG_ASSERT
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "jbmod_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CJBModRules::FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "GetWeaponRespawnTime" );
		if ( hFunction )
		{
			bool bLimitInWorld = ( pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD ) != 0;
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, pWeapon->GetClassname(), bLimitInWorld );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_FLOAT || result.GetType() == FIELD_INTEGER )
				return (float)result;
		}
	}
#endif

	return 0;
}


bool CJBModRules::IsIntermission( void )
{
#ifndef CLIENT_DLL
	return m_flIntermissionEndTime > gpGlobals->curtime;
#endif

	return false;
}

void CJBModRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	if ( IsIntermission() )
		return;
	BaseClass::PlayerKilled( pVictim, info );
#endif
}


void CJBModRules::Think( void )
{

#ifndef CLIENT_DLL
	
	CGameRules::Think();

	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			if ( !m_bChangelevelDone )
			{
				ChangeLevel(); // intermission is over
				m_bChangelevelDone = true;
			}
		}

		return;
	}

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnThink" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}

	if ( gpGlobals->curtime > m_tmNextPeriodicThink )
	{		
		CheckAllPlayersReady();
		CheckRestartGame();
		m_tmNextPeriodicThink = gpGlobals->curtime + 1.0;
	}

	if ( m_flRestartGameTime > 0.0f && m_flRestartGameTime <= gpGlobals->curtime )
	{
		RestartGame();
	}

	if( m_bAwaitingReadyRestart && m_bHeardAllPlayersReady )
	{
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "All players ready. Game will restart in 5 seconds" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "All players ready. Game will restart in 5 seconds" );

		m_flRestartGameTime = gpGlobals->curtime + 5;
		m_bAwaitingReadyRestart = false;
	}

	ManageObjectRelocation();

#endif
}

void CJBModRules::GoToIntermission( void )
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )
		return;

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnRoundEnd" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}

	g_fGameOver = true;

	m_flIntermissionEndTime = gpGlobals->curtime + mp_chattime.GetInt();

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );
		pPlayer->AddFlag( FL_FROZEN );
	}
#endif
	
}

bool CJBModRules::CheckGameOver()
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel(); // intermission is over			
		}

		return true;
	}
#endif

	return false;
}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CJBModRules::FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon && (pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( gEntList.NumberOfEntities() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}
#endif
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CJBModRules::VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	CWeaponJBModBase *pHL2Weapon = dynamic_cast< CWeaponJBModBase*>( pWeapon );

	if ( pHL2Weapon )
	{
		return pHL2Weapon->GetOriginalSpawnOrigin();
	}
#endif
	
	return pWeapon->GetAbsOrigin();
}

#ifndef CLIENT_DLL

CItem* IsManagedObjectAnItem( CBaseEntity *pObject )
{
	return dynamic_cast< CItem*>( pObject );
}

CWeaponJBModBase* IsManagedObjectAWeapon( CBaseEntity *pObject )
{
	return dynamic_cast< CWeaponJBModBase*>( pObject );
}

bool GetObjectsOriginalParameters( CBaseEntity *pObject, Vector &vOriginalOrigin, QAngle &vOriginalAngles )
{
	if ( CItem *pItem = IsManagedObjectAnItem( pObject ) )
	{
		if ( pItem->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;
		
		vOriginalOrigin = pItem->GetOriginalSpawnOrigin();
		vOriginalAngles = pItem->GetOriginalSpawnAngles();

		pItem->m_flNextResetCheckTime = gpGlobals->curtime + sv_jbmod_item_respawn_time.GetFloat();
		return true;
	}
	else if ( CWeaponJBModBase *pWeapon = IsManagedObjectAWeapon( pObject )) 
	{
		if ( pWeapon->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;

		vOriginalOrigin = pWeapon->GetOriginalSpawnOrigin();
		vOriginalAngles = pWeapon->GetOriginalSpawnAngles();

		pWeapon->m_flNextResetCheckTime = gpGlobals->curtime + sv_jbmod_weapon_respawn_time.GetFloat();
		return true;
	}

	return false;
}

void CJBModRules::ManageObjectRelocation( void )
{
	int iTotal = m_hRespawnableItemsAndWeapons.Count();

	if ( iTotal > 0 )
	{
		for ( int i = 0; i < iTotal; i++ )
		{
			CBaseEntity *pObject = m_hRespawnableItemsAndWeapons[i].Get();
			
			if ( pObject )
			{
				Vector vSpawOrigin;
				QAngle vSpawnAngles;

				if ( GetObjectsOriginalParameters( pObject, vSpawOrigin, vSpawnAngles ) == true )
				{
					float flDistanceFromSpawn = (pObject->GetAbsOrigin() - vSpawOrigin ).Length();

					if ( flDistanceFromSpawn > WEAPON_MAX_DISTANCE_FROM_SPAWN )
					{
						bool shouldReset = false;
						IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

						if ( pPhysics )
						{
							shouldReset = pPhysics->IsAsleep();
						}
						else
						{
							shouldReset = (pObject->GetFlags() & FL_ONGROUND) ? true : false;
						}

						if ( shouldReset )
						{
							pObject->Teleport( &vSpawOrigin, &vSpawnAngles, NULL );
							pObject->EmitSound( "AlyxEmp.Charge" );

							IPhysicsObject *pPhys = pObject->VPhysicsGetObject();

							if ( pPhys )
							{
								pPhys->Wake();
							}
						}
					}
				}
			}
		}
	}
}

//=========================================================
//AddLevelDesignerPlacedWeapon
//=========================================================
void CJBModRules::AddLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) == -1 )
	{
		m_hRespawnableItemsAndWeapons.AddToTail( pEntity );
	}
}

//=========================================================
//RemoveLevelDesignerPlacedWeapon
//=========================================================
void CJBModRules::RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) != -1 )
	{
		m_hRespawnableItemsAndWeapons.FindAndRemove( pEntity );
	}
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CJBModRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetOriginalSpawnOrigin();
}

//=========================================================
// What angles should this item use to respawn?
//=========================================================
QAngle CJBModRules::VecItemRespawnAngles( CItem *pItem )
{
	return pItem->GetOriginalSpawnAngles();
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CJBModRules::FlItemRespawnTime( CItem *pItem )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "GetItemRespawnTime" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, pItem->GetClassname() );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_FLOAT || result.GetType() == FIELD_INTEGER )
				return (float)result;
		}
	}

	return 0.0f;
}


//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CJBModRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "CanPickupWeapon" );
		if ( hFunction )
		{
			bool bOwnsWeapon = pPlayer->Weapon_OwnsThisType( pItem->GetClassname(), pItem->GetSubType() ) != NULL;
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result,
				pPlayer->GetScriptInstance(),
				pItem->GetClassname(),
				bOwnsWeapon );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_BOOLEAN )
				return (bool)result;
		}
	}

	return BaseClass::CanHavePlayerItem( pPlayer, pItem );
}

float CJBModRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "GetFallDamage" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result,
				pPlayer->GetScriptInstance(),
				pPlayer->m_Local.m_flFallVelocity );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_FLOAT || result.GetType() == FIELD_INTEGER )
				return (float)result;
		}
	}

	return BaseClass::FlPlayerFallDamage( pPlayer );
}

#endif

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CJBModRules::WeaponShouldRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "ShouldWeaponRespawn" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, pWeapon->GetClassname() );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_BOOLEAN )
				return (bool)result ? GR_WEAPON_RESPAWN_YES : GR_WEAPON_RESPAWN_NO;
		}
	}

	if ( pWeapon->HasSpawnFlags( SF_NORESPAWN ) )
	{
		return GR_WEAPON_RESPAWN_NO;
	}
#endif

	return GR_WEAPON_RESPAWN_YES;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just left the game
//-----------------------------------------------------------------------------
void CJBModRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	// Msg( "CLIENT DISCONNECTED, REMOVING FROM TEAM.\n" );

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );
	if ( pPlayer )
	{
		// Remove the player from his team
		if ( pPlayer->GetTeam() )
		{
			pPlayer->GetTeam()->RemovePlayer( pPlayer );
		}

		if ( pPlayer->IsInAVehicle() )
			pPlayer->LeaveVehicle();

		if ( g_pScriptVM )
		{
			HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerDisconnect" );
			if ( hFunction )
			{
				g_pScriptVM->Call( hFunction, NULL, true, NULL, pPlayer->GetScriptInstance() );
				g_pScriptVM->ReleaseFunction( hFunction );
			}
		}
	}

	BaseClass::ClientDisconnected( pClient );

#endif
}


//=========================================================
// Deathnotice. 
//=========================================================
void CJBModRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );

	// Custom kill type?
	if ( info.GetDamageCustom() )
	{
		killer_weapon_name = GetDamageCustomString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname();
					}
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}

		// strip the NPC_* or weapon_* from the inflictor's classname
		if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		{
			killer_weapon_name += 7;
		}
		else if ( strncmp( killer_weapon_name, "npc_", 4 ) == 0 )
		{
			killer_weapon_name += 4;
		}
		else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		{
			killer_weapon_name += 5;
		}
		else if ( strstr( killer_weapon_name, "physics" ) )
		{
			killer_weapon_name = "physics";
		}

		if ( strcmp( killer_weapon_name, "prop_combine_ball" ) == 0 )
		{
			killer_weapon_name = "combine_ball";
		}
		else if ( strcmp( killer_weapon_name, "grenade_ar2" ) == 0 )
		{
			killer_weapon_name = "smg1_grenade";
		}
		else if ( strcmp( killer_weapon_name, "satchel" ) == 0 || strcmp( killer_weapon_name, "tripmine" ) == 0)
		{
			killer_weapon_name = "slam";
		}


	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}
#endif

}

void CJBModRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	
	CJBMod_Player *pHL2Player = ToJBModPlayer( pPlayer );

	if ( pHL2Player == NULL )
		return;

	const char *pCurrentModel = modelinfo->GetModelName( pPlayer->GetModel() );
	const char *szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_playermodel" );

	//If we're different.
	if ( stricmp( szModelName, pCurrentModel ) )
	{
		//Too soon, set the cvar back to what it was.
		//Note: this will make this function be called again
		//but since our models will match it'll just skip this whole dealio.
		if ( pHL2Player->GetNextModelChangeTime() >= gpGlobals->curtime )
		{
			char szReturnString[512];

			Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", pCurrentModel );
			engine->ClientCommand ( pHL2Player->edict(), szReturnString );

			Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch.\n", (int)(pHL2Player->GetNextModelChangeTime() - gpGlobals->curtime) );
			ClientPrint( pHL2Player, HUD_PRINTTALK, szReturnString );
			return;
		}

		if ( g_pScriptVM )
		{
			HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerModelChanged" );
			if ( hFunction )
			{
				g_pScriptVM->Call( hFunction, NULL, true, NULL, pHL2Player->GetScriptInstance(), szModelName );
				g_pScriptVM->ReleaseFunction( hFunction );
			}
		}
	}
	if ( sv_report_client_settings.GetInt() == 1 )
	{
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pHL2Player->GetPlayerName(), engine->GetClientConVarValue( pHL2Player->entindex(), "cl_cmdrate" ));
	}

	const char *pszFov = engine->GetClientConVarValue( pHL2Player->entindex(), "fov_desired" );
	if ( pszFov )
	{
		static ConVarRef sv_fov_min( "sv_fov_min" );
		static ConVarRef sv_fov_max( "sv_fov_max" );
		int iFov = atoi( pszFov );
		iFov = clamp( iFov, sv_fov_min.GetInt(), sv_fov_max.GetInt() );
		pHL2Player->SetDefaultFOV( iFov );
	}

	BaseClass::ClientSettingsChanged( pPlayer );
#endif
	
}

int CJBModRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() )
		return GR_NOTTEAMMATE;

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "GetPlayerRelationship" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result,
				ToHScript( pPlayer ),
				ToHScript( pTarget ) );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_INTEGER )
				return (int)result;
		}
	}
#endif

	return GR_NOTTEAMMATE;
}

const char *CJBModRules::GetGameDescription( void )
{ 
#ifndef CLIENT_DLL
	if ( m_szCustomGameDescription[0] )
		return m_szCustomGameDescription;
#endif

	return "JBMod";
} 

bool CJBModRules::IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer )
{
	return true;
}
 
float CJBModRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	// timelimit is in minutes

	float timeleft = (m_flGameStartTime + mp_timelimit.GetInt() * 60.0f ) - gpGlobals->curtime;

	return timeleft;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJBModRules::Precache( void )
{
	CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" );
}

bool CJBModRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		V_swap(collisionGroup0,collisionGroup1);
	}

	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 

}

bool CJBModRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
#ifndef CLIENT_DLL
	if( BaseClass::ClientCommand( pEdict, args ) )
		return true;


	CJBMod_Player *pPlayer = (CJBMod_Player *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;
#endif

	return false;
}

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	
	if ( !bInitted )
	{
		bInitted = true;

		def.AddAmmoType("AR2",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			60,			BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("AR2AltFire",		DMG_DISSOLVE,				TRACER_NONE,			0,			0,			3,			0,							0 );
		def.AddAmmoType("Pistol",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			150,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("SMG1",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			225,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			12,			BULLET_IMPULSE(800, 5000),	0 );
		def.AddAmmoType("XBowBolt",			DMG_BULLET,					TRACER_LINE,			0,			0,			10,			BULLET_IMPULSE(800, 8000),	0 );
		def.AddAmmoType("Buckshot",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE,			0,			0,			30,			BULLET_IMPULSE(400, 1200),	0 );
		def.AddAmmoType("RPG_Round",		DMG_BURN,					TRACER_NONE,			0,			0,			3,			0,							0 );
		def.AddAmmoType("SMG1_Grenade",		DMG_BURN,					TRACER_NONE,			0,			0,			3,			0,							0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
		def.AddAmmoType("slam",				DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
	}

	return &def;
}

#ifdef CLIENT_DLL

	ConVar cl_autowepswitch(
		"cl_autowepswitch",
		"1",
		FCVAR_ARCHIVE | FCVAR_USERINFO,
		"Automatically switch to picked up weapons (if more powerful)" );

#else

	bool CJBModRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
	{		
		if ( pPlayer->GetActiveWeapon() && pPlayer->IsNetClient() )
		{
			// Player has an active item, so let's check cl_autowepswitch.
			const char *cl_autowepswitch = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_autowepswitch" );
			if ( cl_autowepswitch && atoi( cl_autowepswitch ) <= 0 )
			{
				return false;
			}
		}

		return BaseClass::FShouldSwitchWeapon( pPlayer, pWeapon );
	}

#endif

#ifndef CLIENT_DLL

void CJBModRules::RestartGame()
{
	// bounds check
	if ( mp_timelimit.GetInt() < 0 )
	{
		mp_timelimit.SetValue( 0 );
	}
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}

	CleanUpMap();
	
	// now respawn all players
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CJBMod_Player *pPlayer = (CJBMod_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		if ( pPlayer->GetActiveWeapon() )
		{
			pPlayer->GetActiveWeapon()->Holster();
		}
		pPlayer->RemoveAllItems( true );
		respawn( pPlayer, false );
		pPlayer->Reset();
	}

	// Respawn entities (glass, doors, etc..)

	CTeam *pRebels = GetGlobalTeam( TEAM_REBELS );
	CTeam *pCombine = GetGlobalTeam( TEAM_COMBINE );

	if ( pRebels )
	{
		pRebels->SetScore( 0 );
	}

	if ( pCombine )
	{
		pCombine->SetScore( 0 );
	}

	m_flIntermissionEndTime = 0;
	m_flRestartGameTime = 0.0;		
	m_bCompleteReset = false;

	IGameEvent * event = gameeventmanager->CreateEvent( "round_start" );
	if ( event )
	{
		event->SetInt("fraglimit", 0 );
		event->SetInt( "priority", 6 ); // HLTV event priority, not transmitted

		event->SetString("objective","DEATHMATCH");

		gameeventmanager->FireEvent( event );
	}

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnRoundStart" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}
}

#ifdef GAME_DLL
void CJBModRules::OnNavMeshLoad( void )
{
	TheNavMesh->SetPlayerSpawnName( "info_player_deathmatch" );
}
#endif

void CJBModRules::CleanUpMap()
{
	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.

	// Get rid of all entities except players.
	CBaseEntity *pCur = gEntList.FirstEnt();
	while ( pCur )
	{
		CBaseJBModCombatWeapon *pWeapon = dynamic_cast< CBaseJBModCombatWeapon* >( pCur );
		// Weapons with owners don't want to be removed..
		if ( pWeapon )
		{
			if ( !pWeapon->GetPlayerOwner() )
			{
				UTIL_Remove( pCur );
			}
		}
		// remove entities that has to be restored on roundrestart (breakables etc)
		else if ( !IsPreservedEntity( pCur->GetClassname() ) )
		{
			UTIL_Remove( pCur );
		}

		pCur = gEntList.NextEnt( pCur );
	}

	// Really remove the entities so we can have access to their slots below.
	gEntList.CleanupDeleteList();

	// Cancel all queued events, in case a func_bomb_target fired some delayed outputs that
	// could kill respawning CTs
	g_EventQueue.Clear();

	// Now reload the map entities.
	class CJBModMapEntityFilter : public IMapEntityFilter
	{
	public:
		virtual bool ShouldCreateEntity( const char *pClassname )
		{
			// Don't recreate the preserved entities.
			if ( !IsPreservedEntity( pClassname ) )
			{
				return true;
			}
			else
			{
				// Increment our iterator since it's not going to call CreateNextEntity for this ent.
				if ( m_iIterator != g_MapEntityRefs.InvalidIndex() )
					m_iIterator = g_MapEntityRefs.Next( m_iIterator );

				return false;
			}
		}


		virtual CBaseEntity* CreateNextEntity( const char *pClassname )
		{
			if ( m_iIterator == g_MapEntityRefs.InvalidIndex() )
			{
				// This shouldn't be possible. When we loaded the map, it should have used 
				// CCSMapLoadEntityFilter, which should have built the g_MapEntityRefs list
				// with the same list of entities we're referring to here.
				Assert( false );
				return NULL;
			}
			else
			{
				CMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
				m_iIterator = g_MapEntityRefs.Next( m_iIterator );	// Seek to the next entity.

				if ( ref.m_iEdict == -1 || engine->PEntityOfEntIndex( ref.m_iEdict ) )
				{
					// Doh! The entity was delete and its slot was reused.
					// Just use any old edict slot. This case sucks because we lose the baseline.
					return CreateEntityByName( pClassname );
				}
				else
				{
					// Cool, the slot where this entity was is free again (most likely, the entity was 
					// freed above). Now create an entity with this specific index.
					return CreateEntityByName( pClassname, ref.m_iEdict );
				}
			}
		}

	public:
		int m_iIterator; // Iterator into g_MapEntityRefs.
	};
	CJBModMapEntityFilter filter;
	filter.m_iIterator = g_MapEntityRefs.Head();

	// DO NOT CALL SPAWN ON info_node ENTITIES!

	MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &filter, true );
}

void CJBModRules::CheckChatForReadySignal( CJBMod_Player *pPlayer, const char *chatmsg )
{
	if( m_bAwaitingReadyRestart && FStrEq( chatmsg, mp_ready_signal.GetString() ) )
	{
		if( !pPlayer->IsReady() )
		{
			pPlayer->SetReady( true );
		}		
	}
}

void CJBModRules::CheckRestartGame( void )
{
	// Restart the game if specified by the server
	int iRestartDelay = mp_restartgame.GetInt();

	if ( iRestartDelay > 0 )
	{
		if ( iRestartDelay > 60 )
			iRestartDelay = 60;


		// let the players know
		char strRestartDelay[64];
		Q_snprintf( strRestartDelay, sizeof( strRestartDelay ), "%d", iRestartDelay );
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );

		m_flRestartGameTime = gpGlobals->curtime + iRestartDelay;
		m_bCompleteReset = true;
		mp_restartgame.SetValue( 0 );
	}

	if( mp_readyrestart.GetBool() )
	{
		m_bAwaitingReadyRestart = true;
		m_bHeardAllPlayersReady = false;
		

		const char *pszReadyString = mp_ready_signal.GetString();


		// Don't let them put anything malicious in there
		if( pszReadyString == NULL || Q_strlen(pszReadyString) > 16 )
		{
			pszReadyString = "ready";
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "jbmod_ready_restart" );
		if ( event )
			gameeventmanager->FireEvent( event );

		mp_readyrestart.SetValue( 0 );

		// cancel any restart round in progress
		m_flRestartGameTime = -1;
	}
}

void CJBModRules::CheckAllPlayersReady( void )
{
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CJBMod_Player *pPlayer = (CJBMod_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		if ( !pPlayer->IsReady() )
			return;
	}
	m_bHeardAllPlayersReady = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CJBModRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
	{
		return NULL;
	}

	const char *pszFormat = NULL;

	// team only
	if ( bTeamOnly == TRUE )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "JBMod_Chat_Spec";
		}
		else
		{
			const char *chatLocation = GetChatLocation( bTeamOnly, pPlayer );
			if ( chatLocation && *chatLocation )
			{
				pszFormat = "JBMod_Chat_Team_Loc";
			}
			else
			{
				pszFormat = "JBMod_Chat_Team";
			}
		}
	}
	// everyone
	else
	{
		if ( pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		{
			pszFormat = "JBMod_Chat_All";	
		}
		else
		{
			pszFormat = "JBMod_Chat_AllSpec";
		}
	}

	return pszFormat;
}

//-----------------------------------------------------------------------------
// VScript Registration
//-----------------------------------------------------------------------------
static void ScriptSetGameDescription( const char *pszDesc )
{
	if ( JBModRules() && pszDesc )
	{
		Q_strncpy( JBModRules()->m_szCustomGameDescription, pszDesc, sizeof( JBModRules()->m_szCustomGameDescription ) );
	}
}

static const char *ScriptGetGameDescription()
{
	if ( JBModRules() )
	{
		return JBModRules()->GetGameDescription();
	}
	return "";
}

static const char *ScriptGetGameMode()
{
	if ( JBModRules() )
	{
		return JBModRules()->m_szGameMode;
	}
	return "";
}

static void ScriptGoToIntermission()
{
	if ( JBModRules() )
	{
		JBModRules()->GoToIntermission();
	}
}

static bool ScriptIsGameOver()
{
	return g_fGameOver;
}

static float ScriptGetMapRemainingTime()
{
	if ( JBModRules() )
	{
		return JBModRules()->GetMapRemainingTime();
	}
	return -1;
}

static bool ScriptIsTeamplay()
{
	if ( JBModRules() )
	{
		return JBModRules()->IsTeamplay();
	}
	return false;
}

static void ScriptPrecacheModel( const char *szModel )
{
	if ( szModel && szModel[0] )
	{
		CBaseEntity::PrecacheModel( szModel );
	}
}

static void ScriptPrecacheSound( const char *szSound )
{
	if ( szSound && szSound[0] )
	{
		CBaseEntity::PrecacheScriptSound( szSound );
	}
}

static HSCRIPT ScriptGetPlayerByIndex( int index )
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( index );
	if ( pPlayer )
		return pPlayer->GetScriptInstance();
	return NULL;
}

static int ScriptGetMaxPlayers()
{
	return gpGlobals->maxClients;
}

static int ScriptGetTeamScore( int team )
{
	if ( team >= 0 && team < g_Teams.Count() )
		return g_Teams[team]->GetScore();
	return 0;
}

static int ScriptGetTeamPlayerCount( int team )
{
	if ( team >= 0 && team < g_Teams.Count() )
		return g_Teams[team]->GetNumPlayers();
	return 0;
}

static const char *ScriptGetMapName()
{
	return STRING( gpGlobals->mapname );
}

static const char *ScriptGetTeamName( int team )
{
	if ( team >= 0 && team < g_Teams.Count() )
		return g_Teams[team]->GetName();
	return "";
}

static void ScriptSetTeamName( int team, const char *pszName )
{
	if ( team >= 0 && team < g_Teams.Count() && pszName )
	{
		Q_strncpy( g_Teams[team]->m_szTeamname.GetForModify(), pszName, MAX_TEAM_NAME_LENGTH );
	}
}

static void ScriptPreserveEntityClass( const char *pszClassname )
{
	if ( !pszClassname || !pszClassname[0] )
		return;

	for ( int i = 0; i < s_ExtraPreserveEnts.Count(); i++ )
	{
		if ( !Q_stricmp( s_ExtraPreserveEnts[i].Get(), pszClassname ) )
			return;
	}

	s_ExtraPreserveEnts.AddToTail( CUtlString( pszClassname ) );
}

void CJBModRules::RegisterScriptFunctions()
{
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptSetGameDescription, "SetGameDescription", "Set the game description." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetGameDescription, "GetGameDescription", "Get the current game description." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetGameMode, "GetGameMode", "Get the current game mode name." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGoToIntermission, "GoToIntermission", "End the game and go to the scoreboard." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptIsGameOver, "IsGameOver", "Returns true if the game is in intermission." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetMapRemainingTime, "GetMapRemainingTime", "Returns seconds remaining on the map timer, or -1 if no limit." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptIsTeamplay, "IsTeamplay", "Returns true if teamplay is enabled." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptPrecacheModel, "PrecacheModel", "Precache a model for use." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptPrecacheSound, "PrecacheSound", "Precache a sound for use." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetPlayerByIndex, "GetPlayerByIndex", "Get a player entity by index (1-based)." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetMaxPlayers, "GetMaxPlayers", "Get the maximum number of players." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetTeamScore, "GetTeamScore", "Get the score for a team by index." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetTeamPlayerCount, "GetTeamPlayerCount", "Get the number of players on a team." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetMapName, "GetMapName", "Get the current map name." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptGetTeamName, "GetTeamName", "Get the name of a team by index." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptSetTeamName, "SetTeamName", "Set the name of a team by index." );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptPreserveEntityClass, "PreserveEntityClass", "Add an entity classname to the round-restart preserve list." );
}

#endif
