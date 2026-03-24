//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_APPROACH_OBJECT_H
#define JBMOD_BOT_APPROACH_OBJECT_H

#include "Path/NextBotPathFollow.h"

class CJBModBotApproachObject : public Action< CJBModBot >
{
public:
	CJBModBotApproachObject( CBaseEntity *loot, float range = 10.0f );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "ApproachObject"; };

private:
	CHandle< CBaseEntity > m_loot;		// what we are collecting
	float m_range;						// how close should we get
	PathFollower m_path;				// how we get to the loot
	CountdownTimer m_repathTimer;
};


#endif // JBMOD_BOT_APPROACH_OBJECT_H
