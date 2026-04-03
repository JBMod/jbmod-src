//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef JBMOD_PLAYER_H
#define JBMOD_PLAYER_H
#pragma once

class CJBMod_Player;

#include "basemultiplayerplayer.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "jbmod_player_shared.h"
#include "jbmod_gamerules.h"
#include "utldict.h"

//=============================================================================
// >> JBMOD_Player
//=============================================================================
class CJBModPlayerStateInfo
{
public:
	JBModPlayerState m_iPlayerState;
	const char *m_pStateName;

	void (CJBMod_Player::*pfnEnterState)();	// Init and deinit the state.
	void (CJBMod_Player::*pfnLeaveState)();

	void (CJBMod_Player::*pfnPreThink)();	// Do a PreThink() in this state.
};

class CJBMod_Player : public CHL2_Player
{
public:
	DECLARE_CLASS( CJBMod_Player, CHL2_Player );

	CJBMod_Player();
	~CJBMod_Player( void );
	
	static CJBMod_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CJBMod_Player::s_PlayerEdict = ed;
		return (CJBMod_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void PostThink( void );
	virtual void PreThink( void );
	virtual void PlayerDeathThink( void );
	virtual void SetAnimation( PLAYER_ANIM playerAnim );
	virtual bool HandleCommand_JoinTeam( int team );
	virtual bool ClientCommand( const CCommand &args );
	virtual void CreateViewModel( int viewmodelindex = 0 );
	virtual bool BecomeRagdollOnClient( const Vector &force );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual bool WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;
	virtual void FireBullets ( const FireBulletsInfo_t &info );
	virtual void OnMyWeaponFired( CBaseCombatWeapon* weapon );
	virtual bool Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0);
	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );
	virtual void ChangeTeam( int iTeam ) OVERRIDE;
	virtual void PlayerUse( void );
	virtual void PickupObject ( CBaseEntity *pObject, bool bLimitMassAndSize );
	virtual void Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL );
	virtual void UpdateOnRemove( void );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual CBaseEntity* EntSelectSpawnPoint( void );
		
	int FlashlightIsOn( void );
	void FlashlightTurnOn( void );
	void FlashlightTurnOff( void );

	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles.Get(); }

	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );

	void CheatImpulseCommands( int iImpulse );
	void CreateRagdollEntity( void );

	void NoteWeaponFired( void );

	void ResetAnimation( void );
	void SetPlayerModel( void );
	Activity TranslateTeamActivity( Activity ActToTranslate );
	
	float GetNextModelChangeTime( void ) { return m_flNextModelChangeTime; }
	float GetNextTeamChangeTime( void ) { return m_flNextTeamChangeTime; }
	void  PickDefaultSpawnTeam( void );
	bool  CallScriptOnPlayerSpawn( void );

	void ScriptEquipSuit( void ) { EquipSuit(); }
	HSCRIPT ScriptGiveItem( const char *szItem ) { return ToHScript( GiveNamedItem( szItem ) ); }
	void ScriptGiveAmmo( int nCount, const char *szAmmoName ) { CBasePlayer::GiveAmmo( nCount, szAmmoName ); }
	void ScriptSwitchToWeapon( const char *szWeapon );
	const char *ScriptGetClientConVar( const char *szCvar );
	void ScriptRemoveAllItems( void ) { RemoveAllItems( true ); }
	int ScriptGetFragCount( void ) { return FragCount(); }
	void ScriptAddFrags( int nCount ) { IncrementFragCount( nCount ); }
	void ScriptAddTeamScore( int nScore );
	void ScriptSetPlayerModel( const char *szModel );
	void ScriptCommitSuicide( void ) { CommitSuicide(); }
	void ScriptForceRespawn( void ) { Spawn(); }
	const char *ScriptGetPlayerName( void ) { return GetPlayerName(); }
	bool ScriptIsAlive( void ) { return IsAlive(); }
	int ScriptGetDeathCount( void ) { return DeathCount(); }
	void ScriptAddDeaths( int nCount ) { IncrementDeathCount( nCount ); }
	int ScriptGetArmorValue( void ) { return ArmorValue(); }
	void ScriptSetArmorValue( int value ) { SetArmorValue( value ); }
	void ScriptShowMOTD( void );
	float ScriptGetDeathTime( void ) { return GetDeathTime(); }
	void ScriptSetMaxSpeed( float speed ) { SetMaxSpeed( speed ); }
	void ScriptSetMaxHealth( int health ) { SetMaxHealth( health ); if ( m_iHealth > health ) m_iHealth = health; }
	int ScriptGetButtons( void ) { return m_nButtons; }
	const Vector &ScriptGetEyeForward( void );
	void ScriptShowViewModel( bool bShow ) { ShowViewModel( bShow ); }

	int	GetMaxAmmo( int iAmmoIndex ) const;
	
	void  DetonateTripmines( void );

	void Reset();

	bool IsReady();
	void SetReady( bool bReady );

	void CheckChatText( char *p, int bufsize );

	void State_Transition( JBModPlayerState newState );
	void State_Enter( JBModPlayerState newState );
	void State_Leave();
	void State_PreThink();
	CJBModPlayerStateInfo *State_LookupInfo( JBModPlayerState state );

	void State_Enter_ACTIVE();
	void State_PreThink_ACTIVE();
	void State_Enter_OBSERVER_MODE();
	void State_PreThink_OBSERVER_MODE();


	virtual bool StartObserverMode( int mode );
	virtual void StopObserverMode( void );


	Vector m_vecTotalBulletForce;	//Accumulator for bullet force in a single frame

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 

	virtual bool	CanHearAndReadChatFrom( CBasePlayer *pPlayer );

	bool IsThreatAimingTowardMe( CBaseEntity* threat, float cosTolerance = 0.8f ) const;
	bool IsThreatFiringAtMe( CBaseEntity* threat ) const;
private:

	CNetworkQAngle( m_angEyeAngles );
	CPlayerAnimState   m_PlayerAnimState;

	int m_iLastWeaponFireUsercmd;
	CNetworkVar( int, m_iSpawnInterpCounter );

	float m_flNextModelChangeTime;
	float m_flNextTeamChangeTime;

	float m_flSlamProtectTime;	

	JBModPlayerState m_iPlayerState;
	CJBModPlayerStateInfo *m_pCurStateInfo;

	bool ShouldRunRateLimitedCommand( const CCommand &args );

	// This lets us rate limit the commands the players can execute so they don't overflow things like reliable buffers.
	CUtlDict<float,int>	m_RateLimitLastCommandTimes;

    bool m_bEnterObserver;
	bool m_bReady;
};

inline CJBMod_Player *ToJBModPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CJBMod_Player*>( pEntity );
}

#endif //JBMOD_PLAYER_H
