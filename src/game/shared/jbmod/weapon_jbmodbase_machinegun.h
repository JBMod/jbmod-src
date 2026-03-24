//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "weapon_jbmodbase.h"
#include "weapon_jbmodbasehlmpcombatweapon.h"

#ifndef BASEHLCOMBATWEAPON_H
#define BASEHLCOMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
	#define CJBModMachineGun C_JBModMachineGun
#endif

//=========================================================
// Machine gun base class
//=========================================================
class CJBModMachineGun : public CBaseJBModCombatWeapon
{
public:
	DECLARE_CLASS( CJBModMachineGun, CBaseJBModCombatWeapon );
	DECLARE_DATADESC();

	CJBModMachineGun();
	
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	PrimaryAttack( void );

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void	ItemPostFrame( void );
	virtual void	FireBullets( const FireBulletsInfo_t &info );
	virtual bool	Deploy( void );

	virtual const Vector &GetBulletSpread( void );

	int				WeaponSoundRealtime( WeaponSound_t shoot_type );

	// utility function
	static void DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime );

private:
	
	CJBModMachineGun( const CJBModMachineGun & );

protected:

	int	m_nShotsFired;	// Number of consecutive shots fired

	float	m_flNextSoundTime;	// real-time clock of when to make next sound
};

#endif // BASEHLCOMBATWEAPON_H
