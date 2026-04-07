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
// Sandbox gamemode
// Starts with a physgun, will eventually be where spawn menus etc are
//=============================================================================
SetGameDescription( "JBMod Sandbox" );
Convars.SetDefault( "sv_infinite_aux_power", "1" );
Convars.SetDefault( "mp_falldamage", "1" );

//=============================================================================
// Include base gamemode for default behavior
//=============================================================================
IncludeScript( "gamemodes/base.nut" );

//=============================================================================
// Spawn with suit + physgun
//=============================================================================
function OnPlayerSpawn( player )
{
	player.EquipSuit();
	player.GiveItem( "weapon_physgun" );
}
