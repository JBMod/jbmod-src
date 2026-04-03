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
// Default gamemode selector
// This script runs if no jbmod_logic_gamemode is found in the map, and tries
// to load a gamemode based on the map name prefix.
//=============================================================================

local mapname = GetMapName();
local lastSlash = mapname.find( "/" );
if ( lastSlash != null )
    mapname = mapname.slice( lastSlash + 1 );

if ( mapname.slice( 0, 3 ) == "dm_" )
    IncludeScript( "gamemodes/deathmatch.nut" );
else if ( mapname.slice( 0, 3 ) == "cs_" || mapname.slice( 0, 3 ) == "de_" )
    IncludeScript( "gamemodes/cstrike.nut" );
else if ( mapname.slice( 0, 4 ) == "dod_" )
    IncludeScript( "gamemodes/dod.nut" );
else if ( mapname.slice( 0, 6 ) == "arena_" || mapname.slice( 0, 3 ) == "cp_" || mapname.slice( 0, 4 ) == "ctf_" || mapname.slice( 0, 5 ) == "koth_" || mapname.slice( 0, 3 ) == "pl_" )
    IncludeScript( "gamemodes/tf.nut" );
