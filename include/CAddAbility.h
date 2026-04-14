#ifndef _INCLUDE_CADDABILITY_PROPER_H_
#define _INCLUDE_CADDABILITY_PROPER_H_
#include "extension.h"
#include "CLuaPropProxy.h"
#include "HL2.h"

bool AddAbility(CBaseEntity* pThis, const char *szNameAbility, const char *szWeapon);
bool GiveDefaultAbility(CBaseEntity* pThis);

#endif