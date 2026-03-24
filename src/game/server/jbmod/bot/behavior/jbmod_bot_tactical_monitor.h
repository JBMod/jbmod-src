//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_TACTICAL_MONITOR_H
#define JBMOD_BOT_TACTICAL_MONITOR_H

class CObjectTeleporter;

class CJBModBotTacticalMonitor : public Action< CJBModBot >
{
public:
	virtual Action< CJBModBot > *InitialContainedAction( CJBModBot *me );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual EventDesiredResult< CJBModBot > OnNavAreaChanged( CJBModBot *me, CNavArea *newArea, CNavArea *oldArea );
	virtual EventDesiredResult< CJBModBot > OnOtherKilled( CJBModBot *me, CBaseCombatCharacter *victim, const CTakeDamageInfo &info );

	// @note Tom Bui: Currently used for the training stuff, but once we get that interface down, we will turn that
	// into a proper API
	virtual EventDesiredResult< CJBModBot > OnCommandString( CJBModBot *me, const char *command );

	virtual const char *GetName( void ) const	{ return "TacticalMonitor"; }

private:
	CountdownTimer m_maintainTimer;

	CountdownTimer m_acknowledgeAttentionTimer;
	CountdownTimer m_acknowledgeRetryTimer;
	CountdownTimer m_attentionTimer;

	CountdownTimer m_stickyBombCheckTimer;
	void MonitorArmedStickyBombs( CJBModBot *me );

	void AvoidBumpingEnemies( CJBModBot *me );
};



#endif // JBMOD_BOT_TACTICAL_MONITOR_H
