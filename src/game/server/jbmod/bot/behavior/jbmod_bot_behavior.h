//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_BEHAVIOR_H
#define JBMOD_BOT_BEHAVIOR_H

#include "Path/NextBotPathFollow.h"

class CJBModBotMainAction : public Action< CJBModBot >
{
public:
	virtual Action< CJBModBot > *InitialContainedAction( CJBModBot *me );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual EventDesiredResult< CJBModBot > OnKilled( CJBModBot *me, const CTakeDamageInfo &info );
	virtual EventDesiredResult< CJBModBot > OnInjured( CJBModBot *me, const CTakeDamageInfo &info );
	virtual EventDesiredResult< CJBModBot > OnContact( CJBModBot *me, CBaseEntity *other, CGameTrace *result = NULL );
	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot *me );

	virtual EventDesiredResult< CJBModBot > OnOtherKilled( CJBModBot *me, CBaseCombatCharacter *victim, const CTakeDamageInfo &info );

	virtual QueryResultType ShouldAttack( const INextBot *me, const CKnownEntity *them ) const;
	virtual QueryResultType	ShouldRetreat( const INextBot *me ) const;							// is it time to retreat?
	virtual QueryResultType	ShouldHurry( const INextBot *me ) const;							// are we in a hurry?

	virtual Vector SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const;		// given a subject, return the world space position we should aim at
	virtual QueryResultType IsPositionAllowed( const INextBot *me, const Vector &pos ) const;

	virtual const CKnownEntity *	SelectMoreDangerousThreat( const INextBot *me, 
															   const CBaseCombatCharacter *subject,
															   const CKnownEntity *threat1, 
															   const CKnownEntity *threat2 ) const;	// return the more dangerous of the two threats to 'subject', or NULL if we have no opinion

	virtual const char *GetName( void ) const	{ return "MainAction"; };

private:
	CountdownTimer m_reloadTimer;
	mutable CountdownTimer m_aimAdjustTimer;
	mutable float m_aimErrorRadius;
	mutable float m_aimErrorAngle;

	float m_yawRate;
	float m_priorYaw;
	IntervalTimer m_steadyTimer;

	bool m_isWaitingForFullReload;

	void FireWeaponAtEnemy( CJBModBot *me );

	CHandle< CBaseEntity > m_lastTouch;
	float m_lastTouchTime;

	bool IsImmediateThreat( const CBaseCombatCharacter *subject, const CKnownEntity *threat ) const;
	const CKnownEntity *SelectCloserThreat( CJBModBot *me, const CKnownEntity *threat1, const CKnownEntity *threat2 ) const;
	const CKnownEntity *GetHealerOfThreat( const CKnownEntity *threat ) const;

	const CKnownEntity *SelectMoreDangerousThreatInternal( const INextBot *me, 
														   const CBaseCombatCharacter *subject,
														   const CKnownEntity *threat1, 
														   const CKnownEntity *threat2 ) const;


	void Dodge( CJBModBot *me );

	IntervalTimer m_undergroundTimer;

	CountdownTimer m_reevaluateClassTimer;
};



#endif // JBMOD_BOT_BEHAVIOR_H
