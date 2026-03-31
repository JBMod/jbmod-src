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
#include "vscript_shared.h"
#include "igamesystem.h"
#include "mapentities_shared.h"
#include "jbmod/jbmod_gamerules.h"
#include "vscript_server.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CServerScriptGameSystem : public CAutoGameSystem
{
public:
	CServerScriptGameSystem() : CAutoGameSystem( "CServerScriptGameSystem" ) {}

	virtual void LevelInitPreEntity()
	{
		// VScript may not be loaded yet (both are using CAutoGameSystem::LevelInitPreEntity)
		if ( !g_pScriptVM )
			VScriptServerInit();

		const char *pMapData = engine->GetMapEntitiesString();
		if ( !pMapData )
			return;

		char szWorkBuffer[2048];
		bool bFoundGamemode = false;
		char szGamemode[256];
		szGamemode[0] = '\0';

		while ( pMapData )
		{
			pMapData = MapEntity_SkipToNextEntity( pMapData, szWorkBuffer );

			char szClassname[MAX_PATH];
			if ( MapEntity_ExtractValue( szWorkBuffer, "classname", szClassname ) )
			{
				if ( !Q_stricmp( szClassname, "jbmod_logic_gamemode" ) )
				{
					if ( MapEntity_ExtractValue( szWorkBuffer, "gamemode", szGamemode ) )
					{
						bFoundGamemode = true;
						break;
					}
				}
			}
		}

		if ( bFoundGamemode && szGamemode[0] )
		{
			if ( JBModRules() )
			{
				Q_strncpy( JBModRules()->m_szGameMode, szGamemode, sizeof( JBModRules()->m_szGameMode ) );
			}

			char szScriptFile[256];
			Q_snprintf( szScriptFile, sizeof( szScriptFile ), "gamemodes/%s", szGamemode );
			VScriptRunScript( szScriptFile, true );
		}
		else
		{
			VScriptRunScript( "gamemodes/default", true );
		}
	}
};

static CServerScriptGameSystem g_ServerScriptGameSystem;
