//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "jbmod_bot.h"
#include "jbmod_bot_squad.h"

//----------------------------------------------------------------------
CJBModBotSquad::CJBModBotSquad( void )
{
	m_leader = NULL;
	m_formationSize = -1.0f;
	m_bShouldPreserveSquad = false;
}


//----------------------------------------------------------------------
void CJBModBotSquad::Join( CJBModBot *bot )
{
	// first member is the leader
	if ( m_roster.Count() == 0 )
	{
		m_leader = bot;
	}

	m_roster.AddToTail( bot );
}


//----------------------------------------------------------------------
void CJBModBotSquad::Leave( CJBModBot *bot )
{
	m_roster.FindAndRemove( bot );

	if ( bot == m_leader.Get() )
	{
		m_leader = NULL;

		// pick the next living leader that's left in the squad
		if ( m_bShouldPreserveSquad )
		{
			CUtlVector< CJBModBot* > members;
			CollectMembers( &members );
			if ( members.Count() )
			{
				m_leader = members[0];
			}
		}
	}
	
	if ( GetMemberCount() == 0 )
	{
		DisbandAndDeleteSquad();
	}
}


//----------------------------------------------------------------------
INextBotEventResponder *CJBModBotSquad::FirstContainedResponder( void ) const
{
	return m_roster.Count() ? m_roster[0] : NULL;
}


//----------------------------------------------------------------------
INextBotEventResponder *CJBModBotSquad::NextContainedResponder( INextBotEventResponder *current ) const
{
	CJBModBot *currentBot = (CJBModBot *)current;

	int i = m_roster.Find( currentBot );

	if ( i == m_roster.InvalidIndex() )
		return NULL;

	if ( ++i >= m_roster.Count() )
		return NULL;

	return (CJBModBot *)m_roster[i];
}


//----------------------------------------------------------------------
CJBModBot *CJBModBotSquad::GetLeader( void ) const
{
	return m_leader;
}


//----------------------------------------------------------------------
void CJBModBotSquad::CollectMembers( CUtlVector< CJBModBot * > *memberVector ) const
{
	for( int i=0; i<m_roster.Count(); ++i )
	{
		if ( m_roster[i] != NULL && m_roster[i]->IsAlive() )
		{
			memberVector->AddToTail( m_roster[i] );
		}
	}
}


//----------------------------------------------------------------------
CJBModBotSquad::Iterator CJBModBotSquad::GetFirstMember( void ) const
{
	// find first non-NULL member
	for( int i=0; i<m_roster.Count(); ++i )
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
			return Iterator( m_roster[i], i );

	return InvalidIterator();
}


//----------------------------------------------------------------------
CJBModBotSquad::Iterator CJBModBotSquad::GetNextMember( const Iterator &it ) const
{
	// find next non-NULL member
	for( int i=it.m_index+1; i<m_roster.Count(); ++i )
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
			return Iterator( m_roster[i], i );

	return InvalidIterator();
}


//----------------------------------------------------------------------
int CJBModBotSquad::GetMemberCount( void ) const
{
	// count the non-NULL members
	int count = 0;
	for( int i=0; i<m_roster.Count(); ++i )
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
			++count;

	return count;
}


//----------------------------------------------------------------------
// Return the speed of the slowest member of the squad
float CJBModBotSquad::GetSlowestMemberSpeed( bool includeLeader ) const
{
	float speed = FLT_MAX;

	int i = includeLeader ? 0 : 1;

	for( ; i<m_roster.Count(); ++i )
	{
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
		{
			float memberSpeed = m_roster[i]->MaxSpeed();
			if ( memberSpeed < speed )
			{
				speed = memberSpeed;
			}
		}
	}

	return speed;
}


//----------------------------------------------------------------------
// Return the speed of the slowest member of the squad, 
// considering their ideal class speed.
float CJBModBotSquad::GetSlowestMemberIdealSpeed( bool includeLeader ) const
{
	float speed = FLT_MAX;

	int i = includeLeader ? 0 : 1;

	for( ; i<m_roster.Count(); ++i )
	{
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
		{
			// TODO(misyl): One could make this consider all the members of the roster's
			// aux power, and see if they are allowed to sprint here.
			//
			// I am not planning on using squads, just pointing it out if someone
			// else wants to use this code.
			float memberSpeed = hl2_normspeed.GetFloat();
			if ( memberSpeed < speed )
			{
				speed = memberSpeed;
			}
		}
	}

	return speed;
}


//----------------------------------------------------------------------
// Return the maximum formation error of the squad's members.
float CJBModBotSquad::GetMaxSquadFormationError( void ) const
{
	float maxError = 0.0f;

	// skip the leader since he's what the formation forms around
	for( int i=1; i<m_roster.Count(); ++i )
	{
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
		{
			float error = m_roster[i]->GetSquadFormationError();
			if ( error > maxError )
			{
				maxError = error;
			}
		}
	}

	return maxError;
}


//----------------------------------------------------------------------
// Return true if the squad leader needs to wait for members to catch up, ignoring those who have broken ranks
bool CJBModBotSquad::ShouldSquadLeaderWaitForFormation( void ) const
{
	// skip the leader since he's what the formation forms around
	for( int i=1; i<m_roster.Count(); ++i )
	{
		// the squad leader should wait if any member is out of position, but not yet broken ranks
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
		{
			if ( m_roster[i]->GetSquadFormationError() >= 1.0f && 
				 !m_roster[i]->HasBrokenFormation() && 
				 !m_roster[i]->GetLocomotionInterface()->IsStuck() )
			{
				// wait for me!
				return true;
			}
		}
	}

	return false;
}


//----------------------------------------------------------------------
// Return true if the squad is in formation (everyone is in or nearly in their desired positions)
bool CJBModBotSquad::IsInFormation( void ) const
{
	// skip the leader since he's what the formation forms around
	for( int i=1; i<m_roster.Count(); ++i )
	{
		if ( m_roster[i].Get() != NULL && m_roster[i]->IsAlive() )
		{
			if ( m_roster[i]->HasBrokenFormation() ||
				 m_roster[i]->GetLocomotionInterface()->IsStuck() )
			{
				// I'm not "in formation"
				continue;
			}

			if ( m_roster[i]->GetSquadFormationError() > 0.75f )
			{
				// I'm not in position yet
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------
// Tell all members to leave the squad and then delete itself
void CJBModBotSquad::DisbandAndDeleteSquad( void )
{
	// Tell each member of the squad to remove this reference
	for( int i=0; i < m_roster.Count(); ++i )
	{
		if ( m_roster[i].Get() != NULL )
		{
			m_roster[i]->DeleteSquad();
		}
	}

	delete this;
}