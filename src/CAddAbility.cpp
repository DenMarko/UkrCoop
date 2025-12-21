#include "CAddAbility.h"
#include "itoolentity.h"
#include "Interface/IBaseAbility.h"

bool AddAbility(CBaseEntity *pThis, const char *szNameAbility, const char *szWeapon)
{
    ITerrorPlayer* g_pEnt = GetVirtualClass<ITerrorPlayer>(pThis);

    g_pEnt->RemoveAllItems(true);
    g_pEnt->GiveNamedItem(szWeapon, 0, true);

    EHANDLE &m_customAbility = access_member<EHANDLE>(pThis, 7736);
    if(m_customAbility) {
        g_CallHelper->UTIL_Remove(m_customAbility);
        m_customAbility.Term();
    }

    QAngle pAngel = g_pEnt->GetAbsAngles();
    Vector pEyePos = g_pEnt->EyePosition();

    IBaseAbility *pAbility = (IBaseAbility*)CreateSpawn(szNameAbility, pEyePos, pAngel, (IBaseEntity *)pThis);
    if(pAbility)
    {
        m_customAbility = pAbility->GetRefEHandle();
        pAbility->OnCreate((CTerrorPlayer *)pThis);
        return true;
    }

    m_customAbility.Term();
    return false;
}

bool GiveDefaultAbility(CBaseEntity *pThis)
{
    ITerrorPlayer *g_pPlayer = GetVirtualClass<ITerrorPlayer>(pThis);

	g_pPlayer->RemoveAllItems(true);
	g_pPlayer->GiveNamedItem("weapon_pistol", 0, true);

    IBaseEntity *pEntity = g_pPlayer->GetCustomAbility();
    if(pEntity)
    {
        g_CallHelper->UTIL_Remove(pEntity);
    }

    return true;
}
