//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"

#include "bot/jbmod_bot.h"
#include "jbmod_bot_proxy.h"
#include "jbmod_bot_generator.h"


BEGIN_DATADESC( CJBModBotProxy )
	DEFINE_KEYFIELD( m_botName,				FIELD_STRING,	"bot_name" ),
	DEFINE_KEYFIELD( m_teamName,			FIELD_STRING,	"team" ),
	DEFINE_KEYFIELD( m_respawnInterval,		FIELD_FLOAT,	"respawn_interval" ),
	DEFINE_KEYFIELD( m_actionPointName,		FIELD_STRING,	"action_point" ),
	DEFINE_KEYFIELD( m_spawnOnStart,		FIELD_STRING,	"spawn_on_start" ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetTeam", InputSetTeam ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetMovementGoal", InputSetMovementGoal ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Spawn", InputSpawn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Delete", InputDelete ),

	DEFINE_OUTPUT( m_onSpawned, "OnSpawned" ),
	DEFINE_OUTPUT( m_onInjured, "OnInjured" ),
	DEFINE_OUTPUT( m_onKilled, "OnKilled" ),
	DEFINE_OUTPUT( m_onAttackingEnemy, "OnAttackingEnemy" ),
	DEFINE_OUTPUT( m_onKilledEnemy, "OnKilledEnemy" ),

	DEFINE_THINKFUNC( Think ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( bot_proxy, CJBModBotProxy );



//------------------------------------------------------------------------------
CJBModBotProxy::CJBModBotProxy( void )
{
	V_strcpy_safe( m_botName, "JBModBot" );
	V_strcpy_safe( m_teamName, "auto" );
	m_bot = NULL;
	m_moveGoal = NULL;
	SetThink( NULL );
}


//------------------------------------------------------------------------------
void CJBModBotProxy::Think( void )
{

}


//------------------------------------------------------------------------------
void CJBModBotProxy::InputSetTeam( inputdata_t &inputdata )
{
	const char *teamName = inputdata.value.String();
	if ( teamName && teamName[0] )
	{
		V_strcpy_safe( m_teamName, teamName );

		// if m_bot exists, tell it to change team
		if ( m_bot != NULL )
		{
			m_bot->HandleCommand_JoinTeam( Bot_GetTeamByName( m_teamName ) );
		}
	}
}


//------------------------------------------------------------------------------
void CJBModBotProxy::InputSetMovementGoal( inputdata_t &inputdata )
{
	const char *entityName = inputdata.value.String();
	if ( entityName && entityName[0] )
	{
		m_moveGoal = dynamic_cast< CJBModBotActionPoint * >( gEntList.FindEntityByName( NULL, entityName ) );

		// if m_bot exists, tell it to move to the new action point
		if ( m_bot != NULL )
		{
			m_bot->SetActionPoint( (CJBModBotActionPoint *)m_moveGoal.Get() );
		}
	}
}


//------------------------------------------------------------------------------
void CJBModBotProxy::InputSpawn( inputdata_t &inputdata )
{
	m_bot = NextBotCreatePlayerBot< CJBModBot >( m_botName );
	if ( m_bot != NULL )
	{
		m_bot->SetSpawnPoint( this );
		m_bot->SetAttribute( CJBModBot::REMOVE_ON_DEATH );
		m_bot->SetAttribute( CJBModBot::IS_NPC );

		m_bot->SetActionPoint( (CJBModBotActionPoint *)m_moveGoal.Get() );

		m_bot->HandleCommand_JoinTeam( Bot_GetTeamByName( m_teamName ) );

		m_onSpawned.FireOutput( m_bot, m_bot );
	}
}


//------------------------------------------------------------------------------
void CJBModBotProxy::InputDelete( inputdata_t &inputdata )
{
	if ( m_bot != NULL )
	{
		engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", m_bot->GetUserID() ) );
		m_bot = NULL;
	}
}


//------------------------------------------------------------------------------
void CJBModBotProxy::OnInjured( void )
{
	m_onInjured.FireOutput( this, this );
}


//------------------------------------------------------------------------------
void CJBModBotProxy::OnKilled( void )
{
	m_onKilled.FireOutput( this, this );
}


//------------------------------------------------------------------------------
void CJBModBotProxy::OnAttackingEnemy( void )
{
	m_onAttackingEnemy.FireOutput( this, this );
}


//------------------------------------------------------------------------------
void CJBModBotProxy::OnKilledEnemy( void )
{
	m_onKilledEnemy.FireOutput( this, this );
}

