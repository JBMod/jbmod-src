//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "jbmod_weapon_parse.h"
#include "ammodef.h"

FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CJBModSWeaponInfo;
}

CJBModSWeaponInfo::CJBModSWeaponInfo()
{
	m_iPlayerDamage = 0;
}

void CJBModSWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	m_iPlayerDamage = pKeyValuesData->GetInt( "damage", 0 );

#ifndef CLIENT_DLL
	const char *pszClassname = pKeyValuesData->GetString( "classname", "" );
	if ( pszClassname && szWeaponName && Q_stricmp( pszClassname, "weapon_scripted" ) == 0 && Q_stricmp( szWeaponName, "weapon_scripted" ) != 0 )
	{
		IEntityFactory *pFactory = EntityFactoryDictionary()->FindFactory( "weapon_scripted" );
		if ( pFactory && !EntityFactoryDictionary()->FindFactory( szWeaponName ) )
		{
			EntityFactoryDictionary()->InstallFactory( pFactory, szWeaponName );
			DevMsg( "CWeaponScripted: Registered alias '%s'\n", szWeaponName );
		}
	}
#endif
}


