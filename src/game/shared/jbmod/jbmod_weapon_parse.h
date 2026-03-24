//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef JBMOD_WEAPON_PARSE_H
#define JBMOD_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"


//--------------------------------------------------------------------------------------------------------
class CJBModSWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CJBModSWeaponInfo, FileWeaponInfo_t );
	
	CJBModSWeaponInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );


public:

	int m_iPlayerDamage;
};


#endif // JBMOD_WEAPON_PARSE_H
