//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_NAV_ENT_MOVE_TO_H
#define JBMOD_BOT_NAV_ENT_MOVE_TO_H

#include "Path/NextBotPathFollow.h"
#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"


class CJBModBotNavEntMoveTo : public Action< CJBModBot >
{
public:
	CJBModBotNavEntMoveTo( const CFuncNavPrerequisite *prereq );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "NavEntMoveTo"; };

private:
	CHandle< CFuncNavPrerequisite > m_prereq;
	Vector m_goalPosition;				// specific position within entity to move to
	CNavArea* m_pGoalArea;

	CountdownTimer m_waitTimer;

	PathFollower m_path;				// how we get to the loot
	CountdownTimer m_repathTimer;
};


#endif // JBMOD_BOT_NAV_ENT_MOVE_TO_H
