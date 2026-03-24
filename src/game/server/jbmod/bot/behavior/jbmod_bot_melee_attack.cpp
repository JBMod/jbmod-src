//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "bot/jbmod_bot.h"
#include "bot/behavior/jbmod_bot_melee_attack.h"
#include "jbmod/weapon_jbmodbasebasebludgeon.h"

#include "nav_mesh.h"

extern ConVar jbmod_bot_path_lookahead_range;

ConVar jbmod_bot_melee_attack_abandon_range( "jbmod_bot_melee_attack_abandon_range", "500", FCVAR_CHEAT, "If threat is farther away than this, bot will switch back to its primary weapon and attack" );


//---------------------------------------------------------------------------------------------
CJBModBotMeleeAttack::CJBModBotMeleeAttack( float giveUpRange )
{
	m_giveUpRange = giveUpRange < 0.0f ? jbmod_bot_melee_attack_abandon_range.GetFloat() : giveUpRange;
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotMeleeAttack::OnStart( CJBModBot *me, Action< CJBModBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CJBModBot >	CJBModBotMeleeAttack::Update( CJBModBot *me, float interval )
{
	// bash the bad guys
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();

	if ( threat == NULL )
	{
		return Done( "No threat" );
	}

	if ( me->IsDistanceBetweenGreaterThan( threat->GetLastKnownPosition(), m_giveUpRange ) )
	{
		// threat is too far away for melee
		return Done( "Threat is too far away for a melee attack" );
	}

	// switch to our melee weapon
	CBaseJBModBludgeonWeapon *meleeWeapon = me->GetBludgeonWeapon();

	if ( !meleeWeapon )
	{
		// misyl: TF nextbot is missing this check... Interesting.
		return Done( "Don't have a melee weapon!" );
	}

	me->Weapon_Switch( meleeWeapon );

	// actual head aiming is handled elsewhere

	// just keep swinging
	me->PressFireButton();

	// chase them down
	CJBModBotPathCost cost( me, FASTEST_ROUTE );
	m_path.Update( me, threat->GetEntity(), cost );

	return Continue();
}
