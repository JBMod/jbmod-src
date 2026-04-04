//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Daslang module for JBMod game API bindings
//=============================================================================//

#include "daslang_jbmod_module.h"
#include "cbase.h"
#include "tier0/memdbgon.h"

// Include actual Daslang headers for binding
#include <daScript/daScript.h>
#include <daScript/module/built_in.h>
#include <daScript/module/math.h>
#include <daScript/module/strings.h>
#include <daScript/module/rtti.h>

// Helper functions for Daslang bindings
namespace DaslangJBMod
{
    // Simple test function to verify bindings work
    int32_t AddInts(int32_t a, int32_t b) { return a + b; }
    
    // Example of binding an engine function (placeholder)
    float GetServerTime() { return 0.0f; }
    
    // Example of binding a player-related function (placeholder)
    int GetPlayerCount() { return 0; }
}

// Implementation of the module registration function
void RegisterDaslangJBModModule(das::ModuleLibrary& lib)
{
    using namespace das;
    
    // Bind a simple test function
    lib->addExtern<DaslangJBMod::AddInts>("add_ints", SideEffects::none, "Adds two integers")
        ->args("a", "b");
    
    // Bind the server time function
    lib->addExtern<DaslangJBMod::GetServerTime>("server_time", SideEffects::none, "Get current server time")
        ->args();
    
    // Bind the player count function
    lib->addExtern<DaslangJBMod::GetPlayerCount>("player_count", SideEffects::none, "Get number of players")
        ->args();
    
    // TODO: Add more JBMod-specific bindings as needed:
    // - Entity functions (e.g., EntIndexToHScript)
    // - Vector math functions
    // - String utility functions
    // - Game event functions
    // etc.
}

#endif // DASLANG_JBMOD_MODULE_H