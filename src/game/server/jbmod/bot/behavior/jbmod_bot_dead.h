//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_DEAD_H
#define JBMOD_BOT_DEAD_H

#include "Path/NextBotChasePath.h"

class CJBModBotDead : public Action< CJBModBot >
{
public:
	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "Dead"; };

private:
	IntervalTimer m_deadTimer;
};

#endif // JBMOD_BOT_DEAD_H
