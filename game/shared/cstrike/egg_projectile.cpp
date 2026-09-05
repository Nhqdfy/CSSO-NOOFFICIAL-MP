//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The flying egg projectile launched by the egg launcher. It breaks on
// the first thing it hits; direct hits on enemies knock them flying.
//
//=============================================================================//

#include "cbase.h"
#include "egg_projectile.h"
#include "engine/IEngineSound.h"
#include "weapon_csbase.h"

#if defined( CLIENT_DLL )

	#include "c_cs_player.h"

#else

	#include "sendproxy.h"
	#include "cs_player.h"

#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#if defined( CLIENT_DLL )

IMPLEMENT_CLIENTCLASS_DT( C_EggProjectile, DT_EggProjectile, CEggProjectile )
END_RECV_TABLE()

#else // GAME_DLL

// Reuse an existing round, shell-like model. Swap this path if you have a real egg model.
#define EGG_MODEL "models/Weapons/w_eq_fraggrenade_dropped.mdl"

LINK_ENTITY_TO_CLASS( egg_projectile, CEggProjectile );
PRECACHE_REGISTER( egg_projectile );

IMPLEMENT_SERVERCLASS_ST( CEggProjectile, DT_EggProjectile )
END_SEND_TABLE()

BEGIN_DATADESC( CEggProjectile )
	DEFINE_THINKFUNC( EggThink ),
	DEFINE_FIELD( m_flEggDieTime, FIELD_TIME ),
END_DATADESC()

// How hard a direct egg hit knocks the victim flying (velocity multiplier).
ConVar sv_egg_knockback( "sv_egg_knockback", "0.5", FCVAR_REPLICATED, "How hard a direct egg hit knocks the victim flying (velocity multiplier)." );

// --------------------------------------------------------------------------------------------------- //
// CEggProjectile implementation.
// --------------------------------------------------------------------------------------------------- //

CEggProjectile* CEggProjectile::Create(
	const Vector &position,
	const QAngle &angles,
	const Vector &velocity,
	const AngularImpulse &angVelocity,
	CBaseCombatCharacter *pOwner )
{
	CEggProjectile *pEgg = ( CEggProjectile* )CBaseEntity::Create( "egg_projectile", position, angles, pOwner );

	pEgg->SetAbsVelocity( velocity );
	pEgg->SetupInitialTransmittedGrenadeVelocity( velocity );
	pEgg->SetThrower( pOwner );
	pEgg->m_flDamage = 1.0f;
	pEgg->ChangeTeam( pOwner->GetTeamNumber() );

	pEgg->SetGravity( BaseClass::GetGrenadeGravity() );
	pEgg->SetFriction( BaseClass::GetGrenadeFriction() );
	pEgg->SetElasticity( BaseClass::GetGrenadeElasticity() );

	pEgg->ApplyLocalAngularVelocityImpulse( angVelocity );

	// Failsafe: if it never hits anything, quietly despawn after a while.
	pEgg->m_flEggDieTime = gpGlobals->curtime + 8.0f;
	pEgg->SetThink( &CEggProjectile::EggThink );
	pEgg->SetNextThink( gpGlobals->curtime + 0.1f );

	pEgg->m_pWeaponInfo = GetWeaponInfo( WEAPON_EGG );

	return pEgg;
}

void CEggProjectile::Spawn()
{
	SetModel( EGG_MODEL );
	BaseClass::Spawn();

	SetThrownBodygroup();
}

void CEggProjectile::Precache()
{
	PrecacheModel( EGG_MODEL );

	PrecacheScriptSound( "Flashbang.Explode" );

	BaseClass::Precache();
}

void CEggProjectile::EggThink()
{
	if ( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	if ( gpGlobals->curtime > m_flEggDieTime )
	{
		// Quietly despawn if it never hit anything.
		SetThink( &CBaseEntity::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 0.05f );
		SetTouch( NULL );
		AddSolidFlags( FSOLID_NOT_SOLID );
		AddEffects( EF_NODRAW );
		SetAbsVelocity( vec3_origin );
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//--------------------------------------------------------------------------------------------------------
// The egg breaks on whatever it touches. If that's an enemy player, they get
// launched first.
//--------------------------------------------------------------------------------------------------------
void CEggProjectile::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
{
	CBaseEntity *pEntity = trace.m_pEnt;

	// Don't break on our own shooter - let the egg fly out of the barrel first.
	if ( pEntity && pEntity == GetThrower() )
		return;

	// Direct hit on an enemy player: knock them flying!
	if ( pEntity && pEntity->IsPlayer() )
	{
		CCSPlayer *pVictim = ToCSPlayer( pEntity );
		if ( pVictim && pVictim->IsAlive() && GetTeamNumber() != pVictim->GetTeamNumber() )
		{
			Vector vecDir = GetAbsVelocity();
			vecDir.z = 0.0f;
			VectorNormalize( vecDir );
			vecDir.z = 0.6f;	// pop them upward a bit for comedy value
			VectorNormalize( vecDir );

			float flMagnitude = clamp( 400.0f + GetAbsVelocity().Length() * sv_egg_knockback.GetFloat(), 300.0f, 1200.0f );
			pVictim->VelocityPunch( vecDir * flMagnitude );
		}
	}

	// The egg broke. Break it.
	Detonate();
}

void CEggProjectile::Detonate()
{
	// "splat" - soft impact feedback
	EmitSound( "Flashbang.Explode" );

	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.05f );
	SetTouch( NULL );
	AddSolidFlags( FSOLID_NOT_SOLID );
	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
}

#endif // !CLIENT_DLL
