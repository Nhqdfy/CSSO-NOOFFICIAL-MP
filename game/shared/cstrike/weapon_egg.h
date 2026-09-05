//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Egg launcher. A full-auto gun that fires eggs which knock enemies
// flying on direct hits. Eggs break on any impact.
//
//=============================================================================//

#ifndef WEAPON_EGG_H
#define WEAPON_EGG_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_csbasegun.h"

#ifdef CLIENT_DLL
	#define CEggGrenade C_EggGrenade
#endif

//-----------------------------------------------------------------------------
// Egg launcher - a gun, not a throwable!
//-----------------------------------------------------------------------------
class CEggGrenade : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CEggGrenade, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CEggGrenade() { m_bEggExplosive = false; }

	virtual CSWeaponID GetCSWeaponID( void ) const { return WEAPON_EGG; }

	// Hold down fire to keep launching eggs.
	virtual void PrimaryAttack();

	// Right mouse toggles between normal eggs and explosive eggs.
	virtual void SecondaryAttack();

private:
	CEggGrenade( const CEggGrenade& );

	bool m_bEggExplosive;	// false = normal egg (knockback), true = explosive egg (mini-boom)
};

#endif // WEAPON_EGG_H
