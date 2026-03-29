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
//#include "vscript_vgui.h"
#include "vscript_shared.h"
#include "igamesystem.h"
#include "icommandline.h"
#include "client_factorylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IScriptManager *scriptmanager = NULL;

class CScriptGameSystem : public CAutoGameSystem
{
public:
	CScriptGameSystem() : CAutoGameSystem( "CScriptGameSystem" ) {}

	virtual bool Init()
	{
		factorylist_t factories;
		FactoryList_Retrieve( factories );

		if ( factories.appSystemFactory && !CommandLine()->CheckParm( "-noscripting" ) )
		{
			scriptmanager = (IScriptManager *)factories.appSystemFactory( VSCRIPT_INTERFACE_VERSION, NULL );
		}

		return true;
	}

	virtual void LevelInitPostEntity()
	{
		if ( g_pScriptVM )
		{
			//g_pScriptVM->RegisterInstance( &g_ScriptVGUI, "vgui" );
		}
	}

	virtual void LevelShutdownPreEntity()
	{
		//g_ScriptVGUI.DestroyAllPanels();
	}
};

static CScriptGameSystem g_ScriptGameSystem;
