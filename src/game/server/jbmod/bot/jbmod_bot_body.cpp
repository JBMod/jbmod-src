//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"

#include "jbmod_bot.h"
#include "jbmod_bot_body.h"


// 
// Return how often we should sample our target's position and 
// velocity to update our aim tracking, to allow realistic slop in tracking
//
float CJBModBotBody::GetHeadAimTrackingInterval( void ) const
{
	CJBModBot *me = (CJBModBot *)GetBot();

	switch( me->GetDifficulty() )
	{
	case CJBModBot::EXPERT:
		return 0.05f;

	case CJBModBot::HARD:
		return 0.1f;

	case CJBModBot::NORMAL:
		return 0.25f;

	case CJBModBot::EASY:
		return 1.0f;
	}

	return 0.0f;
}
