#ifndef _INCLUDE_AMMO_DEF_
#define _INCLUDE_AMMO_DEF_

#include "extension.h"

struct Ammo_t 
{
	char 				*pName;
	int					nDamageType;
	int					eTracerType;
	float				physicsForceImpulse;
	int					nMinSplashSize;
	int					nMaxSplashSize;

	int					nFlags;

	int					pPlrDmg;
	int					pNPCDmg;
	int					pMaxCarry;
	const ConVar*		pPlrDmgCVar;
	const ConVar*		pNPCDmgCVar;
	const ConVar*		pMaxCarryCVar;
};

typedef struct CAmmoDef
{
	void* vTable;
    
	int         m_nAmmoIndex;
    Ammo_t      m_AmmoType[32];
} _CAmmoDef;

Ammo_t		*GetAmmoOfIndex(_CAmmoDef *pThis, int nAmmoIndex);
int 		MaxCarry(_CAmmoDef *pThis, int nAmmoIndex);
int 		Index(_CAmmoDef *pThis, const char *psz);

#endif