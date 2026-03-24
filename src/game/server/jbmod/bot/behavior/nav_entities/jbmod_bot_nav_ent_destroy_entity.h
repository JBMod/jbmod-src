//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_NAV_ENT_DESTROY_ENTITY_H
#define JBMOD_BOT_NAV_ENT_DESTROY_ENTITY_H

#include "Path/NextBotPathFollow.h"
#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"
#include "jbmod/weapon_slam.h"

class CJBModBotNavEntDestroyEntity : public Action< CJBModBot >
{
public:
	CJBModBotNavEntDestroyEntity( const CFuncNavPrerequisite *prereq );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );
	virtual void					OnEnd( CJBModBot *me, Action< CJBModBot > *nextAction );

	virtual const char *GetName( void ) const	{ return "NavEntDestroyEntity"; };

private:
	CHandle< CFuncNavPrerequisite > m_prereq;
	PathFollower m_path;				// how we get to the target
	CountdownTimer m_repathTimer;
	bool m_wasIgnoringEnemies;

	void DetonateStickiesWhenSet( CJBModBot *me, CWeapon_SLAM *stickyLauncher ) const;
	bool m_isReadyToLaunchSticky;
};


#endif // JBMOD_BOT_NAV_ENT_DESTROY_ENTITY_H
