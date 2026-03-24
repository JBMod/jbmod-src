//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_MOVE_TO_VANTAGE_POINT_H
#define JBMOD_BOT_MOVE_TO_VANTAGE_POINT_H

#include "Path/NextBotChasePath.h"

class CJBModBotMoveToVantagePoint : public Action< CJBModBot >
{
public:
	CJBModBotMoveToVantagePoint( float maxTravelDistance = 2000.0f );
	virtual ~CJBModBotMoveToVantagePoint() { }

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot *me );
	virtual EventDesiredResult< CJBModBot > OnMoveToSuccess( CJBModBot *me, const Path *path );
	virtual EventDesiredResult< CJBModBot > OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason );

	virtual const char *GetName( void ) const	{ return "MoveToVantagePoint"; };

private:
	float m_maxTravelDistance;
	PathFollower m_path;
	CountdownTimer m_repathTimer;
	CNavArea *m_vantageArea;
};

#endif // JBMOD_BOT_MOVE_TO_VANTAGE_POINT_H
