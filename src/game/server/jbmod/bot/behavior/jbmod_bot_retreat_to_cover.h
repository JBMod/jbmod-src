//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_RETREAT_TO_COVER_H
#define JBMOD_BOT_RETREAT_TO_COVER_H

class CJBModBotRetreatToCover : public Action< CJBModBot >
{
public:
	CJBModBotRetreatToCover( float hideDuration = -1.0f );
	CJBModBotRetreatToCover( Action< CJBModBot > *actionToChangeToOnceCoverReached );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot *me );
	virtual EventDesiredResult< CJBModBot > OnMoveToSuccess( CJBModBot *me, const Path *path );
	virtual EventDesiredResult< CJBModBot > OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason );

	virtual QueryResultType ShouldHurry( const INextBot *me ) const;					// are we in a hurry?

	virtual const char *GetName( void ) const	{ return "RetreatToCover"; };

private:
	float m_hideDuration;
	Action< CJBModBot > *m_actionToChangeToOnceCoverReached;

	PathFollower m_path;
	CountdownTimer m_repathTimer;

	CNavArea *m_coverArea;
	CountdownTimer m_waitInCoverTimer;

	CNavArea *FindCoverArea( CJBModBot *me );
};



#endif // JBMOD_BOT_RETREAT_TO_COVER_H
