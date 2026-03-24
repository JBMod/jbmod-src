//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_GET_PROP_H
#define JBMOD_BOT_GET_PROP_H

#include "Path/NextBotChasePath.h"


class CJBModBotGetProp : public Action< CJBModBot >
{
public:
	CJBModBotGetProp( void );
	~CJBModBotGetProp( void );

	static bool IsPossible( CJBModBot* me );	// Return true if this Action has what it needs to perform right now

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot* me, Action< CJBModBot >* priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot* me, float interval );
	virtual ActionResult< CJBModBot >	OnSuspend( CJBModBot* me, Action< CJBModBot >* interruptingAction );
	virtual ActionResult< CJBModBot >	OnResume( CJBModBot* me, Action< CJBModBot >* interruptingAction );
	virtual void						OnEnd( CJBModBot* me, Action< CJBModBot >* nextAction );

	virtual EventDesiredResult< CJBModBot > OnContact( CJBModBot *me, CBaseEntity *other, CGameTrace *result = NULL );

	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot* me );
	virtual EventDesiredResult< CJBModBot > OnMoveToSuccess( CJBModBot* me, const Path* path );
	virtual EventDesiredResult< CJBModBot > OnMoveToFailure( CJBModBot* me, const Path* path, MoveToFailureType reason );

	virtual QueryResultType ShouldHurry( const INextBot* me ) const;					// are we in a hurry?

	virtual const char* GetName( void ) const { return "GetProp"; };

private:
	PathFollower m_path;
	CHandle< CBaseEntity > m_prop;

	bool m_pushedPhyscannon = false;
};


#endif // JBMOD_BOT_GET_PROP_H
