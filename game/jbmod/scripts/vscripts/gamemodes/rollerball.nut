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
// Rollerball gamemode
// Sample gamemode for testing non-standard player movement
//=============================================================================
SetGameDescription( "Rollerball" );

//=============================================================================
// Include base gamemode for default behavior
//=============================================================================
IncludeScript( "gamemodes/base.nut" );

//=============================================================================
// Internal entity mapping
//=============================================================================
PlayerBalls <- {};
JumpCooldowns <- {};
SprintPower <- {};
BallCounter <- 0;

//=============================================================================
// Physics tuning
//=============================================================================
ROLL_FORCE <- 400.0;
ROLL_FORCE_BACK <- 320.0;
JUMP_FORCE <- 15000.0;
JUMP_COOLDOWN <- 0.8;

//=============================================================================
// Sprint tuning
//=============================================================================
SPRINT_MULTIPLIER <- 2.0;
SPRINT_MAX <- 100.0;
SPRINT_DRAIN <- 25.0;
SPRINT_RECHARGE <- 15.0;

//=============================================================================
// Precache rollermine
//=============================================================================
function OnPrecache()
{
	PrecacheModel( "models/roller.mdl" );
}

//=============================================================================
// Spawn: create a ball, parent the player to it
//=============================================================================
function OnPlayerSpawn( player )
{
	player.RemoveAllItems();
	local idx = player.entindex();

	// Clean up old ball if it exists (respawn case)
	player.AcceptInput( "ClearParent", "", null, null );
	if ( idx in PlayerBalls && PlayerBalls[idx] != null && PlayerBalls[idx].IsValid() )
	{
		PlayerBalls[idx].Destroy();
		delete PlayerBalls[idx];
	}
	BallCounter++;
	local ballName = "rollerball_" + idx + "_" + BallCounter;

	// Spawn a rollermine at the player's position
	local ball = SpawnEntityFromTable( "prop_physics_override", {
		model = "models/roller.mdl",
		origin = player.GetOrigin() + Vector( 0, 0, 16 ),
		targetname = ballName,
		health = 0
	} );
	ball.AddEFlags( 128 ); // EFL_FORCE_CHECK_TRANSMIT

	// Track it
	PlayerBalls[idx] <- ball;
	SprintPower[idx] <- SPRINT_MAX;

	// Make the player invisible, non-solid
	player.SetDrawEnabled( false );
	player.ShowViewModel( false );
	player.SetSolid( 0 ); // SOLID_NONE
	player.SetMoveType( 0, 0 ); // MOVETYPE_NONE

	// Parent the player to the ball
	player.SetOrigin( ball.GetOrigin() );
	player.AcceptInput( "SetParent", ballName, ball, ball );
}

//=============================================================================
// Block weapon pickups
//=============================================================================
function CanPickupWeapon( player, classname, ownsWeapon )
{
	return false;
}

//=============================================================================
// No fall damage
//=============================================================================
function GetFallDamage( player, fallSpeed )
{
	return 0;
}

//=============================================================================
// Immediate respawn
//=============================================================================
function OnPlayerRespawn( player )
{
	return true;
}

//=============================================================================
// Per-tick: read buttons and apply forces to the ball
//=============================================================================
function OnThink()
{
	local dt = FrameTime();

	for ( local i = 1; i <= GetMaxPlayers(); i++ )
	{
		local player = GetPlayerByIndex( i );
		if ( player == null )
			continue;

		local idx = player.entindex();

		// If player is dead, clean up their ball immediately
		if ( !player.IsAlive() )
		{
			if ( idx in PlayerBalls && PlayerBalls[idx] != null && PlayerBalls[idx].IsValid() )
			{
				player.AcceptInput( "ClearParent", "", null, null );
				PlayerBalls[idx].Destroy();
			}
			if ( idx in PlayerBalls )
				delete PlayerBalls[idx];
			continue;
		}

		if ( !(idx in PlayerBalls) || PlayerBalls[idx] == null || !PlayerBalls[idx].IsValid() )
			continue;

		local ball = PlayerBalls[idx];
		local buttons = player.GetButtons();
		local fwd = player.GetEyeForward();

		// Sprint: boost force while holding IN_SPEED, drains power
		local sprinting = false;
		if ( (buttons & Constants.FButtons.IN_SPEED) && SprintPower[idx] > 0 )
		{
			sprinting = true;
			SprintPower[idx] <- SprintPower[idx] - SPRINT_DRAIN * dt;
			if ( SprintPower[idx] < 0 )
				SprintPower[idx] <- 0.0;
		}
		else
		{
			// Recharge when not sprinting
			SprintPower[idx] <- SprintPower[idx] + SPRINT_RECHARGE * dt;
			if ( SprintPower[idx] > SPRINT_MAX )
				SprintPower[idx] <- SPRINT_MAX;
		}
		local forceMul = sprinting ? SPRINT_MULTIPLIER : 1.0;

		// Forward movement
		if ( buttons & Constants.FButtons.IN_FORWARD )
		{
			local f = ROLL_FORCE * forceMul;
			ball.ApplyForceCenter( Vector( fwd.x * f, fwd.y * f, 0 ) );
		}

		// Backward movement
		if ( buttons & Constants.FButtons.IN_BACK )
		{
			local f = ROLL_FORCE_BACK * forceMul;
			ball.ApplyForceCenter( Vector( fwd.x * -f, fwd.y * -f, 0 ) );
		}

		// Jump (spacebar) with cooldown
		if ( buttons & Constants.FButtons.IN_JUMP )
		{
			local now = Time();
			if ( !(idx in JumpCooldowns) || JumpCooldowns[idx] < now )
			{
				ball.ApplyForceCenter( Vector( 0, 0, JUMP_FORCE ) );
				JumpCooldowns[idx] <- now + JUMP_COOLDOWN;
			}
		}
	}
}

//=============================================================================
// Clean up ball on disconnect
//=============================================================================
function OnPlayerDisconnect( player )
{
	local idx = player.entindex();
	if ( idx in PlayerBalls && PlayerBalls[idx] != null && PlayerBalls[idx].IsValid() )
		PlayerBalls[idx].Destroy();
	if ( idx in PlayerBalls )
		delete PlayerBalls[idx];
	if ( idx in SprintPower )
		delete SprintPower[idx];
}

//=============================================================================
// Welcome message
//=============================================================================
function OnPlayerConnect( player )
{
	local name = player.GetPlayerName();
	if ( name == "" )
		name = "<unconnected>";
	ClientPrint( null, Constants.EHudNotify.HUD_PRINTNOTIFY, name + " has joined the game\n" );

	ClientPrint( player, Constants.EHudNotify.HUD_PRINTTALK, "Welcome to Rollerball! WASD to roll, Space to jump, Shift for a speed boost.\n" );
}
