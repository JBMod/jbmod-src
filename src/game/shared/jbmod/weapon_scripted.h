// Copyright 2025 The JBMod Authors
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

#ifndef WEAPON_SCRIPTED_H
#define WEAPON_SCRIPTED_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_jbmodbasehlmpcombatweapon.h"
#include "vscript/ivscript.h"
#include "vscript_shared.h"

#if defined( CLIENT_DLL )
#define CWeaponScripted C_WeaponScripted
#endif

class CWeaponScripted : public CBaseJBModCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponScripted, CBaseJBModCombatWeapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
	DECLARE_ENT_SCRIPTDESC();

	CWeaponScripted();

	virtual void	Precache( void );
	virtual void	Spawn( void );
#if defined( CLIENT_DLL )
	virtual void	OnDataChanged( DataUpdateType_t type );
#endif
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual void	ItemPostFrame( void );

	void			LoadVScript( void );
	bool			CallScriptFunction( const char *pszFunc );
	bool			ValidateScriptScope( void );

private:
	char			m_szVScriptName[MAX_WEAPON_STRING];
	CNetworkString( m_szScriptedWeaponName, MAX_WEAPON_STRING );
	bool			m_bScriptLoaded;
	HSCRIPT			m_ScriptScope;
};

#endif // WEAPON_SCRIPTED_H
