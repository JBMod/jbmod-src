//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"

#include "jbmod_bot.h"
#include "jbmod_bot_locomotion.h"
#include "particle_parse.h"

extern ConVar falldamage;

//-----------------------------------------------------------------------------------------
void CJBModBotLocomotion::Update( void )
{
	BaseClass::Update();

	CJBModBot* me = ToJBModBot( GetBot()->GetEntity() );
	if ( !me )
	{
		return;
	}

	// always 'crouch jump'
	if ( IsOnGround() )
	{
		me->ReleaseCrouchButton();
	}
	else
	{
		me->PressCrouchButton( 0.3f );
	}
}


//-----------------------------------------------------------------------------------------
// Move directly towards the given position
void CJBModBotLocomotion::Approach( const Vector& pos, float goalWeight )
{
	BaseClass::Approach( pos, goalWeight );
}


//-----------------------------------------------------------------------------------------
// Distance at which we will die if we fall
float CJBModBotLocomotion::GetDeathDropHeight( void ) const
{
	CJBModBot* me = ( CJBModBot* )GetBot()->GetEntity();

	// misyl: Fall damage only deals 10 health otherwise. 
	if ( falldamage.GetInt() != 1 )
	{
		if ( me->GetHealth() > 10.0f )
			return MAX_COORD_FLOAT;

		// #define PLAYER_MAX_SAFE_FALL_SPEED	526.5f // approx 20 feet sqrt( 2 * gravity * 20 * 12 )
		return 240.0f;
	}
	else
	{
		return 1000.0f;
	}
}


//-----------------------------------------------------------------------------------------
// Get maximum running speed
float CJBModBotLocomotion::GetRunSpeed( void ) const
{
	return hl2_normspeed.GetFloat();
	// TODO(misyl): Teach bots to sprint.
	//return hl2_sprintspeed.GetFloat();
}


//-----------------------------------------------------------------------------------------
// Return true if given area can be used for navigation
bool CJBModBotLocomotion::IsAreaTraversable( const CNavArea* area ) const
{
	CJBModBot* me = ( CJBModBot* )GetBot()->GetEntity();

	if ( area->IsBlocked( me->GetTeamNumber() ) )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------
bool CJBModBotLocomotion::IsEntityTraversable( CBaseEntity* obstacle, TraverseWhenType when ) const
{
	// assume all players are "traversable" in that they will move or can be killed
	if ( obstacle && obstacle->IsPlayer() )
	{
		return true;
	}

	// assume held objects will move.
	if ( obstacle && obstacle->VPhysicsGetObject() && ( obstacle->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) )
	{
		return true;
	}

	if ( obstacle )
	{
		// misyl:
		// override the base brush logic here to work better for hl2mp (w/ solid flags)
		// changing this for TF would be scary.
		//
		// if we hit a clip brush, ignore it if it is not BRUSHSOLID_ALWAYS
		if ( FClassnameIs( obstacle, "func_brush" ) )
		{
			CFuncBrush* brush = ( CFuncBrush* )obstacle;

			switch ( brush->m_iSolidity )
			{
			case CFuncBrush::BRUSHSOLID_ALWAYS:
				return false;
			case CFuncBrush::BRUSHSOLID_NEVER:
				return true;
			case CFuncBrush::BRUSHSOLID_TOGGLE:
				return brush->GetSolidFlags() & FSOLID_NOT_SOLID;
			}
		}
	}

	return PlayerLocomotion::IsEntityTraversable( obstacle, when );
}
