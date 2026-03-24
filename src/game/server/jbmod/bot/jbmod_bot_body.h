//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_BODY_H
#define JBMOD_BOT_BODY_H

#include "NextBot/Player/NextBotPlayerBody.h"

//----------------------------------------------------------------------------
class CJBModBotBody : public PlayerBody
{
public:
	CJBModBotBody( INextBot* bot ) : PlayerBody( bot )
	{
	}

	virtual ~CJBModBotBody() { }

	virtual float GetHeadAimTrackingInterval( void ) const;			// return how often we should sample our target's position and velocity to update our aim tracking, to allow realistic slop in tracking
};

#endif // JBMOD_BOT_BODY_H
