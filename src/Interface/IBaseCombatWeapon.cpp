#include "IBaseCombatWeapon.h"

bool IBaseCombatWeapon::UsesPrimaryAmmo(void)
{
    if(m_iPrimaryAmmoType < 0)
        return false;
    return true;
}

bool IBaseCombatWeapon::UsesSecondaryAmmo(void)
{
    if(m_iSecondaryAmmoType < 0)
        return false;
    return true;
}
