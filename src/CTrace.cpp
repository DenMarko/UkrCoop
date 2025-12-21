#include "CTrace.h"
#include <ICollideable.h>
#include "HL2.h"
#include "CLuaPropProxy.h"
#include "Interface/ISurvivorBot.h"
#include "Interface/IGameRules.h"

struct worldbrushdata_t;
class CEngineSprite;
ConVar ukr_moveprode_ignoresmall("ukr_moveprode_ignoresmall", "0", 0, "Ignore small props", true, 0.f, true, 1.0f);

enum modtype_t
{
	mod_bad = 0, 
	mod_brush, 
	mod_sprite, 
	mod_studio
};

struct brushdata_t
{
	worldbrushdata_t	*pShared;
	int			firstmodelsurface, nummodelsurfaces;

	unsigned short	renderHandle;
	unsigned short	firstnode;
};

struct spritedata_t
{
	int				numframes;
	int				width;
	int				height;
	CEngineSprite	*sprite;
};

struct model_t
{
	void*				fnHandle;
	char				szName[96];

	int					nLoadFlags;
	int					nServerCount;

	modtype_t			type;
	int					flags;

	Vector				mins, maxs;
	float				radius;

	union
	{
		brushdata_t		brush;
		MDLHandle_t		studio;
		spritedata_t	sprite;
	};
};

NOINLINE bool IsStaticProp( CBaseHandle handle )
{
	return (handle.GetSerialNumber() == (0x40000000 >> NUM_ENT_ENTRY_BITS));
}

NOINLINE IBaseEntity *GetEntityFromEntityHandle( IHandleEntity *pHandleEntity )
{
	IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
	IBaseEntity *pEnt = reinterpret_cast<IBaseEntity*>(pUnk->GetBaseEntity());
	if (IsStaticProp(pEnt->GetRefEHandle()))
		return nullptr;

	return pEnt;
}

IBasePlayer* GetBasePlayer(IBaseEntity* _pEntity)
{
	if(!_pEntity || !_pEntity->IsPlayer())
		return nullptr;

	return static_cast<IBasePlayer*>(_pEntity);
}

CTraceFilterSimples::CTraceFilterSimples( IHandleEntity *passedict, int collisionGroup, ShouldHitFunc_t pExtraShouldHitFunc )
{
	m_pPassEnt = passedict;
	m_collisionGroup = collisionGroup;
	m_pExtraShouldHitCheckFunction = pExtraShouldHitFunc;
}

bool CTraceFilterNoNPC_OrPlayer::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	if ( CTraceFilterSimples::ShouldHitEntity( pHandleEntity, contentsMask ) )
	{
		IBaseEntity *pEntity = GetEntityFromEntityHandle( pHandleEntity );
		if ( !pEntity )
			return NULL;

		if ( pEntity->Classify() == 2 )
			return false;

		return (!pEntity->IsNPC() && !pEntity->IsPlayer());
	}
	return false;
}


NOINLINE bool StandartFilterRules(IHandleEntity* pHandleEntity, int iContentsMask)
{
	IBaseEntity *pEnt = nullptr;
    if((pEnt = GetEntityFromEntityHandle(pHandleEntity)) == nullptr)
	{
        return true;
	}

    SolidType_t solid = pEnt->GetSolid();
	const model_t* pModel = pEnt->GetModel();

    if(((pModel ? pModel->type : -1) != mod_brush) || (solid != SOLID_BSP && solid != SOLID_VPHYSICS))
    {
        if((iContentsMask & 0x2000000) == 0)
		{
            return false;
		}
    }

    if(!(iContentsMask & 0x2) &&  pEnt->IsTransparent())
	{
        return false;
	}

	if(!(iContentsMask & 0x4000) && (pEnt->GetMoveType() == MOVETYPE_PUSH))
	{
		return false;
	}

    return true;
}

NOINLINE bool PassServerEntityFilter(IHandleEntity *pTouch, IHandleEntity *pPass)
{
    if(!pPass)
        return true;

    if(pTouch == pPass)
        return false;

    IBaseEntity *pEntTouch = GetEntityFromEntityHandle(pTouch);
    IBaseEntity *pEntPass  = GetEntityFromEntityHandle(pPass);

    if(!pEntPass || !pEntTouch)
        return true;

    if(pEntTouch->GetOwnerEntity() == pEntPass)
        return false;

    if(pEntPass->GetOwnerEntity() == pEntTouch)
        return false;

    return true;
}

bool CTraceFilterSimples::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
    if(!StandartFilterRules(pHandleEntity, contentsMask))
	{
        return false;
	}

    if(m_pPassEnt)
    {
        if(!PassServerEntityFilter(pHandleEntity, m_pPassEnt))
		{
            return false;
		}
    }

    IBaseEntity *pEntity = GetEntityFromEntityHandle(pHandleEntity);
    if(!pEntity)
	{
        return false;
	}

    if(!pEntity->ShouldCollide(m_collisionGroup, contentsMask))
	{
        return false;
	}

	int iCollideGroup = pEntity->GetCollideable()->GetCollisionGroup();
	if(!g_HL2->GetGameRules()->ShouldCollide(m_collisionGroup, iCollideGroup))
	{
		return false;
	}

	if ( m_pExtraShouldHitCheckFunction && (! ( m_pExtraShouldHitCheckFunction( pHandleEntity, contentsMask ) ) ) )
	{
		return false;
	}

    return true;
}

NOINLINE bool ShouldPropbeCollideAgainstEntity(IBaseEntity *pEnt)
{
	if(ukr_moveprode_ignoresmall.GetBool())
	{
		if(pEnt->GetMoveType() == MOVETYPE_VPHYSICS)
		{
			IPhysicsObject *pPhysics = nullptr;
			if((pPhysics = pEnt->VPhysicsGetObject()) != nullptr)
			{
				if(pPhysics->IsMoveable() && pPhysics->GetMass() < 40.0f)
				{
					return false;
				}
			}
		}
	}
	return true;
}

CTraceFilterNavig::CTraceFilterNavig(const CBaseEntity *pProber, bool bIgnoreTransientEntities, IHandleEntity *passedict, int collisionGroup, bool bAllowPlayerAvoid) : CTraceFilterSimples(passedict, collisionGroup)
{
	m_pProder = pProber;
	m_bIgnoreTransientEntitys = bIgnoreTransientEntities;
	m_bAllowPlayerAvoid = bAllowPlayerAvoid;
	g_pCollisionHash = g_HL2->GetCollisionHash();
	if(g_pCollisionHash != nullptr)
		m_bCheckCollisionTable = g_pCollisionHash->IsObjectInHash((void *)pProber);
	else
		m_bCheckCollisionTable = false;
}

bool CTraceFilterNavig::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	IServerEntity *PServerEnt = (IServerEntity *)pServerEntity;
	CBaseEntity *pEnt = (CBaseEntity *)PServerEnt;

	if(m_pProder == pEnt)
		return false;

	if(m_bIgnoreTransientEntitys && (reinterpret_cast<IBasePlayer *>(pEnt)->IsPlayer() || g_HL2->IsNPC(pEnt)))
		return false;

	if(m_bAllowPlayerAvoid && reinterpret_cast<IBasePlayer *>(pEnt)->IsPlayer())
		return false;

	if(m_bCheckCollisionTable)
	{
		if(g_pCollisionHash->IsObjectPairInHash((void *)m_pProder, pEnt))
			return false;
	}

	if(!ShouldPropbeCollideAgainstEntity((IBaseEntity *)pEnt))
		return false;

	return CTraceFilterSimples::ShouldHitEntity(pServerEntity, contentsMask);
}

bool UTIL_EntHasMatchingRootParent(IBaseEntity *pRootParent, IBaseEntity* pEntity)
{
	if(pRootParent)
	{
		if(pRootParent == pEntity->GetRootMoveParent())
			return true;

		if(pEntity->GetOwnerEntity() && pRootParent == pEntity->GetRootMoveParent()->GetOwnerEntity())
			return true;
	}
	return false;
}

class CTraceFilterEntity : public CTraceFilterSimples
{
public:
    CTraceFilterEntity(IBaseEntity* pEntity, int nCollisionGroup) : CTraceFilterSimples(pEntity->GetNetworkable()->GetEntityHandle(), nCollisionGroup)
    {
		m_pRootParent = pEntity->GetRootMoveParent();
		m_pEntity = pEntity;
		m_checkHash = g_HL2->GetCollisionHash() ? g_HL2->GetCollisionHash()->IsObjectInHash(pEntity) : false;
	}

	bool ShouldHitEntity(IHandleEntity* pHandle, int nContentMask)
	{
		IBaseEntity *pEntity = GetEntityFromEntityHandle(pHandle);
		if(!pEntity)
			return false;

		if(UTIL_EntHasMatchingRootParent(m_pRootParent, pEntity))
			return false;

		if(m_checkHash)
		{
			auto m_collisionHash = g_HL2->GetCollisionHash();
			if(m_collisionHash)
			{
				if(m_collisionHash->IsObjectPairInHash(m_pRootParent, pEntity))
					return false;
			}
		}

		return CTraceFilterSimples::ShouldHitEntity(pHandle, nContentMask);
	}
private:
	IBaseEntity *m_pRootParent;
	IBaseEntity *m_pEntity;
	bool m_checkHash;
};

void util_TraceLineFilterEntity(CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, int nCollisionGroup, trace_t *ptr)
{
	CTraceFilterEntity traceFilter((IBaseEntity *)pEntity, nCollisionGroup);
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);

	g_pTarce->TraceRay(ray, mask, &traceFilter, ptr);
}

bool NextBotTraceFilterOnlyActors::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
    if ( CTraceFilterSimples::ShouldHitEntity( pServerEntity, contentsMask ) )
    {
        IBaseEntity *entity = GetEntityFromEntityHandle( pServerEntity );

        IBasePlayer *player = GetBasePlayer( entity );
        if ( player && player->IsGhost() )
            return false;

        return ( entity->MyNextBotPointer() || entity->IsPlayer() );
    }
    return false;
}

bool NextBotTraceFilterIgnoreActors::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if ( CTraceFilterSimples::ShouldHitEntity( pServerEntity, contentsMask ) )
	{
		auto IgnoreActorsTraceFilterFunction = [](IBaseEntity* pEnt){ return (pEnt->MyCombatCharacterPointer() == NULL); };

		if(IgnoreActorsTraceFilterFunction(GetEntityFromEntityHandle( pServerEntity )))
		{
			return true;
		}
	}
    return false;
}

CTraceFilterChain::CTraceFilterChain( ITraceFilter *pTraceFilter1, ITraceFilter *pTraceFilter2 )
{
	m_pTraceFilter1 = pTraceFilter1;
	m_pTraceFilter2 = pTraceFilter2;
}

bool CTraceFilterChain::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	bool bResult1 = true;
	bool bResult2 = true;

	if ( m_pTraceFilter1 )
		bResult1 = m_pTraceFilter1->ShouldHitEntity( pHandleEntity, contentsMask );

	if ( m_pTraceFilter2 )
		bResult2 = m_pTraceFilter2->ShouldHitEntity( pHandleEntity, contentsMask );

	return ( bResult1 && bResult2 );
}

bool VisionTraceFilterFunction(IHandleEntity *pServerEntity, int contentsMask)
{
	IBaseEntity *entity = GetEntityFromEntityHandle( pServerEntity );
	return ( entity->MyCombatCharacterPointer() == NULL && entity->BlocksLOS() );
}
