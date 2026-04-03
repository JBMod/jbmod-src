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
// Day of Defeat gamemode
// Very basic version to load maps, no gameplay logic
//=============================================================================

//=============================================================================
// Include base gamemode for default behavior
//=============================================================================
IncludeScript( "gamemodes/base.nut" );

//=============================================================================
// Spawn Point Registration
//=============================================================================
RegisterEntityClass( "info_player_allies", "info_player_start" );
RegisterEntityClass( "info_player_axis", "info_player_start" );

//=============================================================================
// Preserve Entities
//=============================================================================
PreserveEntityClass( "info_player_allies" );
PreserveEntityClass( "info_player_axis" );

//=============================================================================
// Team Constants
//=============================================================================
const TEAM_ALLIES = Constants.ETeam.TEAM_COMBINE;
const TEAM_AXIS  = Constants.ETeam.TEAM_REBELS;

//=============================================================================
// Spawn Point Selection
//=============================================================================
function GetSpawnPointClassname( player )
{
	local team = player.GetTeam();
	if ( team == TEAM_ALLIES )
		return "info_player_allies";
	else if ( team == TEAM_AXIS )
		return "info_player_axis";
	return "info_player_deathmatch";
}
