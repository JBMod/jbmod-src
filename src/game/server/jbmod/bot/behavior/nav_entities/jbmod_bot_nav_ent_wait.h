//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_NAV_ENT_WAIT_H
#define JBMOD_BOT_NAV_ENT_WAIT_H

#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"


class CJBModBotNavEntWait : public Action< CJBModBot >
{
public:
	CJBModBotNavEntWait( const CFuncNavPrerequisite *prereq );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "NavEntWait"; };

private:
	CHandle< CFuncNavPrerequisite > m_prereq;
	CountdownTimer m_timer;
};


#endif // JBMOD_BOT_NAV_ENT_WAIT_H
