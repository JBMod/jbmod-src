//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( CLIENTMODE_HLNORMAL_H )
#define CLIENTMODE_HLNORMAL_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui/Cursor.h>

class CHudViewport;


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class ClientModeJBModNormal : public ClientModeShared
{
public:
	DECLARE_CLASS( ClientModeJBModNormal, ClientModeShared );

	ClientModeJBModNormal();
	~ClientModeJBModNormal();

	virtual void	Init();
	virtual int		GetDeathMessageStartHeight( void );

	void			UnCheatConVars( const char **ppCVarNames );
};

extern IClientMode *GetClientModeNormal();
extern vgui::HScheme g_hVGuiCombineScheme;

extern ClientModeJBModNormal* GetClientModeJBModNormal();

#endif // CLIENTMODE_HLNORMAL_H
