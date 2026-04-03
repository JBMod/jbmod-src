//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_jbmodbasehlmpcombatweapon.h"
#include "jbmod_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "jbmod_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "jbmod_gamerules.h"
#include "KeyValues.h"
#include "viewport_panel_names.h"
#include "team.h"
#include "weapon_jbmodbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "gamestats.h"
#include "ammodef.h"
#include "NextBot.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"
#include "vscript_server.h"


extern CBaseEntity				*g_pLastSpawn;

ConVar jbmod_spawn_frag_fallback_radius( "jbmod_spawn_frag_fallback_radius", "48", FCVAR_NONE, "If no spawns are available, kill players with this radius to allow new players to spawn." );

#define JBMOD_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CJBMod_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CJBMod_Player );

// specific to the local player
BEGIN_SEND_TABLE_NOBASE( CJBMod_Player, DT_JBModLocalPlayerExclusive )
	// send a hi-res origin to the local player for use in prediction
	SendPropVectorXY(SENDINFO(m_vecOrigin),               -1, SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginXY ),
	SendPropFloat   (SENDINFO_VECTORELEM(m_vecOrigin, 2), -1, SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginZ ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

END_SEND_TABLE()

// all players except the local player
BEGIN_SEND_TABLE_NOBASE( CJBMod_Player, DT_JBModNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVectorXY(SENDINFO(m_vecOrigin),               -1, SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginXY ),
	SendPropFloat   (SENDINFO_VECTORELEM(m_vecOrigin, 2), -1, SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginZ ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST(CJBMod_Player, DT_JBMod_Player)
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),

	// misyl:
	// m_flMaxspeed is fully predicted by the client and the client's
	// maxspeed is sent in the user message.
	// Other games like DOD, etc don't use this var at all and just fully
	// predict in GameMovement, but the HL2 codebase doesn't do that and modifies this
	// on the player.
	// So, just never send it, and don't predict it on the client either.
	SendPropExclude( "DT_BasePlayer", "m_flMaxspeed" ),


	// Data that only gets sent to the local player
	SendPropDataTable( "jbmodlocaldata", 0, &REFERENCE_SEND_TABLE( DT_JBModLocalPlayerExclusive ), SendProxy_SendLocalDataTable ),

	// Data that gets sent to all other players
	SendPropDataTable( "jbmodnonlocaldata", 0, &REFERENCE_SEND_TABLE( DT_JBModNonLocalPlayerExclusive ), SendProxy_SendNonLocalDataTable ),

	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),	
END_SEND_TABLE()

BEGIN_DATADESC( CJBMod_Player )
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CJBMod_Player, CHL2_Player, "JBMod Player" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptEquipSuit, "EquipSuit", "Give the player the HEV suit." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGiveItem, "GiveItem", "Give the player a specific item by classname." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGiveAmmo, "GiveAmmo", "Give the player a specific amount of ammo (count, ammoName)." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSwitchToWeapon, "SwitchToWeapon", "Switch to a weapon the player owns by classname." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetClientConVar, "GetClientConVar", "Read a client ConVar value by name." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptRemoveAllItems, "RemoveAllItems", "Strip all weapons and suit from the player." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetFragCount, "GetFragCount", "Get the player's frag count." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptAddFrags, "AddFrags", "Add to the player's frag count." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptAddTeamScore, "AddTeamScore", "Add to this player's team score." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetPlayerModel, "SetPlayerModel", "Set player model." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptCommitSuicide, "CommitSuicide", "Kill the player." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptForceRespawn, "ForceRespawn", "Force the player to respawn immediately." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetPlayerName, "GetPlayerName", "Get the player's name." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptIsAlive, "IsAlive", "Returns true if the player is alive." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetDeathCount, "GetDeathCount", "Get the player's death count." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptAddDeaths, "AddDeaths", "Add to the player's death count." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetArmorValue, "GetArmorValue", "Get the player's armor value." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetArmorValue, "SetArmorValue", "Set the player's armor value." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptShowMOTD, "ShowMOTD", "Show the Message of the Day panel to this player." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetDeathTime, "GetDeathTime", "Get the time this player last died." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetMaxSpeed, "SetMaxSpeed", "Set the player's maximum movement speed." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetMaxHealth, "SetMaxHealth", "Set the player's maximum health." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetButtons, "GetButtons", "Get the player's currently pressed buttons (bitmask)." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetEyeForward, "GetEyeForward", "Get the player's eye forward direction vector." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptShowViewModel, "ShowViewModel", "Show or hide the player's viewmodel." )
END_SCRIPTDESC();

ConVar sv_model_change_interval( "sv_model_change_interval", "5.0", FCVAR_GAMEDLL, "Minimum seconds between player model changes." );
#define TEAM_CHANGE_INTERVAL 5.0f

#define JBMODPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CJBMod_Player::CJBMod_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	BaseClass::ChangeTeam( 0 );
	
	//UseClientSideAnimation();
}

CJBMod_Player::~CJBMod_Player( void )
{

}

void CJBMod_Player::UpdateOnRemove( void )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerDisconnect" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL, GetScriptInstance() );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}

	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CJBMod_Player::Precache( void )
{
	BaseClass::Precache();

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPrecache" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}
}

void CJBMod_Player::PickDefaultSpawnTeam( void )
{
	if ( GetTeamNumber() == 0 )
	{
		if ( g_pScriptVM )
		{
			HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerPickTeam" );
			if ( hFunction )
			{
				ScriptVariant_t result;
				g_pScriptVM->Call( hFunction, NULL, true, &result, GetScriptInstance() );
				g_pScriptVM->ReleaseFunction( hFunction );

				if ( result.GetType() == FIELD_INTEGER )
				{
					ChangeTeam( (int)result );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CJBMod_Player::Spawn(void)
{
	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	PickDefaultSpawnTeam();

	BaseClass::Spawn();
	
	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );

		CallScriptOnPlayerSpawn();
	}

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = JBMODPLAYER_PHYSDAMAGE_SCALE;

	if ( JBModRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;
}

void CJBMod_Player::PlayerUse( void )
{
	// Block +use when player is invisible (e.g. Rollerball mode)
	if ( IsEffectActive( EF_NODRAW ) )
		return;

	BaseClass::PlayerUse();
}
ConVar jbmod_allow_pickup( "jbmod_allow_pickup", "0", FCVAR_GAMEDLL );

void CJBMod_Player::PickupObject( CBaseEntity* pObject, bool bLimitMassAndSize )
{
	if ( !jbmod_allow_pickup.GetBool() )
		return;

	return BaseClass::PickupObject( pObject, bLimitMassAndSize );
}

void CJBMod_Player::SetPlayerModel( void )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerSetModel" );
		if ( hFunction )
		{
			const char *pRequestedModel = engine->GetClientConVarValue( entindex(), "cl_playermodel" );
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, GetScriptInstance(), pRequestedModel );
			g_pScriptVM->ReleaseFunction( hFunction );

			const char *pModelStr = (const char *)result;
			if ( result.GetType() == FIELD_CSTRING )
				ScriptSetPlayerModel( pModelStr );
		}
	}
}

void CJBMod_Player::ScriptShowMOTD( void )
{
	const ConVar *hostname = cvar->FindVar( "hostname" );
	const char *title = (hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

	KeyValues *data = new KeyValues("data");
	data->SetString( "title", title );
	data->SetString( "type", "1" );
	data->SetString( "msg", "motd" );
	data->SetBool( "unload", true );

	ShowViewPortPanel( PANEL_INFO, true, data );

	data->deleteThis();
}

void CJBMod_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
		else if ( ( GetFlags() & FL_ONGROUND ) != FL_ONGROUND)
			SetAnimation( PLAYER_JUMP );
		else
			SetAnimation( PLAYER_IDLE );
	}
}


bool CJBMod_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CJBMod_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

void CJBMod_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
}

void CJBMod_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}
}

void CJBMod_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponJBModBase *pWeapon = dynamic_cast<CWeaponJBModBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetJBModWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );

	if ( pWeapon )
		this->OnMyWeaponFired( pWeapon );
}

void CJBMod_Player::OnMyWeaponFired( CBaseCombatWeapon* weapon )
{
	BaseClass::OnMyWeaponFired( weapon );

	TheNextBots().OnWeaponFired( this, weapon );
}

void CJBMod_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CJBMod_Player::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pPlayer->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CJBMod_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	const char *pModelName = STRING( GetModelName() );
	if ( Q_stristr( pModelName, "combine" ) || Q_stristr( pModelName, "police" ) )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CJBMod_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					/*
					if ( bRunning == false )
					{
						idealActivity = ACT_WALK;
					}
					else
					*/
					{
						idealActivity = ACT_HL2MP_RUN;
					}
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		//
		// misyl: it was and was causing a pred error every time.
		// the weapons already call SendWeaponAnim with the right activity.
		//Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CJBMod_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CJBMod_Player::ChangeTeam( int iTeam )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerChangeTeam" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, GetScriptInstance(), iTeam );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_BOOLEAN && !(bool)result )
				return;
		}
	}

	bool bKill = false;

	if ( JBModRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR )
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	if ( JBModRules()->IsTeamplay() == true )
	{
		if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	SetPlayerModel();

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}

	if ( bKill == true )
	{
		CommitSuicide();
	}
}

bool CJBMod_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() && !IsHLTV() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam( team );

	return true;
}

bool CJBMod_Player::ClientCommand( const CCommand &args )
{
	if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if ( FStrEq( args[0], "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( args ) )
		{
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		return true;
	}
	else if ( FStrEq( args[0], "joingame" ) )
	{
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CJBMod_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() && g_pScriptVM )
				{
					HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnGiveAllItems" );
					if ( hFunction )
					{
						g_pScriptVM->Call( hFunction, NULL, true, NULL, GetScriptInstance() );
						g_pScriptVM->ReleaseFunction( hFunction );
					}
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CJBMod_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < JBMOD_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CJBMod_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CJBMod_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CJBModRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CJBModRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( jbmod_ragdoll, CJBModRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CJBModRagdoll, DT_JBModRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CJBMod_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CJBModRagdoll *pRagdoll = dynamic_cast< CJBModRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CJBModRagdoll* >( CreateEntityByName( "jbmod_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CJBMod_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CJBMod_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CJBMod_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CJBMod_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}

int CJBMod_Player::GetMaxAmmo( int iAmmoIndex ) const
{
	if ( iAmmoIndex == -1 )
		return 0;

	if ( GetAmmoDef()->MaxCarry( iAmmoIndex ) == INFINITE_AMMO )
		return 999;

	return GetAmmoDef()->MaxCarry( iAmmoIndex );
}

void CJBMod_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

void CJBMod_Player::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	if ( !IsEffectActive( EF_NODRAW ) )
	{
		CreateRagdollEntity();
	}

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerKilled" );
		if ( hFunction )
		{
			g_pScriptVM->Call( hFunction, NULL, true, NULL,
				GetScriptInstance(),
				ToHScript( pAttacker ) );
			g_pScriptVM->ReleaseFunction( hFunction );
		}
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();
}

int CJBMod_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//return here if the player is in the respawn grace period vs. slams.
	if ( gpGlobals->curtime < m_flSlamProtectTime &&  (inputInfo.GetDamageType() == DMG_BLAST ) )
		return 0;

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerTakeDamage" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result,
				GetScriptInstance(),
				ToHScript( inputInfo.GetAttacker() ),
				inputInfo.GetDamage() );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_FLOAT || result.GetType() == FIELD_INTEGER )
			{
				float flNewDamage = (float)result;
				if ( flNewDamage < 0 )
					return 0;

				CTakeDamageInfo modifiedInfo = inputInfo;
				modifiedInfo.SetDamage( flNewDamage );

				m_vecTotalBulletForce += modifiedInfo.GetDamageForce();
				gamestats->Event_PlayerDamage( this, modifiedInfo );
				return BaseClass::OnTakeDamage( modifiedInfo );
			}
		}
	}

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	gamestats->Event_PlayerDamage( this, inputInfo );

	return BaseClass::OnTakeDamage( inputInfo );
}

void CJBMod_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	const char *pSoundName = NULL;

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerDeathSound" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, GetScriptInstance() );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_CSTRING )
				pSoundName = (const char *)result;
		}
	}

	if ( !pSoundName || !pSoundName[0] )
		return;

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( pSoundName, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CJBMod_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "GetSpawnPointClassname" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, GetScriptInstance() );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_CSTRING && (const char *)result != NULL && ((const char *)result)[0] )
			{
				pSpawnpointName = (const char *)result;
			}
		}
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), jbmod_spawn_frag_fallback_radius.GetFloat() ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	g_pLastSpawn = pSpot;

	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CJBMod_Player *pPlayer = ToJBModPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)JBModRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CJBMod_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CJBMod_Player::IsReady()
{
	return m_bReady;
}

void CJBMod_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

bool CJBMod_Player::CallScriptOnPlayerSpawn( void )
{
	if ( g_pScriptVM == NULL )
		return false;

	HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerSpawn" );
	if ( hFunction )
	{
		g_pScriptVM->Call( hFunction, NULL, true, NULL, GetScriptInstance() );
		g_pScriptVM->ReleaseFunction( hFunction );
		return true;
	}
	return false;
}

void CJBMod_Player::ScriptSwitchToWeapon( const char *szWeapon )
{
	CBaseCombatWeapon *pWeapon = Weapon_OwnsThisType( szWeapon );
	if ( pWeapon )
	{
		Weapon_Switch( pWeapon );
	}
}

const char *CJBMod_Player::ScriptGetClientConVar( const char *szCvar )
{
	if ( !szCvar || !szCvar[0] )
		return "";

	return engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), szCvar );
}

void CJBMod_Player::ScriptAddTeamScore( int nScore )
{
	CTeam *pTeam = GetTeam();
	if ( pTeam )
	{
		pTeam->AddScore( nScore );
	}
}

void CJBMod_Player::ScriptSetPlayerModel( const char *szModel )
{
	if ( !szModel || !szModel[0] || !Q_stristr( szModel, ".mdl" ) )
		return;

	SetModel( szModel );
	m_flNextModelChangeTime = gpGlobals->curtime + sv_model_change_interval.GetFloat();
}

void CJBMod_Player::CheckChatText( char *p, int bufsize )
{
	if ( g_pScriptVM )
	{
		HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnPlayerChat" );
		if ( hFunction )
		{
			ScriptVariant_t result;
			g_pScriptVM->Call( hFunction, NULL, true, &result, GetScriptInstance(), p );
			g_pScriptVM->ReleaseFunction( hFunction );

			if ( result.GetType() == FIELD_BOOLEAN && !(bool)result )
			{
				p[0] = '\0';
				return;
			}
			else if ( result.GetType() == FIELD_CSTRING )
			{
				Q_strncpy( p, (const char *)result, bufsize );
			}
		}
	}

	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	JBModRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CJBMod_Player::State_Transition( JBModPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CJBMod_Player::State_Enter( JBModPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CJBMod_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CJBMod_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CJBModPlayerStateInfo *CJBMod_Player::State_LookupInfo( JBModPlayerState state )
{
	// This table MUST match the 
	static CJBModPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CJBMod_Player::State_Enter_ACTIVE, NULL, &CJBMod_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CJBMod_Player::State_Enter_OBSERVER_MODE,	NULL, &CJBMod_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

const Vector &CJBMod_Player::ScriptGetEyeForward( void )
{
	static Vector vecForward;
	AngleVectors( pl.v_angle, &vecForward );
	return vecForward;
}

bool CJBMod_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CJBMod_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CJBMod_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CJBMod_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CJBMod_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CJBMod_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CJBMod_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}

//-----------------------------------------------------------------------------------------------------
// Return true if the given threat is aiming in our direction
bool CJBMod_Player::IsThreatAimingTowardMe( CBaseEntity* threat, float cosTolerance ) const
{
	CJBMod_Player* player = ToJBModPlayer( threat );
	Vector to = GetAbsOrigin() - threat->GetAbsOrigin();
	Vector forward;

	if ( player == NULL )
	{
		return false;
	}

	// is the player pointing at me?
	player->EyeVectors( &forward );

	if ( DotProduct( to, forward ) > cosTolerance )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------
// Return true if the given threat is aiming in our direction and firing its weapon
bool CJBMod_Player::IsThreatFiringAtMe( CBaseEntity* threat ) const
{
	if ( IsThreatAimingTowardMe( threat ) )
	{
		CJBMod_Player* player = ToJBModPlayer( threat );

		if ( player )
		{
			return player->IsFiringWeapon();
		}
	}

	return false;
}
