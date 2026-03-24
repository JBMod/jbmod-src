//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_ATTACK_H
#define JBMOD_BOT_ATTACK_H

#include "Path/NextBotChasePath.h"


//-------------------------------------------------------------------------------
class CJBModBotAttack : public Action< CJBModBot >
{
public:
	CJBModBotAttack( void );
	virtual ~CJBModBotAttack() { }

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot *me );
	virtual EventDesiredResult< CJBModBot > OnMoveToSuccess( CJBModBot *me, const Path *path );
	virtual EventDesiredResult< CJBModBot > OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason );

	virtual QueryResultType	ShouldRetreat( const INextBot *me ) const;							// is it time to retreat?
	virtual QueryResultType ShouldHurry( const INextBot *me ) const;					// are we in a hurry?

	virtual const char *GetName( void ) const	{ return "Attack"; };

private:
	PathFollower m_path;
	ChasePath m_chasePath;
	CountdownTimer m_repathTimer;
};


#endif // JBMOD_BOT_ATTACK_H
