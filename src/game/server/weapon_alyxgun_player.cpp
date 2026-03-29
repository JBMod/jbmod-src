//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Alyx Gun for the player
// //TODO: Burst fire with secondary attack (couldnt get it to work)
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_JBMod_player.h"
#else
#include "JBMod_player.h"
#endif

#include "weapon_jbmodbasehlmpcombatweapon.h"
#include "dt_send.h"

// Modify this to alter the rate of fire
#define ROF 0.1f

// The gun will fire up to this number of bullets while you hold the fire button
// If you set it to 1 the gun will be semi-auto. If you set it to 3 the gun will fire three-round bursts
#define BURST 500;

#ifdef CLIENT_DLL
#define CWeaponAlyxGun_P C_WeaponAlyxGun_P
#endif

//-----------------------------------------------------------------------------
// CWeaponAlyxGun_P
//-----------------------------------------------------------------------------
class CWeaponAlyxGun_P : public CBaseJBModCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponAlyxGun_P, CBaseJBModCombatWeapon);

	CWeaponAlyxGun_P();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	void			Precache();
	void			ItemPostFrame();
	void			ItemPreFrame();
	void			PrimaryAttack();
	void			AddViewKick();
	void			DryFire();
	void			GetStance();
	Activity		GetPrimaryAttackActivity();

	virtual bool	Reload();

	int				GetMinBurst() { return 2; }
	int				GetMaxBurst() { return 5; }
	float			GetFireRate() { return ROF; }

	// Modify this part to control the general accuracy of the gun
	virtual const Vector& GetBulletSpread()
	{
		Vector cone = VECTOR_CONE_2DEGREES;

		return cone;
	}
	DECLARE_ACTTABLE();

private:
	CNetworkVar(int, m_iBurst);
	CNetworkVar(float, m_flAttackEnds);
	CNetworkVar(int, m_iStance);

private:
	CWeaponAlyxGun_P(const CWeaponAlyxGun_P&);
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAlyxGun_P, DT_WeaponAlyxGun_P)

BEGIN_NETWORK_TABLE(CWeaponAlyxGun_P, DT_WeaponAlyxGun_P)
#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_iBurst)),
RecvPropTime(RECVINFO(m_flAttackEnds)),
RecvPropInt(RECVINFO(m_iStance)),
#else
SendPropInt(SENDINFO(m_iBurst)),
SendPropTime(SENDINFO(m_flAttackEnds)),
SendPropInt(SENDINFO(m_iStance)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponAlyxGun_P)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_alyxgun_p, CWeaponAlyxGun_P);
PRECACHE_WEAPON_REGISTER(weapon_alyxgun_p);

acttable_t CWeaponAlyxGun_P::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE,					ACT_HL2MP_IDLE_PISTOL,						false },
	{ ACT_MP_CROUCH_IDLE,					ACT_HL2MP_IDLE_CROUCH_PISTOL,				false },
	{ ACT_MP_RUN,							ACT_HL2MP_RUN_PISTOL,						false },
	{ ACT_MP_CROUCHWALK,					ACT_HL2MP_WALK_CROUCH_PISTOL,				false },
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,		ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,		ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,		false },
	{ ACT_MP_RELOAD_STAND,					ACT_HL2MP_GESTURE_RELOAD_PISTOL,			false },
	{ ACT_MP_RELOAD_CROUCH,					ACT_HL2MP_GESTURE_RELOAD_PISTOL,			false },
	{ ACT_MP_JUMP, 							ACT_HL2MP_JUMP_PISTOL,						false },
};

IMPLEMENT_ACTTABLE(CWeaponAlyxGun_P);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponAlyxGun_P::CWeaponAlyxGun_P()
{
	m_iBurst = BURST;
	m_iStance = 10;
	m_fMinRange1 = 1;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 1;
	m_fMaxRange2 = 200;
	m_bFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: Required for caching the entity during loading
//-----------------------------------------------------------------------------
void CWeaponAlyxGun_P::Precache()
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: The gun is empty, plays a clicking noise with a dryfire anim
//-----------------------------------------------------------------------------
void CWeaponAlyxGun_P::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: This happens if you click and hold the primary fire button
//-----------------------------------------------------------------------------
void CWeaponAlyxGun_P::PrimaryAttack()
{
	// Do we have any bullets left from the current burst cycle?
	if (m_iBurst != 0)
	{
		CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;

		WeaponSound(SINGLE);
		pPlayer->DoMuzzleFlash();

		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		pPlayer->SetAnimation(PLAYER_ATTACK1);
		//ToHL2MPPlayer(pPlayer)->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

		// Each time the player fires the gun, reset the view punch
		CBasePlayer* pOwner = ToBasePlayer(GetOwner());
		if (pOwner)
			pOwner->ViewPunchReset();

		BaseClass::PrimaryAttack();

		// We fired one shot, decrease the number of bullets available for this burst cycle
		m_iBurst--;
		m_flNextPrimaryAttack = gpGlobals->curtime + ROF;
		m_flAttackEnds = gpGlobals->curtime + SequenceDuration();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAlyxGun_P::ItemPreFrame()
{
	GetStance();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as the button is pressed
//-----------------------------------------------------------------------------
void CWeaponAlyxGun_P::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if (m_bInReload)
		return;

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	if (pOwner->m_nButtons & IN_ATTACK)
	{
		if (m_flAttackEnds < gpGlobals->curtime)
			SendWeaponAnim(ACT_VM_IDLE);
	}
	else
	{
		// The firing cycle ended. Reset the burst counter to the max value
		m_iBurst = BURST;
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime) && (m_iClip1 <= 0))
			DryFire();
	}

	// Check the character's current stance for the accuracy calculation
	GetStance();
}

//-----------------------------------------------------------------------------
// Purpose: If we have bullets left then play the attack anim otherwise idle
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponAlyxGun_P::GetPrimaryAttackActivity()
{
	if (m_iBurst != 0)
		return ACT_VM_PRIMARYATTACK;
	else
		return ACT_VM_IDLE;
}

//-----------------------------------------------------------------------------
// Purpose: The gun is being reloaded 
//-----------------------------------------------------------------------------
bool CWeaponAlyxGun_P::Reload()
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
		//ToHL2MPPlayer(GetOwner())->DoAnimationEvent(PLAYERANIMEVENT_RELOAD);

		// Reset the burst counter to the default
		m_iBurst = BURST;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: Calculate the view kick
//-----------------------------------------------------------------------------
void CWeaponAlyxGun_P::AddViewKick()
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	int iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
	RandomSeed(iSeed);

	QAngle viewPunch;

	viewPunch.x = random->RandomFloat(0.25f, 0.5f);
	viewPunch.y = random->RandomFloat(-.6f, .6f);
	viewPunch.z = 0.0f;

	// Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}

//-----------------------------------------------------------------------------
// Purpose: Get the current stance/status of the player
//----------------------------------------------------------------------------- 
void CWeaponAlyxGun_P::GetStance()
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	m_iStance = E_STAND;

	// Movement based stance
	if (pPlayer->m_nButtons & IN_DUCK)
		m_iStance = E_DUCK;
	if (pPlayer->m_nButtons & IN_FORWARD)
		m_iStance = E_MOVE;
	if (pPlayer->m_nButtons & IN_BACK)
		m_iStance = E_MOVE;
	if (pPlayer->m_nButtons & IN_MOVERIGHT)
		m_iStance = E_MOVE;
	if (pPlayer->m_nButtons & IN_MOVELEFT)
		m_iStance = E_MOVE;
	if (pPlayer->m_nButtons & IN_RUN)
		m_iStance = E_RUN;
	if (pPlayer->m_nButtons & IN_SPEED)
		m_iStance = E_RUN;
	if (pPlayer->m_nButtons & IN_JUMP)
		m_iStance = E_JUMP;

	// Health based status
	if (pPlayer->GetHealth() < 25)
		m_iStance = E_INJURED;
	if (pPlayer->GetHealth() < 10)
		m_iStance = E_DYING;
}