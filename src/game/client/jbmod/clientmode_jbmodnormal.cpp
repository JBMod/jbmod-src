//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws the normal TF2 or HL2 HUD.
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "clientmode_jbmodnormal.h"
#include "vgui_int.h"
#include "hud.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include "iinput.h"
#include "jbmodclientscoreboard.h"
#include "jbmodtextwindow.h"
#include "ienginevgui.h"
#include "jbmod_achievements.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar fov_desired("fov_desired", "75", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sets the base field-of-view.", true, 75.0, true, 90.0);

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
vgui::HScheme g_hVGuiCombineScheme = 0;


// Instance the singleton and expose the interface to it.
IClientMode *GetClientModeNormal()
{
	static ClientModeJBModNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}

ClientModeJBModNormal* GetClientModeJBModNormal()
{
	Assert( dynamic_cast< ClientModeJBModNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeJBModNormal* >( GetClientModeNormal() );
}

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport
{
private:
	DECLARE_CLASS_SIMPLE( CHudViewport, CBaseViewport );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		gHUD.InitColors( pScheme );

		SetPaintBackgroundEnabled( false );
	}

	virtual IViewPortPanel *CreatePanelByName( const char *szPanelName );
};

int ClientModeJBModNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

IViewPortPanel* CHudViewport::CreatePanelByName( const char *szPanelName )
{
	IViewPortPanel* newpanel = NULL;

	if ( Q_strcmp( PANEL_SCOREBOARD, szPanelName) == 0 )
	{
		newpanel = new CJBModClientScoreBoardDialog( this );
		return newpanel;
	}
	else if ( Q_strcmp(PANEL_INFO, szPanelName) == 0 )
	{
		newpanel = new CJBModTextWindow( this );
		return newpanel;
	}
	else if ( Q_strcmp(PANEL_SPECGUI, szPanelName) == 0 )
	{
		newpanel = new CJBModSpectatorGUI( this );	
		return newpanel;
	}

	
	return BaseClass::CreatePanelByName( szPanelName ); 
}

//-----------------------------------------------------------------------------
// ClientModeHLNormal implementation
//-----------------------------------------------------------------------------
ClientModeJBModNormal::ClientModeJBModNormal()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeJBModNormal::~ClientModeJBModNormal()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeJBModNormal::Init()
{
	BaseClass::Init();

	// Load up the combine control panel scheme
	g_hVGuiCombineScheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/CombinePanelScheme.res", "CombineScheme" );
	if (!g_hVGuiCombineScheme)
	{
		Warning( "Couldn't load combine panel scheme!\n" );
	}

	CheckStartupAchievements();

	const char *cvarsToUncheat[] = {
		"cl_drawhud",
		"hidehud",
		NULL // End of list
	};
	UnCheatConVars( cvarsToUncheat );
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove FCVAR_CHEAT from ConVars at runtime
// We do this instead of changing the shared code to make any future merges
// easier, and because some cvars can be defined in the engine code instead.
//-----------------------------------------------------------------------------
struct ConCommandBase_Hack
{
	void* m_pVTable;
	ConCommandBase* m_pNext;
	bool m_bRegistered;
	const char* m_pszName;
	const char* m_pszHelpString;
	int m_nFlags;
};

void ClientModeJBModNormal::UnCheatConVars( const char **ppCVarNames )
{
	if ( !ppCVarNames )
		return;

	for ( int i = 0; ppCVarNames[i]; i++ )
	{
		ConVar *pVar = cvar->FindVar( ppCVarNames[i] );
		if ( pVar )
		{
			ConCommandBase_Hack *pHack = reinterpret_cast<ConCommandBase_Hack*>( pVar );
			pHack->m_nFlags &= ~FCVAR_CHEAT;
		}
	}
}



