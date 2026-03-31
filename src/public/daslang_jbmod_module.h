//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Daslang module for JBMod game API bindings
//=============================================================================//

#ifndef DASLANG_JBMOD_MODULE_H
#define DASLANG_JBMOD_MODULE_H
#ifdef _WIN32
#pragma once
#endif

// Forward declarations for Daslang types
namespace das
{
    class ModuleLibrary;
}

// Forward declare our module class
class Daslang_JBMod_Module;

// Function to create and register the module
void RegisterDaslangJBModModule(das::ModuleLibrary& lib);

#endif // DASLANG_JBMOD_MODULE_H