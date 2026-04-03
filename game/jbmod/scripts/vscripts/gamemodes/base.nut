// Copyright 2026 The JBMod Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//=============================================================================
// Base gamemode defaults
// All gamemodes should usually IncludeScript this first.
// Override any function below to customize behavior.
//=============================================================================

//=============================================================================
// Spawn Point Registration
//=============================================================================
RegisterEntityClass( "info_player_deathmatch", "info_player_start" );

//=============================================================================
// Precache
//=============================================================================
function OnPrecache()
{
}

//=============================================================================
// Spawn Point Selection
//=============================================================================
function GetSpawnPointClassname( player )
{
	return "info_player_deathmatch";
}

//=============================================================================
// Team Selection
//=============================================================================
function OnPlayerPickTeam( player )
{
	return Constants.ETeam.TEAM_UNASSIGNED;
}

//=============================================================================
// Model Selection (no-op: engine picks default)
//=============================================================================
function OnPlayerSetModel( player, model )
{
	return model;
}

//=============================================================================
// Model Change Notification
//=============================================================================
function OnPlayerModelChanged( player, model )
{
}

//=============================================================================
// Loadout
//=============================================================================
function OnPlayerSpawn( player )
{
	player.EquipSuit();
}

//=============================================================================
// Impulse 101 / Give All Items (no-op by default)
//=============================================================================
function OnGiveAllItems( player )
{
}

//=============================================================================
// Respawn (immediate by default)
//=============================================================================
function OnPlayerRespawn( player )
{
	return true;
}

//=============================================================================
// Player Killed
//=============================================================================
function OnPlayerKilled( victim, attacker )
{
}

//=============================================================================
// Death Sound
//=============================================================================
function OnPlayerDeathSound( player )
{
	return "NPC_Citizen.Die";
}

//=============================================================================
// Per-tick Think
//=============================================================================
function OnThink()
{
}

//=============================================================================
// Damage Modification
//=============================================================================
function OnPlayerTakeDamage( victim, attacker, damage )
{
	return damage;
}

//=============================================================================
// Fall Damage (null for default, 0 for none, or custom value)
//=============================================================================
function GetFallDamage( player, fallSpeed )
{
	return null;
}

//=============================================================================
// Team Changes
//=============================================================================
function OnPlayerChangeTeam( player, newTeam )
{
	return true;
}

//=============================================================================
// Player Relationship
//=============================================================================
function GetPlayerRelationship( player, target )
{
	return Constants.ERelationship.GR_NOTTEAMMATE;
}

//=============================================================================
// Weapon/Item Respawn
//=============================================================================
function GetWeaponRespawnTime( classname, limitInWorld )
{
	if ( Convars.GetFloat( "mp_weaponstay" ) > 0 && !limitInWorld )
		return 0;

	return Convars.GetFloat( "sv_jbmod_weapon_respawn_time" );
}

function GetItemRespawnTime( classname )
{
	return Convars.GetFloat( "sv_jbmod_item_respawn_time" );
}

function ShouldWeaponRespawn( classname )
{
	return true;
}

function CanPickupWeapon( player, classname, ownsWeapon )
{
	if ( Convars.GetFloat( "mp_weaponstay" ) > 0 && ownsWeapon )
		return false;

	return true;
}

//=============================================================================
// Connection / Disconnection
//=============================================================================
function OnPlayerConnect( player )
{
	local name = player.GetPlayerName();
	if ( name == "" )
		name = "<unconnected>";
	ClientPrint( null, Constants.EHudNotify.HUD_PRINTNOTIFY, name + " has joined the game\n" );

	// Show MOTD by default, override in your own gamemode if you don't want it
	player.ShowMOTD();
}

function OnPlayerDisconnect( player )
{
}

//=============================================================================
// Chat (return false to block)
//=============================================================================
function OnPlayerChat( player, text )
{
	return text;
}

//=============================================================================
// Round Lifecycle
//=============================================================================
function OnRoundStart()
{
}

function OnRoundEnd()
{
}
