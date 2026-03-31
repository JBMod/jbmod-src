// Default gamemode selector
// This script runs if no jbmod_logic_gamemode is found in the map.

// TODO: Once we have mounting place, we can also match by folder/vpk
// This is just a demo for now

local mapname = GetMapName();
local lastSlash = mapname.find( "/" );
if ( lastSlash != null )
    mapname = mapname.slice( lastSlash + 1 );

local dot = mapname.find( ".bsp" );
if ( dot != null )
    mapname = mapname.slice( 0, dot );

if ( mapname.slice( 0, 3 ) == "dm_" )
    IncludeScript( "gamemodes/deathmatch.nut" );
else if ( mapname.slice( 0, 3 ) == "cs_" || mapname.slice( 0, 3 ) == "de_" )
    IncludeScript( "gamemodes/cstrike.nut" );
else if ( mapname.slice( 0, 4 ) == "dod_" )
    IncludeScript( "gamemodes/dod.nut" );
else if ( mapname.slice( 0, 6 ) == "arena_" || mapname.slice( 0, 3 ) == "cp_" || mapname.slice( 0, 4 ) == "ctf_" || mapname.slice( 0, 5 ) == "koth_" || mapname.slice( 0, 3 ) == "pl_" )
    IncludeScript( "gamemodes/tf.nut" );
