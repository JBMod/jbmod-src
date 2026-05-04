//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "hud.h"
#include "in_buttons.h"
#include "beamdraw.h"
#include "c_weapon__stubs.h"
#include "clienteffectprecachesystem.h"
#include "c_jbmod_player.h"
#include "c_baseplayer.h"
#include "usercmd.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar physgun_beam_color( "physgun_beam_color", "255 255 255", FCVAR_ARCHIVE );
ConVar physgun_model_color( "physgun_model_color", "255 0 255", FCVAR_ARCHIVE );


CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectGravityGun )
CLIENTEFFECT_MATERIAL( "sprites/physbeam" )
CLIENTEFFECT_REGISTER_END()

class C_BeamQuadratic : public CDefaultClientRenderable
{
public:
	C_BeamQuadratic();
	void			Update( C_BaseEntity *pOwner );

	// IClientRenderable
	virtual const Vector &GetRenderOrigin( void ) { return m_worldPosition; }
	virtual const QAngle &GetRenderAngles( void ) { return vec3_angle; }
	virtual const					matrix3x4_t &RenderableToWorldTransform();
	virtual bool					ShouldDraw( void ) { return true; }
	virtual bool					IsTransparent( void ) { return true; }
	virtual bool					ShouldReceiveProjectedTextures( int flags ) { return false; }
	virtual int						DrawModel( int flags );

	// Returns the bounds relative to the origin (render bounds)
	virtual void	GetRenderBounds( Vector &mins, Vector &maxs )
	{
		// bogus.  But it should draw if you can see the end point
		mins.Init( -99999, -99999, -99999 );
		maxs.Init( 99999, 99999, 99999 );
	}

	C_BaseEntity *m_pOwner;
	Vector					m_targetPosition;
	Vector					m_worldPosition;
	int						m_active;
	int						m_glueTouching;
};


class C_WeaponGravityGun : public C_BaseCombatWeapon
{
	DECLARE_CLASS( C_WeaponGravityGun, C_BaseCombatWeapon );
public:
	C_WeaponGravityGun() {}

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	int KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
	{
		if ( gHUD.m_iKeyBits & IN_ATTACK )
		{
			switch ( keynum )
			{
			case MOUSE_WHEEL_UP:
				gHUD.m_iKeyBits |= IN_WEAPON1;
				return 0;

			case MOUSE_WHEEL_DOWN:
				gHUD.m_iKeyBits |= IN_WEAPON2;
				return 0;
			}
		}

		// Allow engine to process
		return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
	}

	void CreateMove( float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles )
	{
		BaseClass::CreateMove( flInputSampleTime, pCmd, vecOldViewAngles );

		if ( !pCmd )
			return;

		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( !pPlayer )
			return;

		if ( pPlayer->GetActiveWeapon() != this )
			return;

		if ( !m_beam.m_active )
			return;

		if ( !( pCmd->buttons & IN_USE ) )
			return;

		const short nMouseX = pCmd->mousedx;
		const short nMouseY = pCmd->mousedy;
		if ( nMouseX == 0 && nMouseY == 0 )
			return;

		// Use the raw mouse deltas for server-side physgun rotation, but keep the local
		// camera from turning while we're in rotate mode.
		pCmd->mousedx = nMouseX;
		pCmd->mousedy = nMouseY;
		pCmd->viewangles = vecOldViewAngles;
		QAngle localViewAngles = vecOldViewAngles;
		engine->SetViewAngles( localViewAngles );
	}

static void ApplyPhysgunModelColor()
{
    int ir = 255, ig = 0, ib = 255;

    if ( sscanf( physgun_model_color.GetString(), "%d %d %d", &ir, &ig, &ib ) != 3 )
        return;

    ir = clamp( ir, 0, 255 );
    ig = clamp( ig, 0, 255 );
    ib = clamp( ib, 0, 255 );

    float r = ir / 255.0f;
    float g = ig / 255.0f;
    float b = ib / 255.0f;


   
// Basically the only way I could really think of, of doing this:

    IMaterial *pMat = materials->FindMaterial(
        "models/weapons/v_physcannon/v_superphyscannon_sheet",
        TEXTURE_GROUP_MODEL
    );

    if ( !pMat || pMat->IsErrorMaterial() )
    	
        return;

    bool found = false;
    IMaterialVar *pTint = pMat->FindVar( "$selfillumtint", &found );

    if ( found && pTint )
    {
    	
        pTint->SetVecValue( r, g, b );
    }
}





	void OnDataChanged( DataUpdateType_t updateType )
	{

		
		BaseClass::OnDataChanged( updateType );
		ApplyPhysgunModelColor();
		m_beam.Update( this );
	}

	void UpdateOnRemove( void )
	{
		m_beam.m_active = false;
		m_beam.Update( this );

		BaseClass::UpdateOnRemove();
	}

private:
	C_WeaponGravityGun( const C_WeaponGravityGun & );

	C_BeamQuadratic	m_beam;
};

STUB_WEAPON_CLASS_IMPLEMENT( weapon_physgun, C_WeaponGravityGun );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponGravityGun, DT_WeaponGravityGun, CWeaponGravityGun )
RecvPropVector( RECVINFO_NAME( m_beam.m_targetPosition, m_targetPosition ) ),
RecvPropVector( RECVINFO_NAME( m_beam.m_worldPosition, m_worldPosition ) ),
RecvPropInt( RECVINFO_NAME( m_beam.m_active, m_active ) ),
RecvPropInt( RECVINFO_NAME( m_beam.m_glueTouching, m_glueTouching ) ),
END_RECV_TABLE()

C_BeamQuadratic::C_BeamQuadratic()
{
	m_pOwner = NULL;
}

void C_BeamQuadratic::Update( C_BaseEntity *pOwner )
{
	m_pOwner = pOwner;
	if ( m_active )
	{
		if ( m_hRenderHandle == INVALID_CLIENT_RENDER_HANDLE )
		{
			ClientLeafSystem()->AddRenderable( this, RENDER_GROUP_TRANSLUCENT_ENTITY );
		}
		else
		{
			ClientLeafSystem()->RenderableChanged( m_hRenderHandle );
		}
	}
	else if ( !m_active && m_hRenderHandle != INVALID_CLIENT_RENDER_HANDLE )
	{
		ClientLeafSystem()->RemoveRenderable( m_hRenderHandle );
	}
}





int	C_BeamQuadratic::DrawModel( int )
{
	Vector points[3];
	QAngle tmpAngle;

	if ( !m_active )
		return 0;

	C_JBMod_Player *pOwner = ToJBModPlayer( m_pOwner->GetOwnerEntity() );
	C_BaseEntity *pEnt = pOwner->GetRenderedWeaponModel();
	if ( !pEnt )
		return 0;
	pEnt->GetAttachment( 1, points[0], tmpAngle );





	points[1] = 0.5 * ( m_targetPosition + points[0] );

	// a little noise 11t & 13t should be somewhat non-periodic looking
	//points[1].z += 4*sin( gpGlobals->curtime*11 ) + 5*cos( gpGlobals->curtime*13 );
	points[2] = m_worldPosition;

	IMaterial *pMat = materials->FindMaterial( "sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS );
	Vector color;
	
	if ( m_glueTouching )
	{
		color.Init( 1, 0, 0 );
	}
	else
	{
	int ir = 255, ig = 255, ib = 255;


	if ( sscanf( physgun_beam_color.GetString(), "%d %d %d", &ir, &ig, &ib ) != 3 )
	{
    ir = ig = ib = 255; 
	}

	
	ir = clamp( ir, 0, 255 );
	ig = clamp( ig, 0, 255 );
	ib = clamp( ib, 0, 255 );

	
	float r = ir / 255.0f;
	float g = ig / 255.0f;
	float b = ib / 255.0f;

	color.Init( r, g, b );
	}

	float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMat );
	DrawBeamQuadratic( points[0], points[1], points[2], 13, color, scrollOffset );
	return 1;
}

const matrix3x4_t &C_BeamQuadratic::RenderableToWorldTransform()
{
	static matrix3x4_t mat;
	AngleMatrix( GetRenderAngles(), GetRenderOrigin(), mat );
	return mat;
}

