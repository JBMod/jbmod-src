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

local FORCE = 500.0
local FORCE_MULTIPLIER = 4.0
local RANGE = 512.0

function PrimaryAttack()
{
    if (SERVER) // No prediction for now
    {
        self.EmitSound( "Weapon_Crossbow.BoltFly" )
        Blow(FORCE)
    }
}

function SecondaryAttack()
{
    if (SERVER) // No prediction for now
    {
        self.EmitSound( "Weapon_PhysCannon.Launch" )
        Blow( FORCE * FORCE_MULTIPLIER )
    }
}

function Blow(force)
{
    if ( owner == null ) return

    local shootPos = owner.Weapon_ShootPosition()
    local forward = owner.GetForwardVector()

    local trace = {
        start = shootPos,
        end   = shootPos + forward * 16384,
        hullmin = Vector( -8, -8, -8 ),
        hullmax = Vector( 8, 8, 8 ),
        mask  = 0x4600400b,
        ignore = owner
    }

    TraceHull( trace )

    if ( !("enthit" in trace) || trace.enthit == null )
        return

    local ent = trace.enthit
    if ( !ent.IsPhysicsObject() )
        return

    local delta = owner.GetCenter() - ent.GetCenter()
    local dist = delta.Length()
    local ratio = 1.0 - (dist / RANGE)
    if (ratio < 0) ratio = 0
    if (ratio > 1) ratio = 1

    local appliedForce = forward * force * ratio
    ent.ApplyForceCenter( appliedForce )
}
