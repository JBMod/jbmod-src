//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TE_JBMOD_SHOTGUN_SHOT_H
#define TE_JBMOD_SHOTGUN_SHOT_H
#ifdef _WIN32
#pragma once
#endif


void TE_JBModFireBullets( 
	int	iPlayerIndex,
	const Vector &vOrigin,
	const Vector &vDir,
	int	iAmmoID,
	int iSeed,
	int iShots,
	float flSpread, 
	bool bDoTracers,
	bool bDoImpacts );


#endif // TE_JBMOD_SHOTGUN_SHOT_H
