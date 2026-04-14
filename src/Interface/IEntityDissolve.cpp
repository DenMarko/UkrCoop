#include "IEntityDissolve.h"
#include "IBasePlayer.h"
#include "HL2.h"

#define DISSOLVE_SPRITE_NAME	"sprites/blueglow1.vmt"	

void IEntityDissolve::AttachToEntity(IBaseEntity *pTarget)
{
    SetParent((CBaseEntity*)pTarget);
    SetLocalOrigin(vec3_origin);
    SetLocalAngles(vec3_angle);
}

void IEntityDissolve::SetStartTime(float flStartTime)
{
	m_flStartTime = flStartTime;
}

namespace EntDissolve
{
    IEntityDissolve *Create(IBaseEntity *pTarget, const char *pMaterialName, float flStartTime, int nDissolveType, bool *pRagdollCreated)
    {
        if(pRagdollCreated)
        {
            *pRagdollCreated = false;
        }

        if(!pMaterialName)
        {
            pMaterialName = DISSOLVE_SPRITE_NAME;
        }

        if(pTarget->IsPlayer())
        {
            IBasePlayer *pPlayer = access_dynamic_cast<IBasePlayer>(pTarget, "CBasePlayer");
            if(pPlayer)
            {
                pPlayer->SetArmorValue(0);
                CTakeDamageInfoHack info(pPlayer, pPlayer, pPlayer->GetHealth(), DMG_GENERIC | DMG_REMOVENORAGDOLL | DMG_PREVENT_PHYSICS_FORCE);
                pPlayer->TakeDamage(info);
            }
        }

        IEntityDissolve *pDissolve = nullptr;
        if((pDissolve = (IEntityDissolve*)g_Sample.CreateEntityByName("env_entity_dissolver")) == nullptr)
            return nullptr;

        pDissolve->m_nDissolveType = nDissolveType;
        pDissolve->SetModelName(g_CallHelper->AllocPooledString(pMaterialName));
        pDissolve->AttachToEntity(pTarget);
        pDissolve->SetStartTime(flStartTime);
        pDissolve->Spawn();

        pDissolve->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
        if((nDissolveType == ENTITY_DISSOLVE_ELECTRICAL) || (nDissolveType == ENTITY_DISSOLVE_ELECTRICAL_LIGHT))
        {
            pTarget->DispatchResponse("TLK_ELECTROCUTESCREAM");
        }
        else
        {
            pTarget->DispatchResponse("TLK_DISSOLVESCREAM");
        }

        return pDissolve;
    }

    IEntityDissolve *Create(IBaseEntity *pTarget, IBaseEntity *pSource)
    {
        for(IBaseEntity* pChild = pSource->FirstMoveChild(); pChild; pChild = pChild->NextMovePeer())
        {
            IEntityDissolve *pDissolve = access_dynamic_cast<IEntityDissolve>(pChild, "CEntityDissolve");
            if(!pDissolve)
                continue;

            return Create(pTarget, STRING(pDissolve->GetModelName()), pDissolve->m_flStartTime, pDissolve->m_nDissolveType);
        }

        return nullptr;
    }
}