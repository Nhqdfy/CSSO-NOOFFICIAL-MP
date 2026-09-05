//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Egg launcher - full-auto gun that fires eggs.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "weapon_csbasegun.h"
#include "gamerules.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "weapon_egg.h"

#ifdef CLIENT_DLL
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
	#include "egg_projectile.h"
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( EggGrenade, DT_EggGrenade )

BEGIN_NETWORK_TABLE( CEggGrenade, DT_EggGrenade )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CEggGrenade )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_egg, CEggGrenade );
PRECACHE_REGISTER( weapon_egg );

// How fast eggs are launched out of the barrel.
#define EGG_LAUNCH_SPEED 1500.0f

//-----------------------------------------------------------------------------
// Purpose: Hold to fire. Every shot consumes one egg from the clip and spawns
// an egg projectile that breaks on impact (knocking enemies flying).
//-----------------------------------------------------------------------------
void CEggGrenade::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( m_iClip1 <= 0 )
	{
		if ( m_bFireOnEmpty )
		{
			PlayEmptySound();
			m_iNumEmptyAttacks++;
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
			m_bFireOnEmpty = false;
		}
		return;
	}

	float flCycleTime = GetCSWpnData().m_flCycleTime[ Primary_Mode ];

	// anims + ammo
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	++pPlayer->m_iShotsFired;
	--m_iClip1;

	// Sound & muzzle flash are handled client side during prediction.
#ifdef CLIENT_DLL
	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();
#endif

#ifndef CLIENT_DLL
	// Server spawns the flying egg.
	{
		QAngle angAim = pPlayer->GetFinalAimAngle();
		Vector vForward;
		AngleVectors( angAim, &vForward );

		Vector vecSrc = pPlayer->Weapon_ShootPosition();

		// Push the spawn point out of walls / the player.
		trace_t tr;
		Vector mins( -2, -2, -2 );
		Vector maxs( 2, 2, 2 );
		UTIL_TraceHull( vecSrc, vecSrc + vForward * 32.0f, mins, maxs, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		vecSrc = tr.endpos;

		Vector vecVel = vForward * EGG_LAUNCH_SPEED + pPlayer->GetAbsVelocity() * 0.5f;
		AngularImpulse angSpin( 300, random->RandomInt( -900, 900 ), 0 );

		CEggProjectile::Create( vecSrc, angAim, vecVel, angSpin, pPlayer );
	}
#endif

	// schedule the next shot so holding the button auto-fires
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + flCycleTime;
	SetWeaponIdleTime( gpGlobals->curtime + GetCSWpnData().m_flTimeToIdleAfterFire );
}
