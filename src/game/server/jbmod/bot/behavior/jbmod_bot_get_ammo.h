//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_GET_AMMO_H
#define JBMOD_BOT_GET_AMMO_H

class CJBModBotGetAmmo : public Action< CJBModBot >
{
public:
	CJBModBotGetAmmo( void );

	static bool IsPossible( CJBModBot *me );			// return true if this Action has what it needs to perform right now

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual EventDesiredResult< CJBModBot > OnContact( CJBModBot *me, CBaseEntity *other, CGameTrace *result = NULL );

	virtual EventDesiredResult< CJBModBot > OnStuck( CJBModBot *me );
	virtual EventDesiredResult< CJBModBot > OnMoveToSuccess( CJBModBot *me, const Path *path );
	virtual EventDesiredResult< CJBModBot > OnMoveToFailure( CJBModBot *me, const Path *path, MoveToFailureType reason );

	virtual QueryResultType ShouldHurry( const INextBot *me ) const;					// are we in a hurry?

	virtual const char *GetName( void ) const	{ return "GetAmmo"; };

private:
	PathFollower m_path;
	CHandle< CBaseEntity > m_ammo;
};


#endif // JBMOD_BOT_GET_AMMO_H
