//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basetempentity.h"


#define NUM_BULLET_SEED_BITS 8


//-----------------------------------------------------------------------------
// Purpose: Display's a blood sprite
//-----------------------------------------------------------------------------
class CTEJBModFireBullets : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEJBModFireBullets, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEJBModFireBullets( const char *name );
	virtual			~CTEJBModFireBullets( void );

public:
	CNetworkVar( int, m_iPlayer );
	CNetworkVector( m_vecOrigin );
	CNetworkVector( m_vecDir );
	CNetworkVar( int, m_iAmmoID );
	CNetworkVar( int, m_iSeed );
	CNetworkVar( int, m_iShots );
	CNetworkVar( float, m_flSpread );
	CNetworkVar( bool, m_bDoImpacts );
	CNetworkVar( bool, m_bDoTracers );
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
//-----------------------------------------------------------------------------
CTEJBModFireBullets::CTEJBModFireBullets( const char *name ) :
	CBaseTempEntity( name )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTEJBModFireBullets::~CTEJBModFireBullets( void )
{
}

IMPLEMENT_SERVERCLASS_ST_NOBASE(CTEJBModFireBullets, DT_TEJBModFireBullets)
	SendPropVector( SENDINFO(m_vecOrigin), -1, SPROP_COORD ),
	SendPropVector( SENDINFO(m_vecDir), -1 ),
	SendPropInt( SENDINFO( m_iAmmoID ), 5, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iSeed ), NUM_BULLET_SEED_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iShots ), 5, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iPlayer ), 6, SPROP_UNSIGNED ), 	// max 64 players, see MAX_PLAYERS
	SendPropFloat( SENDINFO( m_flSpread ), 10, 0, 0, 1 ),	
	SendPropBool( SENDINFO( m_bDoImpacts ) ),
	SendPropBool( SENDINFO( m_bDoTracers ) ),
END_SEND_TABLE()


// Singleton
static CTEJBModFireBullets g_TEJBModFireBullets( "Shotgun Shot" );


void TE_JBModFireBullets( 
	int	iPlayerIndex,
	const Vector &vOrigin,
	const Vector &vDir,
	int	iAmmoID,
	int iSeed,
	int iShots,
	float flSpread,
	bool bDoTracers,
	bool bDoImpacts )
{
	CPASFilter filter( vOrigin );
	filter.UsePredictionRules();

	g_TEJBModFireBullets.m_iPlayer = iPlayerIndex;
	g_TEJBModFireBullets.m_vecOrigin = vOrigin;
	g_TEJBModFireBullets.m_vecDir = vDir;
	g_TEJBModFireBullets.m_iSeed = iSeed;
	g_TEJBModFireBullets.m_iShots = iShots;
	g_TEJBModFireBullets.m_flSpread = flSpread;
	g_TEJBModFireBullets.m_iAmmoID = iAmmoID;
	g_TEJBModFireBullets.m_bDoTracers = bDoTracers;
	g_TEJBModFireBullets.m_bDoImpacts = bDoImpacts;
	
	Assert( iSeed < (1 << NUM_BULLET_SEED_BITS) );
	
	g_TEJBModFireBullets.Create( filter, 0 );
}
