//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_SQUAD_H
#define JBMOD_BOT_SQUAD_H

#include "NextBot/NextBotEventResponderInterface.h"

class CJBModBot;

class CJBModBotSquad : public INextBotEventResponder
{
public:
	CJBModBotSquad( void );
	virtual ~CJBModBotSquad() { }		

	// EventResponder ------
	virtual INextBotEventResponder *FirstContainedResponder( void ) const;
	virtual INextBotEventResponder *NextContainedResponder( INextBotEventResponder *current ) const;
	//----------------------

	bool IsMember( CJBModBot *bot ) const;		// is the given bot in this squad?
	bool IsLeader( CJBModBot *bot ) const;		// is the given bot the leader of this squad?

// 	CJBModBot *GetMember( int i );
 	int GetMemberCount( void ) const;

	CJBModBot *GetLeader( void ) const;

	class Iterator
	{
	public:
		Iterator( void )
		{
			m_bot = NULL;
			m_index = -1;
		}

		Iterator( CJBModBot *bot, int index )
		{
			m_bot = bot;
			m_index = index;
		}

		CJBModBot *operator() ( void )
		{
			return m_bot;
		}

		bool operator==( const Iterator &it ) const	{ return m_bot == it.m_bot && m_index == it.m_index; }
		bool operator!=( const Iterator &it ) const	{ return m_bot != it.m_bot || m_index != it.m_index; }

		CJBModBot *m_bot;
		int m_index;
	};

	Iterator GetFirstMember( void ) const;
	Iterator GetNextMember( const Iterator &it ) const;
	Iterator InvalidIterator() const;

	void CollectMembers( CUtlVector< CJBModBot * > *memberVector ) const;

	#define EXCLUDE_LEADER false
	float GetSlowestMemberSpeed( bool includeLeader = true ) const;
	float GetSlowestMemberIdealSpeed( bool includeLeader = true ) const;
	float GetMaxSquadFormationError( void ) const;

	bool ShouldSquadLeaderWaitForFormation( void ) const;		// return true if the squad leader needs to wait for members to catch up, ignoring those who have broken ranks
	bool IsInFormation( void ) const;						// return true if the squad is in formation (everyone is in or nearly in their desired positions)

	float GetFormationSize( void ) const;
	void SetFormationSize( float size );

	void DisbandAndDeleteSquad( void );

	void SetShouldPreserveSquad( bool bShouldPreserveSquad ) { m_bShouldPreserveSquad = bShouldPreserveSquad; }
	bool ShouldPreserveSquad() const { return m_bShouldPreserveSquad; }

private:
	friend class CJBModBot;

	void Join( CJBModBot *bot );
	void Leave( CJBModBot *bot );

	CUtlVector< CHandle< CJBModBot > > m_roster;
	CHandle< CJBModBot > m_leader;

	float m_formationSize;
	bool m_bShouldPreserveSquad;
};

inline bool CJBModBotSquad::IsMember( CJBModBot *bot ) const
{
	return m_roster.HasElement( bot );
}

inline bool CJBModBotSquad::IsLeader( CJBModBot *bot ) const
{
	return m_leader == bot;
}

inline CJBModBotSquad::Iterator CJBModBotSquad::InvalidIterator() const
{
	return Iterator( NULL, -1 );
}

inline float CJBModBotSquad::GetFormationSize( void ) const
{
	return m_formationSize;
}

inline void CJBModBotSquad::SetFormationSize( float size )
{
	m_formationSize = size;
}


#endif // JBMOD_BOT_SQUAD_H

