//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef JBMOD_BOT_MELEE_ATTACK_H
#define JBMOD_BOT_MELEE_ATTACK_H

#include "Path/NextBotChasePath.h"

class CJBModBotMeleeAttack : public Action< CJBModBot >
{
public:
	CJBModBotMeleeAttack( float giveUpRange = -1.0f );

	virtual ActionResult< CJBModBot >	OnStart( CJBModBot *me, Action< CJBModBot > *priorAction );
	virtual ActionResult< CJBModBot >	Update( CJBModBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "MeleeAttack"; };

private:
	float m_giveUpRange;			// if non-negative and if threat is farther than this, give up our melee attack
	ChasePath m_path;
};

#endif // JBMOD_BOT_MELEE_ATTACK_H
