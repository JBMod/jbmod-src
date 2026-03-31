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

#include "cbase.h"
#include "weapon_scripted.h"
#include "filesystem.h"
#include "weapon_parse.h"
#include "vscript/ivscript.h"
#include "vscript_shared.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponScripted, DT_WeaponScripted )

BEGIN_NETWORK_TABLE( CWeaponScripted, DT_WeaponScripted )
#ifdef CLIENT_DLL
	RecvPropString( RECVINFO( m_szScriptedWeaponName ) ),
#else
	SendPropString( SENDINFO( m_szScriptedWeaponName ) ),
#endif
END_NETWORK_TABLE()

#if defined( CLIENT_DLL )
BEGIN_PREDICTION_DATA( CWeaponScripted )
	DEFINE_PRED_FIELD( m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flNextSecondaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nNextThinkTick, FIELD_INTEGER, FTYPEDESC_INSENDTABLE )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_scripted, CWeaponScripted );

#if !defined( CLIENT_DLL )
BEGIN_DATADESC( CWeaponScripted )
END_DATADESC()
#endif

BEGIN_ENT_SCRIPTDESC( CWeaponScripted, CBaseCombatWeapon, "JBMod Scripted Weapon" )
END_SCRIPTDESC();

CWeaponScripted::CWeaponScripted()
{
	m_szVScriptName[0] = '\0';
	m_szScriptedWeaponName.GetForModify()[0] = '\0';
	m_bScriptLoaded = false;
	m_ScriptScope = NULL;
}

void CWeaponScripted::Precache( void )
{
#ifndef CLIENT_DLL
	BaseClass::Precache();
	Q_strncpy( m_szScriptedWeaponName.GetForModify(), GetClassname(), MAX_WEAPON_STRING );

	FileWeaponInfo_t *pInfo = GetFileWeaponInfoFromHandle( m_hWeaponFileInfo );
	if ( pInfo && pInfo->szAmmo1[0] == 0 && pInfo->iMaxClip1 == -1 )
	{
		pInfo->m_bMeleeWeapon = true;
	}
#else
	const char *pszWeaponName = m_szScriptedWeaponName;
	if ( !pszWeaponName || !pszWeaponName[0] )
		return;

	m_iPrimaryAmmoType = m_iSecondaryAmmoType = -1;
	if ( ReadWeaponDataFromFileForSlot( filesystem, pszWeaponName, &m_hWeaponFileInfo, GetEncryptionKey() ) )
	{
		if ( GetWpnData().szAmmo1[0] )
		{
			m_iPrimaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo1 );
		}
		if ( GetWpnData().szAmmo2[0] )
		{
			m_iSecondaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo2 );
		}

		FileWeaponInfo_t *pInfo = GetFileWeaponInfoFromHandle( GetWeaponFileInfoHandle() );
		if ( pInfo && m_iPrimaryAmmoType == -1 && pInfo->iMaxClip1 == -1 )
		{
			pInfo->m_bMeleeWeapon = true;
		}

		gWR.LoadWeaponSprites( GetWeaponFileInfoHandle() );

		m_iViewModelIndex = 0;
		m_iWorldModelIndex = 0;
		if ( GetViewModel() && GetViewModel()[0] )
		{
			m_iViewModelIndex = CBaseEntity::PrecacheModel( GetViewModel() );
		}
		if ( GetWorldModel() && GetWorldModel()[0] )
		{
			m_iWorldModelIndex = CBaseEntity::PrecacheModel( GetWorldModel() );
		}

		for ( int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i )
		{
			const char *shootsound = GetShootSound( i );
			if ( shootsound && shootsound[0] )
			{
				CBaseEntity::PrecacheScriptSound( shootsound );
			}
		}
	}
	else
	{
		Warning( "CWeaponScripted: Error reading weapon data file for: %s\n", pszWeaponName );
	}
#endif

	const FileWeaponInfo_t &info = GetWpnData();
	if ( info.bParsedScript )
	{
		char szFullName[512];
		Q_snprintf( szFullName, sizeof(szFullName), "scripts/%s", info.szClassName );

		KeyValues *pKV = ReadEncryptedKVFile( filesystem, szFullName, NULL );
		if ( pKV )
		{
			const char *pszVScript = pKV->GetString( "vscript", NULL );
			if ( pszVScript && *pszVScript )
			{
				Q_strncpy( m_szVScriptName, pszVScript, sizeof(m_szVScriptName) );
				DevMsg( "CWeaponScripted: Found vscript '%s' for weapon '%s'\n", m_szVScriptName, info.szClassName );
			}
			pKV->deleteThis();
		}
	}
}

void CWeaponScripted::Spawn( void )
{
#ifndef CLIENT_DLL
	Q_strncpy( m_szScriptedWeaponName.GetForModify(), GetClassname(), MAX_WEAPON_STRING );
#endif
	BaseClass::Spawn();

	LoadVScript();
}

#if defined( CLIENT_DLL )
void CWeaponScripted::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		Precache();
		LoadVScript();
	}
}
#endif

void CWeaponScripted::LoadVScript( void )
{
	if ( m_bScriptLoaded || !g_pScriptVM )
		return;

	if ( !m_szVScriptName[0] )
	{
		return;
	}

	if ( !ValidateScriptScope() )
	{
		Warning( "CWeaponScripted: Failed to create script scope for '%s'\n", GetClassname() );
		return;
	}

	HSCRIPT hScript = VScriptCompileScript( m_szVScriptName, true );
	if ( !hScript )
	{
		Warning( "CWeaponScripted: Failed to compile script '%s'\n", m_szVScriptName );
		return;
	}

	if ( g_pScriptVM->Run( hScript, m_ScriptScope ) == SCRIPT_ERROR )
	{
		Warning( "CWeaponScripted: Failed to run script '%s'\n", m_szVScriptName );
		return;
	}

	m_bScriptLoaded = true;
	DevMsg( "CWeaponScripted [%s]: Loaded vscript '%s' for weapon '%s'\n", 
		(IsClient() ? "Client" : "Server"), m_szVScriptName, GetClassname() );
}

bool CWeaponScripted::CallScriptFunction( const char *pszFunc )
{
	if ( !m_bScriptLoaded || !g_pScriptVM || !m_ScriptScope )
		return false;

	HSCRIPT hFunc = g_pScriptVM->LookupFunction( pszFunc, m_ScriptScope );
	if ( !hFunc )
		return false;

	g_pScriptVM->SetValue( m_ScriptScope, "self", this->GetScriptInstance() );

	CBaseEntity *pOwner = GetOwner();
	if ( pOwner )
	{
		g_pScriptVM->SetValue( m_ScriptScope, "owner", pOwner->GetScriptInstance() );
	}

	g_pScriptVM->Call( hFunc, m_ScriptScope );
	g_pScriptVM->ReleaseFunction( hFunc );
	return true;
}

bool CWeaponScripted::ValidateScriptScope( void )
{
	if ( !g_pScriptVM )
		return false;

	if ( m_ScriptScope )
		return true;

	m_ScriptScope = g_pScriptVM->CreateScope( GetClassname(), NULL );
	if ( !m_ScriptScope )
		return false;

	return true;
}

void CWeaponScripted::PrimaryAttack( void )
{
	CallScriptFunction( "PrimaryAttack" );
}

void CWeaponScripted::SecondaryAttack( void )
{
	CallScriptFunction( "SecondaryAttack" );
}

void CWeaponScripted::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CallScriptFunction( "Think" );
}
