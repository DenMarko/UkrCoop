#include "CAmmoDef.h"

Ammo_t *GetAmmoOfIndex(_CAmmoDef *pThis, int nAmmoIndex)
{
	if(nAmmoIndex >= pThis->m_nAmmoIndex)
	{
		return 0;
	}

	return &pThis->m_AmmoType[nAmmoIndex];
}

int MaxCarry(_CAmmoDef *pThis, int nAmmoIndex)
{
	if(nAmmoIndex < 1 || nAmmoIndex >= pThis->m_nAmmoIndex)
	{
		return 0;
	}

	if(pThis->m_AmmoType[nAmmoIndex].pMaxCarry == -1)
	{
		if(pThis->m_AmmoType[nAmmoIndex].pMaxCarryCVar)
		{
			return pThis->m_AmmoType[nAmmoIndex].pMaxCarryCVar->GetFloat();
		}

		return 0;
	}
	else
	{
		return pThis->m_AmmoType[nAmmoIndex].pMaxCarry;
	}
}

int Index(_CAmmoDef *pThis, const char *psz)
{
	if(!psz)
	{
		return -1;
	}

	for(int i = 1; i < pThis->m_nAmmoIndex; i++)
	{
		if(g_Sample.my_strcmp(psz, pThis->m_AmmoType[i].pName) == 0)
		{
			return i;
		}
	}
	return -1;
}
