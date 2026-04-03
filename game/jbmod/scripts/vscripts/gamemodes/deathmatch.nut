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
// Deathmatch gamemode
// Implements most HL2DM logic
//=============================================================================
SetGameDescription( "JBMod Deathmatch" );

//=============================================================================
// Include base gamemode for default behavior
//=============================================================================
IncludeScript( "gamemodes/base.nut" );

//=============================================================================
// Spawn Point Registration
//=============================================================================
RegisterEntityClass( "info_player_combine", "info_player_start" );
RegisterEntityClass( "info_player_rebel", "info_player_start" );

//=============================================================================
// Preserve Entities
//=============================================================================
PreserveEntityClass( "info_player_combine" );
PreserveEntityClass( "info_player_rebel" );

//=============================================================================
// In deathmatch, players can be either combine or rebels
//=============================================================================
CitizenModels <- [
	"models/humans/group03/male_01.mdl",
	"models/humans/group03/male_02.mdl",
	"models/humans/group03/female_01.mdl",
	"models/humans/group03/male_03.mdl",
	"models/humans/group03/female_02.mdl",
	"models/humans/group03/male_04.mdl",
	"models/humans/group03/female_03.mdl",
	"models/humans/group03/male_05.mdl",
	"models/humans/group03/female_04.mdl",
	"models/humans/group03/male_06.mdl",
	"models/humans/group03/female_06.mdl",
	"models/humans/group03/male_07.mdl",
	"models/humans/group03/female_07.mdl",
	"models/humans/group03/male_08.mdl",
	"models/humans/group03/male_09.mdl",
];

CombineModels <- [
	"models/combine_soldier.mdl",
	"models/combine_soldier_prisonguard.mdl",
	"models/combine_super_soldier.mdl",
	"models/police.mdl",
];

LastCitizenModel <- 0;
LastCombineModel <- 0;

//=============================================================================
// Precache models and sounds
//=============================================================================
function OnPrecache()
{
	foreach ( m in CitizenModels )
		PrecacheModel( m );
	foreach ( m in CombineModels )
		PrecacheModel( m );

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );
}

//=============================================================================
// Only allow models defined above
//=============================================================================
function IsValidModel( model )
{
	foreach ( m in CitizenModels )
		if ( m == model ) return true;
	foreach ( m in CombineModels )
		if ( m == model ) return true;
	return false;
}

//=============================================================================
// Model Selection
//=============================================================================
function OnPlayerSetModel( player, model )
{
	if ( IsTeamplay() )
	{
		local team = player.GetTeam();
		if ( team == Constants.ETeam.TEAM_COMBINE )
		{
			model = CombineModels[LastCombineModel % CombineModels.len()];
			LastCombineModel++;
		}
		else if ( team == Constants.ETeam.TEAM_REBELS )
		{
			model = CitizenModels[LastCitizenModel % CitizenModels.len()];
			LastCitizenModel++;
		}
	}

	if ( !IsValidModel( model ) )
		model = "models/combine_soldier.mdl";

	return model;
}

//=============================================================================
// Model Change Notification
//=============================================================================
function OnPlayerModelChanged( player, model )
{
	if ( IsTeamplay() )
		return;

	if ( !IsValidModel( model ) )
		player.SetPlayerModel( "models/combine_soldier.mdl" );
}

//=============================================================================
// Spawn Point Selection
//=============================================================================
function GetSpawnPointClassname( player )
{
	if ( !IsTeamplay() )
		return "info_player_deathmatch";

	local team = player.GetTeam();
	if ( team == Constants.ETeam.TEAM_COMBINE )
		return "info_player_combine";
	else if ( team == Constants.ETeam.TEAM_REBELS )
		return "info_player_rebel";
	return "info_player_deathmatch";
}

//=============================================================================
// Team Selection
//=============================================================================
function OnPlayerPickTeam( player )
{
	if ( IsTeamplay() )
	{
		// Auto-balance: assign to the smaller team
		local combineCount = GetTeamPlayerCount( Constants.ETeam.TEAM_COMBINE );
		local rebelsCount = GetTeamPlayerCount( Constants.ETeam.TEAM_REBELS );

		if ( combineCount > rebelsCount )
			return Constants.ETeam.TEAM_REBELS;
		else if ( combineCount < rebelsCount )
			return Constants.ETeam.TEAM_COMBINE;
		else
			return (RandomInt( 0, 1 ) == 0) ? Constants.ETeam.TEAM_COMBINE : Constants.ETeam.TEAM_REBELS;
	}
	return Constants.ETeam.TEAM_UNASSIGNED;
}

//=============================================================================
// Loadout
//=============================================================================
function IsCombineModel( player )
{
	local model = player.GetModelName();
	return model.find( "police" ) != null || model.find( "combine" ) != null;
}
function OnPlayerSpawn( player )
{
	player.EquipSuit();

	player.GiveAmmo( 255, "Pistol" );
	player.GiveAmmo( 45,  "SMG1" );
	player.GiveAmmo( 1,   "grenade" );
	player.GiveAmmo( 6,   "Buckshot" );
	player.GiveAmmo( 6,   "357" );

	if ( IsCombineModel( player ) )
		player.GiveItem( "weapon_stunstick" );
	else
		player.GiveItem( "weapon_crowbar" );

	player.GiveItem( "weapon_pistol" );
	player.GiveItem( "weapon_smg1" );
	player.GiveItem( "weapon_frag" );
	player.GiveItem( "weapon_physcannon" );

	local defaultWpn = player.GetClientConVar( "cl_defaultweapon" );
	if ( defaultWpn != "" )
		player.SwitchToWeapon( defaultWpn );
	else
		player.SwitchToWeapon( "weapon_physcannon" );
}

//=============================================================================
// Scoring
//=============================================================================
function OnPlayerKilled( victim, attacker )
{
	if ( attacker == null )
		return;

	if ( attacker == victim )
		attacker.AddTeamScore( -1 );
	else
		attacker.AddTeamScore( 1 );
}

//=============================================================================
// Impulse 101 / Give All Items
//=============================================================================
function OnGiveAllItems( player )
{
	player.EquipSuit();

	player.GiveAmmo( 255, "Pistol" );
	player.GiveAmmo( 255, "AR2" );
	player.GiveAmmo( 5,   "AR2AltFire" );
	player.GiveAmmo( 255, "SMG1" );
	player.GiveAmmo( 1,   "smg1_grenade" );
	player.GiveAmmo( 255, "Buckshot" );
	player.GiveAmmo( 32,  "357" );
	player.GiveAmmo( 3,   "rpg_round" );
	player.GiveAmmo( 16,  "XBowBolt" );
	player.GiveAmmo( 1,   "grenade" );
	player.GiveAmmo( 2,   "slam" );

	player.GiveItem( "weapon_crowbar" );
	player.GiveItem( "weapon_stunstick" );
	player.GiveItem( "weapon_pistol" );
	player.GiveItem( "weapon_357" );
	player.GiveItem( "weapon_smg1" );
	player.GiveItem( "weapon_ar2" );
	player.GiveItem( "weapon_shotgun" );
	player.GiveItem( "weapon_frag" );
	player.GiveItem( "weapon_crossbow" );
	player.GiveItem( "weapon_rpg" );
	player.GiveItem( "weapon_slam" );
	player.GiveItem( "weapon_physcannon" );
}

//=============================================================================
// Death Sound
//=============================================================================
function GetModelSoundPrefix( player )
{
	local model = player.GetModelName();
	if ( model.find( "police" ) != null )
		return "NPC_MetroPolice";
	else if ( model.find( "combine" ) != null )
		return "NPC_CombineS";
	return "NPC_Citizen";
}
function OnPlayerDeathSound( player )
{
	return GetModelSoundPrefix( player ) + ".Die";
}

//=============================================================================
// Win Conditions
//=============================================================================
function CheckWinConditions()
{
	if ( IsGameOver() )
		return;

	// Time limit
	if ( GetMapRemainingTime() < 0 )
	{
		GoToIntermission();
		return;
	}

	// Frag limit
	local fragLimit = Convars.GetFloat( "mp_fraglimit" );
	if ( fragLimit <= 0 )
		return;

	if ( IsTeamplay() )
	{
		if ( GetTeamScore( Constants.ETeam.TEAM_COMBINE ) >= fragLimit
		  || GetTeamScore( Constants.ETeam.TEAM_REBELS ) >= fragLimit )
		{
			GoToIntermission();
			return;
		}
	}
	else
	{
		for ( local i = 1; i <= GetMaxPlayers(); i++ )
		{
			local player = GetPlayerByIndex( i );
			if ( player != null && player.GetFragCount() >= fragLimit )
			{
				GoToIntermission();
				return;
			}
		}
	}
}
function OnThink()
{
	CheckWinConditions();
}

//=============================================================================
// Respawn (wait for death animation, then respawn)
//=============================================================================
function OnPlayerRespawn( player )
{
	return Time() > player.GetDeathTime() + Constants.Server.DEATH_ANIMATION_TIME;
}

//=============================================================================
// Player Relationship
//=============================================================================
function GetPlayerRelationship( player, target )
{
	if ( IsTeamplay() && player.GetTeam() == target.GetTeam() )
		return Constants.ERelationship.GR_TEAMMATE;
	return Constants.ERelationship.GR_NOTTEAMMATE;
}

//=============================================================================
// Player Connect
//=============================================================================
function OnPlayerConnect( player )
{
	local name = player.GetPlayerName();
	if ( name == "" )
		name = "<unconnected>";
	ClientPrint( null, Constants.EHudNotify.HUD_PRINTNOTIFY, name + " has joined the game\n" );

	// Show team info in teamplay
	if ( IsTeamplay() )
	{
		ClientPrint( player, Constants.EHudNotify.HUD_PRINTTALK, "You are on team " + GetTeamName( player.GetTeam() ) + "\n" );
	}

	// Show MOTD
	player.ShowMOTD();
}
