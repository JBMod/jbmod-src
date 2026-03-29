//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_JBMODBASE_H
#define WEAPON_JBMODBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "jbmod_player_shared.h"
#include "basecombatweapon_shared.h"
#include "jbmod_weapon_parse.h"
#ifndef CLIENT_DLL
#include "ai_basenpc.h"
#endif

#if defined( CLIENT_DLL )
	#define CWeaponJBModBase C_WeaponJBModBase
	void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip );
#endif

class CJBMod_Player;

// These are the names of the ammo types that go in the CAmmoDefs and that the 
// weapon script files reference.

// Given an ammo type (like from a weapon's GetPrimaryAmmoType()), this compares it
// against the ammo name you specify.
// MIKETODO: this should use indexing instead of searching and strcmp()'ing all the time.
bool IsAmmoType( int iAmmoType, const char *pAmmoName );

class CWeaponJBModBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponJBModBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponJBModBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	
		void SendReloadSoundEvent( void );

		void Materialize( void );
		virtual	int	ObjectCaps( void );
	#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const;

	CBasePlayer* GetPlayerOwner() const;
	CJBMod_Player* GetJBModPlayerOwner() const;

	void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );
	
	CJBModSWeaponInfo const	&GetJBModWpnData() const;


	virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual void FallInit( void );
	
public:
	#if defined( CLIENT_DLL )
		
		virtual bool	ShouldPredict();
		virtual void	OnDataChanged( DataUpdateType_t type );

		virtual bool	OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options );

	#else

		virtual void	Spawn();

	#endif

	float		m_flPrevAnimTime;
	float  m_flNextResetCheckTime;

	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin;	}
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles;	}
	void	SetOriginalSpawnOrigin( const Vector& origin ) { m_vOriginalSpawnOrigin = origin; }
	void	SetOriginalSpawnAngles( const QAngle& angles ) { m_vOriginalSpawnAngles = angles; }

private:

	CWeaponJBModBase( const CWeaponJBModBase & );

	Vector m_vOriginalSpawnOrigin;
	QAngle m_vOriginalSpawnAngles;
};


#endif // WEAPON_JBMODBASE_H
