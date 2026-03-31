//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Daslang VM interface for VScript system
//=============================================================================//

#ifndef IDASLANGMV_H
#define IDASLANGMV_H
#ifdef _WIN32
#pragma once
#endif

#include "ivscript.h"

// Forward declare the Daslang VM interface
class IDaslangVM : public IScriptVM
{
public:
    // Daslang-specific methods would go here
    // For now, it's just a marker interface to distinguish it from other VMs
};

#endif // IDASLANGMV_H