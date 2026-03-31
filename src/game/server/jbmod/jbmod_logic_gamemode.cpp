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

#include "cbase.h"
#include "jbmod_logic_gamemode.h"
#include "jbmod_gamerules.h"
#include "vscript_server.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CJBModLogicGamemode )
	DEFINE_KEYFIELD( m_iszGameMode, FIELD_STRING, "gamemode" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( jbmod_logic_gamemode, CJBModLogicGamemode );

CJBModLogicGamemode::CJBModLogicGamemode()
{
	m_iszGameMode = NULL_STRING;
}

void CJBModLogicGamemode::Activate()
{
	BaseClass::Activate();

	ApplySettings();
}

void CJBModLogicGamemode::ApplySettings()
{
	if ( !JBModRules() )
		return;

	const char *pszGameMode = ( m_iszGameMode != NULL_STRING ) ? STRING( m_iszGameMode ) : "";

	if ( pszGameMode && *pszGameMode )
	{
		if ( !Q_stricmp( JBModRules()->m_szGameMode, pszGameMode ) )
			return;

		Q_strncpy( JBModRules()->m_szGameMode, pszGameMode, 64 );

		char szScriptPath[256];
		Q_snprintf( szScriptPath, sizeof( szScriptPath ), "gamemodes/%s", pszGameMode );
		VScriptRunScript( szScriptPath );
	}
}
