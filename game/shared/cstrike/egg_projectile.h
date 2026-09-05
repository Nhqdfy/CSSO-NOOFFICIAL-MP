//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The flying egg projectile launched by the egg launcher. Breaks on
// the first thing it hits; direct hits on enemies knock them flying.
//
//=============================================================================//

#ifndef EGG_PROJECTILE_H
#define EGG_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif

#include "basecsgrenade_projectile.h"

#ifdef CLIENT_DLL

class C_EggProjectile : public C_BaseCSGrenadeProjectile
{
public:
	DECLARE_CLASS( C_EggProjectile, C_BaseCSGrenadeProjectile );
	DECLARE_NETWORKCLASS();
};

#else // GAME_DLL

class CEggProjectile : public CBaseCSGrenadeProjectile
{
public:
	DECLARE_CLASS( CEggProjectile, CBaseCSGrenadeProjectile );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

// Overrides.
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void Detonate( void );

	// Break on any impact (knocking hit enemies flying).
	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	void EggThink( void );

	static CEggProjectile* Create(
		const Vector &position,
		const QAngle &angles,
		const Vector &velocity,
		const AngularImpulse &angVelocity,
		CBaseCombatCharacter *pOwner );

private:
	float m_flEggDieTime;
};

#endif // GAME_DLL

#endif // EGG_PROJECTILE_H
