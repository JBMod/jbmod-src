//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_SEEK_AND_DESTROY_H
#define JBMOD_BOT_SEEK_AND_DESTROY_H

#include "Path/NextBotChasePath.h"


//
// Roam around the map attacking enemies
//
class CJBModBotSeekAndDestroy : public Action< CJBModBot >
{
public:
	CJBModBotSeekAndDestroy( float duration = -1.0f );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual ActionResult< CJBModBot >	OnResume( CJBModBot *me, Action< CJBModBot > *interruptingAction );

	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot *me );
	virtual EventDesiredResult< CJBModBot > OnMoveToSuccess( CJBModBot *me, const Path *path );
	virtual EventDesiredResult< CJBModBot > OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason );

	virtual QueryResultType	ShouldRetreat( const INextBot *me ) const;					// is it time to retreat?
	virtual QueryResultType ShouldHurry( const INextBot *me ) const;					// are we in a hurry?

	virtual EventDesiredResult< CJBModBot > OnTerritoryCaptured( CJBModBot *me, int territoryID );
	virtual EventDesiredResult< CJBModBot > OnTerritoryLost( CJBModBot *me, int territoryID );
	virtual EventDesiredResult< CJBModBot > OnTerritoryContested( CJBModBot *me, int territoryID );

	virtual EventDesiredResult< CJBModBot > OnCommandApproach( CJBModBot *me, const Vector& pos, float range );

	virtual const char *GetName( void ) const	{ return "SeekAndDestroy"; };

private:
	PathFollower m_path;
	CountdownTimer m_repathTimer;
	CountdownTimer m_itemStolenTimer;
	EHANDLE m_hTargetEntity;
	bool m_bGoingToTargetEntity = false;
	Vector m_vGoalPos = vec3_origin;
	bool m_bTimerElapsed = false;
	void RecomputeSeekPath( CJBModBot *me );

	bool m_bOverrideApproach = false;
	Vector m_vOverrideApproach = vec3_origin;

	CountdownTimer m_giveUpTimer;
};


#endif // JBMOD_BOT_SEEK_AND_DESTROY_H
