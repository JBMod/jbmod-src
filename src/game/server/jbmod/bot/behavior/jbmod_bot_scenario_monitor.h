//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_SCENARIO_MONITOR_H
#define JBMOD_BOT_SCENARIO_MONITOR_H

class CJBModBotScenarioMonitor : public Action< CJBModBot >
{
public:
	virtual Action< CJBModBot > *InitialContainedAction( CJBModBot *me );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "ScenarioMonitor"; }

private:
	CountdownTimer m_ignoreLostFlagTimer;
	CountdownTimer m_lostFlagTimer;

	virtual Action< CJBModBot > *DesiredScenarioAndClassAction( CJBModBot *me );
};


#endif // JBMOD_BOT_SCENARIO_MONITOR_H
