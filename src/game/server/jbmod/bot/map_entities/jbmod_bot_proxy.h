//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_PROXY_H
#define JBMOD_BOT_PROXY_H


class CJBModBot;
class CJBModBotActionPoint;


class CJBModBotProxy : public CPointEntity
{
	DECLARE_CLASS( CJBModBotProxy, CPointEntity );
public:
	DECLARE_DATADESC();

	CJBModBotProxy( void );
	virtual ~CJBModBotProxy() { }

	void Think( void );

	// Input
	void InputSetTeam( inputdata_t &inputdata );
	void InputSetMovementGoal( inputdata_t &inputdata );
	void InputSpawn( inputdata_t &inputdata );
	void InputDelete( inputdata_t &inputdata );

	void OnInjured( void );
	void OnKilled( void );
	void OnAttackingEnemy( void );
	void OnKilledEnemy( void );

protected:
	// Output
	COutputEvent m_onSpawned;
	COutputEvent m_onInjured;
	COutputEvent m_onKilled;
	COutputEvent m_onAttackingEnemy;
	COutputEvent m_onKilledEnemy;

	char m_botName[64];
	char m_teamName[64];

	string_t m_spawnOnStart;
	string_t m_actionPointName;
	float m_respawnInterval;

	CHandle< CJBModBot > m_bot;
	CHandle< CJBModBotActionPoint > m_moveGoal;
};


#endif // JBMOD_BOT_PROXY_H
