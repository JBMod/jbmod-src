//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"

#include "bot/jbmod_bot.h"
#include "jbmod_bot_generator.h"

#include "bot/jbmod_bot.h"
#include "bot/jbmod_bot_manager.h"
#include "jbmod_gamerules.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"

extern ConVar jbmod_bot_prefix_name_with_difficulty;
extern ConVar jbmod_bot_difficulty;

extern void CreateBotName( int iTeam, CJBModBot::DifficultyType skill, char* pBuffer, int iBufferSize );

//------------------------------------------------------------------------------

BEGIN_DATADESC( CJBModBotGenerator )
	DEFINE_KEYFIELD( m_spawnCount,		FIELD_INTEGER,	"count" ),
	DEFINE_KEYFIELD( m_maxActiveCount,	FIELD_INTEGER,	"maxActive" ),
	DEFINE_KEYFIELD( m_spawnInterval,	FIELD_FLOAT,	"interval" ),
	DEFINE_KEYFIELD( m_teamName,		FIELD_STRING,	"team" ),
	DEFINE_KEYFIELD( m_actionPointName,	FIELD_STRING,	"action_point" ),
	DEFINE_KEYFIELD( m_initialCommand,	FIELD_STRING,	"initial_command" ),
	DEFINE_KEYFIELD( m_bSuppressFire,	FIELD_BOOLEAN,	"suppressFire" ),
	DEFINE_KEYFIELD( m_bDisableDodge,	FIELD_BOOLEAN,	"disableDodge" ),
	DEFINE_KEYFIELD( m_iOnDeathAction,	FIELD_INTEGER,	"actionOnDeath" ),
	DEFINE_KEYFIELD( m_bUseTeamSpawnpoint,	FIELD_BOOLEAN,	"useTeamSpawnPoint" ),
	DEFINE_KEYFIELD( m_difficulty,		FIELD_INTEGER,	"difficulty" ),
	DEFINE_KEYFIELD( m_bRetainBuildings,	FIELD_BOOLEAN,	"retainBuildings" ),
	DEFINE_KEYFIELD( m_bSpawnOnlyWhenTriggered,	FIELD_BOOLEAN, "spawnOnlyWhenTriggered" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetSuppressFire", InputSetSuppressFire ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetDisableDodge", InputSetDisableDodge ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetDifficulty", InputSetDifficulty ),
	DEFINE_INPUTFUNC( FIELD_STRING, "CommandGotoActionPoint", InputCommandGotoActionPoint ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetAttentionFocus", InputSetAttentionFocus ),
	DEFINE_INPUTFUNC( FIELD_STRING, "ClearAttentionFocus", InputClearAttentionFocus ),

	DEFINE_INPUTFUNC( FIELD_VOID, "SpawnBot", InputSpawnBot ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveBots", InputRemoveBots ),

	DEFINE_OUTPUT( m_onSpawned, "OnSpawned" ),
	DEFINE_OUTPUT( m_onExpended, "OnExpended" ),
	DEFINE_OUTPUT( m_onBotKilled, "OnBotKilled" ),

	DEFINE_THINKFUNC( GeneratorThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( bot_generator, CJBModBotGenerator );

enum
{
	kOnDeath_Respawn,
	kOnDeath_RemoveSelf,
	kOnDeath_MoveToSpectatorTeam,
};

//------------------------------------------------------------------------------
CJBModBotGenerator::CJBModBotGenerator( void ) 
	: m_bSuppressFire(false)
	, m_bDisableDodge(false)
	, m_bUseTeamSpawnpoint(false)
	, m_bRetainBuildings(false)
	, m_bExpended(false)
	, m_iOnDeathAction(kOnDeath_RemoveSelf)
	, m_difficulty(CJBModBot::UNDEFINED)
	, m_spawnCountRemaining(0)
	, m_bSpawnOnlyWhenTriggered(false)
	, m_bEnabled(true)
{
	SetThink( NULL );
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;

	if ( m_bExpended )
	{
		return;
	}

	SetThink( &CJBModBotGenerator::GeneratorThink );

	if ( m_spawnCountRemaining )
	{
		// already generating - don't restart count
		return;
	}
	SetNextThink( gpGlobals->curtime );
	m_spawnCountRemaining = m_spawnCount;
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;

	// just stop thinking
	SetThink( NULL );
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputSetSuppressFire( inputdata_t &inputdata )
{
	m_bSuppressFire = inputdata.value.Bool();
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputSetDisableDodge( inputdata_t &inputdata )
{
	m_bDisableDodge = inputdata.value.Bool();
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputSetDifficulty( inputdata_t &inputdata )
{
	m_difficulty = clamp( inputdata.value.Int(), (int) CJBModBot::UNDEFINED, (int) CJBModBot::EXPERT );
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputCommandGotoActionPoint( inputdata_t &inputdata )
{
	CJBModBotActionPoint *pActionPoint = dynamic_cast<CJBModBotActionPoint *>( gEntList.FindEntityByName( NULL, inputdata.value.String() ) );
	if ( pActionPoint == NULL )
	{
		return;
	}
	for ( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CHandle< CJBModBot > hBot = m_spawnedBotVector[i];
		if ( hBot == NULL )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		if ( hBot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		hBot->SetActionPoint( pActionPoint );
		hBot->OnCommandString( "goto action point" );
		++i;
	}
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputSetAttentionFocus( inputdata_t &inputdata )
{
	CBaseEntity *focus = gEntList.FindEntityByName( NULL, inputdata.value.String() );

	if ( focus == NULL )
	{
		return;
	}

	for( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CJBModBot *bot = m_spawnedBotVector[i];

		if ( !bot || bot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}

		bot->SetAttentionFocus( focus );

		++i;
	}
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputClearAttentionFocus( inputdata_t &inputdata )
{
	for( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CJBModBot *bot = m_spawnedBotVector[i];

		if ( !bot || bot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}

		bot->ClearAttentionFocus();

		++i;
	}
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputSpawnBot( inputdata_t &inputdata )
{
	if ( m_bEnabled )
	{
		SpawnBot();
	}
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::InputRemoveBots( inputdata_t &inputdata )
{
	for( int i = 0; i < m_spawnedBotVector.Count(); i++ )
	{
		CJBModBot *pBot = m_spawnedBotVector[i];
		if ( pBot )
		{
			pBot->Remove();
			engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", pBot->GetUserID() ) );
		}

		m_spawnedBotVector.FastRemove(i);
	}
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::OnBotKilled( CJBModBot *pBot )
{
	m_onBotKilled.FireOutput( pBot, this );
}

//------------------------------------------------------------------------------

void CJBModBotGenerator::Activate()
{
	BaseClass::Activate();
	m_moveGoal = gEntList.FindEntityByName( NULL, m_actionPointName.ToCStr() );
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::GeneratorThink( void )
{
	// create the bot finally...
	if ( !m_bSpawnOnlyWhenTriggered )
	{
		SpawnBot();
	}
}

//------------------------------------------------------------------------------
void CJBModBotGenerator::SpawnBot( void )
{
	// did we exceed the max active count?
	for ( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CHandle< CJBModBot > hBot = m_spawnedBotVector[i];
		if ( hBot == NULL )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		if ( hBot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		++i;
	}

	if ( m_spawnedBotVector.Count() >= m_maxActiveCount )
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
		return;
	}

	char name[256];
	CJBModBot *bot = TheJBModBots().GetAvailableBotFromPool();
	if ( bot == NULL )
	{
		CreateBotName( TEAM_UNASSIGNED, (CJBModBot::DifficultyType)m_difficulty, name, sizeof(name) );
		bot = NextBotCreatePlayerBot< CJBModBot >( name );
	}

	if ( bot ) 
	{										   
		m_spawnedBotVector.AddToTail( bot );

		bot->SetSpawner( this );

		if ( m_bUseTeamSpawnpoint == false )
		{
			bot->SetSpawnPoint( this );		
		}

		if ( m_bSuppressFire )
		{
			bot->SetAttribute( CJBModBot::SUPPRESS_FIRE );
		}

		if ( m_bDisableDodge )
		{
			bot->SetAttribute( CJBModBot::DISABLE_DODGE );
		}

		if ( m_difficulty != CJBModBot::UNDEFINED )
		{
			bot->SetDifficulty( (CJBModBot::DifficultyType )m_difficulty );
		}

		// propagate the generator's spawn flags into all bots generated
		bot->ClearBehaviorFlag( JBMODBOT_ALL_BEHAVIOR_FLAGS );
		bot->SetBehaviorFlag( m_spawnflags );

		switch ( m_iOnDeathAction )
		{
		case kOnDeath_RemoveSelf:
			bot->SetAttribute( CJBModBot::REMOVE_ON_DEATH );
			break;
		case kOnDeath_MoveToSpectatorTeam:
			bot->SetAttribute( CJBModBot::BECOME_SPECTATOR_ON_DEATH );
			break;
		} // switch

		bot->SetActionPoint( dynamic_cast<CJBModBotActionPoint *>( m_moveGoal.Get() ) );

		// XXX(misyl): TODO!!!!!
#if 0
		// pick a team and force the team change
		// HandleCommand_JoinTeam() may fail, but this should always succeed
		int iTeam = TEAM_UNASSIGNED;
		if ( FStrEq( m_teamName.ToCStr(), "auto" ) )
		{
			iTeam = bot->GetAutoTeam();
		}
		else if ( FStrEq( m_teamName.ToCStr(), "spectate" ) )
		{
			iTeam = TEAM_SPECTATOR;
		}
		else
		{
			for ( int i = 0; i < TF_TEAM_COUNT; ++i )
			{
				COMPILE_TIME_ASSERT( TF_TEAM_COUNT == ARRAYSIZE( g_aTeamNames ) );
				if ( FStrEq( m_teamName.ToCStr(), g_aTeamNames[i] ) )
				{
					iTeam = i;
					break;
				}
			}
		}
		if ( iTeam == TEAM_UNASSIGNED )
		{
			iTeam = bot->GetAutoTeam();
		}
		bot->ChangeTeam( iTeam, false, false );
		
		const char* pClassName =  m_bBotChoosesClass ? bot->GetNextSpawnClassname() : m_className.ToCStr();
		bot->HandleCommand_JoinClass( pClassName );
#endif

		if ( bot->IsAlive() == false )
		{
			bot->ForceRespawn();
		}

		// make sure the bot is facing the right way.
		// @todo Tom Bui: for some reason it is still turning towards another direction...need to investigate
		bot->SnapEyeAngles( GetAbsAngles() );

		if ( FStrEq( m_initialCommand.ToCStr(), "" ) == false )
		{
			// @note Tom Bui: we call Update() once here to make sure the bot is ready to receive commands
			bot->Update();
			bot->OnCommandString( m_initialCommand.ToCStr() );
		}
		m_onSpawned.FireOutput( bot, this );

		--m_spawnCountRemaining;
		if ( m_spawnCountRemaining )
		{
			SetNextThink( gpGlobals->curtime + m_spawnInterval );
		}
		else
		{
			SetThink( NULL );
			m_onExpended.FireOutput( this, this );
			m_bExpended = true;
		}
	}
}

//------------------------------------------------------------------------------

BEGIN_DATADESC( CJBModBotActionPoint )
	DEFINE_KEYFIELD( m_stayTime,			FIELD_FLOAT,	"stay_time" ),
	DEFINE_KEYFIELD( m_desiredDistance,		FIELD_FLOAT,	"desired_distance" ),
	DEFINE_KEYFIELD( m_nextActionPointName,	FIELD_STRING,	"next_action_point" ),
	DEFINE_KEYFIELD( m_command,				FIELD_STRING,	"command" ),
	DEFINE_OUTPUT( m_onReachedActionPoint, "OnBotReached" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( bot_action_point, CJBModBotActionPoint );

//------------------------------------------------------------------------------

CJBModBotActionPoint::CJBModBotActionPoint()
: m_stayTime( 0.0f )
, m_desiredDistance( 1.0f )

{

}

//------------------------------------------------------------------------------

void CJBModBotActionPoint::Activate()
{
	BaseClass::Activate();
	m_moveGoal = gEntList.FindEntityByName( NULL, m_nextActionPointName.ToCStr() );
}

//------------------------------------------------------------------------------

bool CJBModBotActionPoint::IsWithinRange( CBaseEntity *entity )
{
	return ( entity->GetAbsOrigin() - GetAbsOrigin() ).IsLengthLessThan( m_desiredDistance );
}

//------------------------------------------------------------------------------

void CJBModBotActionPoint::ReachedActionPoint( CJBModBot* pBot )
{
	if ( FStrEq( m_command.ToCStr(), "" ) == false )
	{
		pBot->OnCommandString( m_command.ToCStr() );
	}
	m_onReachedActionPoint.FireOutput( pBot, this );
}

//------------------------------------------------------------------------------
