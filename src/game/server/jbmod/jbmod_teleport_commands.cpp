

// Copied these includes from client.cpp just to be on the safe side, but I probably don't need most of them to do this
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "jbmod_gamerules.h"
#include "game.h"
#include "physics.h"
#include "entitylist.h"
#include "shake.h"
#include "globalstate.h"
#include "event_tempentity_tester.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
#include <ctype.h>
#include "tier1/strtools.h"
#include "te_effect_dispatch.h"
#include "globals.h"
#include "nav_mesh.h"
#include "team.h"
#include "datacache/imdlcache.h"
#include "basemultiplayerplayer.h"
#include "voice_gamemgr.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar player_teleport_distance("player_teleport_distance","48",FCVAR_NOTIFY, "Distance in Hammer units used by goto_player and bring_player.");


static CBasePlayer* FindPlayerByPartialName(const char* pszName)
{
	if (!pszName || !pszName[0])
		return NULL;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = ToBasePlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (Q_stristr(pPlayer->GetPlayerName(), pszName))
			return pPlayer;
	}

	return NULL;
}


static bool IsTeleportPositionValid(CBasePlayer* pPlayer, const Vector& vecPos)
{
	if (!pPlayer)
		return false;

	trace_t tr;
	UTIL_TraceHull(
		vecPos,
		vecPos,
		VEC_HULL_MIN,
		VEC_HULL_MAX,
		MASK_PLAYERSOLID,
		pPlayer,
		COLLISION_GROUP_PLAYER_MOVEMENT,
		&tr
	);

	return !tr.startsolid && !tr.allsolid;
}



// Probably a better way of doing this, unsure though
static bool FindSafeTeleportPosition(CBasePlayer* pPlayer, const Vector& vecBasePos, float flDistance, Vector& vecOut)
{
	Vector testOffsets[] =
	{
		Vector(flDistance, 0, 0),
		Vector(-flDistance, 0, 0),
		Vector(0, flDistance, 0),
		Vector(0, -flDistance, 0),
		Vector(flDistance, flDistance, 0),
		Vector(-flDistance, flDistance, 0),
		Vector(flDistance, -flDistance, 0),
		Vector(-flDistance, -flDistance, 0),
		Vector(0, 0, flDistance),
	};

	for (int i = 0; i < ARRAYSIZE(testOffsets); i++)
	{
		Vector vecTest = vecBasePos + testOffsets[i];

		if (IsTeleportPositionValid(pPlayer, vecTest))
		{
			vecOut = vecTest;
			return true;
		}
	}

	return false;
}


CON_COMMAND(goto_player, "Teleport yourself near another player. Usage: goto_player <player name>")
{
	CBasePlayer* pCaller = ToBasePlayer(UTIL_GetCommandClient());


	if (!UTIL_IsCommandIssuedByServerAdmin())
		return;




	if (!pCaller)
		return;

	if (args.ArgC() < 2)
	{
		ClientPrint(pCaller, HUD_PRINTCONSOLE, "Usage: goto_player <player name>\n");
		return;
	}

	CBasePlayer* pTarget = FindPlayerByPartialName(args[1]);

	if (!pTarget)
	{
		ClientPrint(pCaller, HUD_PRINTCONSOLE, "Player not found.\n");
		return;
	}

	float flDistance = player_teleport_distance.GetFloat();

	Vector vecNewPos;

	if (!FindSafeTeleportPosition(pCaller, pTarget->GetAbsOrigin(), flDistance, vecNewPos))
	{
		ClientPrint(pCaller, HUD_PRINTCONSOLE, "Could not find safe teleport position.\n");
		return;
	}

	pCaller->Teleport(&vecNewPos, NULL, NULL);
}

CON_COMMAND(bring_player, "Bring another player near you. Usage: bring_player <player name>")
{
	CBasePlayer* pCaller = ToBasePlayer(UTIL_GetCommandClient());

	if (!UTIL_IsCommandIssuedByServerAdmin())
		return;

	if (!pCaller)
		return;

	if (args.ArgC() < 2)
	{
		ClientPrint(pCaller, HUD_PRINTCONSOLE, "Usage: bring_player <player name>\n");
		return;
	}

	CBasePlayer* pTarget = FindPlayerByPartialName(args[1]);

	if (!pTarget)
	{
		ClientPrint(pCaller, HUD_PRINTCONSOLE, "Player not found.\n");
		return;
	}

	if (pTarget == pCaller)
		return;

	float flDistance = player_teleport_distance.GetFloat();

	Vector vecNewPos;

	if (!FindSafeTeleportPosition(pTarget, pCaller->GetAbsOrigin(), flDistance, vecNewPos))
	{
		ClientPrint(pCaller, HUD_PRINTCONSOLE, "Could not find safe teleport position.\n");
		return;
	}

	pTarget->Teleport(&vecNewPos, NULL, NULL);
}