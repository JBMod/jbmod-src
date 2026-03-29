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

printl( "Initializing Sandbox Game Mode..." );
SetGameDescription( "JBMod Sandbox" );
Convars.SetValue( "sv_infinite_aux_power", 1 );

function OnPlayerSpawn( player )
{
	player.EquipSuit();
	player.GiveItem( "weapon_physcannon" );
	player.GiveItem( "weapon_crowbar" );
	player.GiveItem( "weapon_pistol" );
	player.GiveItem( "weapon_357" );
	player.GiveItem( "weapon_stunstick" );
	player.GiveItem( "weapon_smg1" );
	player.GiveItem( "weapon_physcannon" );
	player.GiveItem( "weapon_ar2" );
	player.GiveItem( "weapon_shotgun" );
	player.GiveItem( "weapon_frag" );
	player.GiveItem( "weapon_crossbow" );
	player.GiveItem( "weapon_rpg" );
	player.GiveItem( "weapon_slam" );
	player.GiveItem( "weapon_alyxgun_p" );
	//player.GiveItem( "weapon_bugbait" );
	//player.GiveItem( "weapon_annabelle" );
	//TODO: Give ammo to the player
}
