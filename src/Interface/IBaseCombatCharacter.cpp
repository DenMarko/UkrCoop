#include "IBaseCombatCharacter.h"
#include "IBaseCombatWeapon.h"
#include "CAmmoDef.h"

IBaseCombatWeapon *IBaseCombatCharacter::GetActiveWeapon() const
{
    return m_hActiveWeapon.Get();
}

IBaseCombatWeapon *IBaseCombatCharacter::GetWeapon(int i) const
{
	Assert((i >= 0) && (i < MAX_WEAPONS));
    return m_hMyWeapons[i].Get();
}

int IBaseCombatCharacter::GiveAmmo(int iCount, const char *szName, bool bSuppressSound)
{
    int iAmmoType = Index((_CAmmoDef *)g_CallHelper->GetAmmoDef(), szName);
    if(iAmmoType == -1)
    {
        return 0;
    }
    return GiveAmmo(iCount, iAmmoType, bSuppressSound);
}

void IBaseCombatCharacter::GiveAmmoBox(int iCount, int iAmmoIndex)
{
    if (iCount <= 0)
        return;

    if ( iAmmoIndex < 0 || iAmmoIndex >= MAX_AMMO_SLOTS )
		return;

    EmitSound("BaseCombatCharacter.AmmoPickup");
    
    m_iAmmo.Set( iAmmoIndex, m_iAmmo[iAmmoIndex] + iCount);
}

CBaseCombatWeapon *IBaseCombatCharacter::Weapon_Create( const char *pWeaponName )
{
	CBaseCombatWeapon *pWeapon = reinterpret_cast<CBaseCombatWeapon *>( CreateSpawn( pWeaponName, GetLocalOrigin(), GetLocalAngles(), this ) );

	return pWeapon;
}
