#ifndef _INCLUDE_CTRACE_PROPER_H_
#define _INCLUDE_CTRACE_PROPER_H_
#include "extension.h"

class IGameRules;
typedef bool (*ShouldHitFunc_t)( IHandleEntity *pHandleEntity, int contentsMask );

class CTraceFilterSimples : public CTraceFilter
{
public:
	CTraceFilterSimples( IHandleEntity *passentity, int collisionGroup, ShouldHitFunc_t pExtraShouldHitFunc = nullptr );
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );

	virtual void SetPassEntity( IHandleEntity *pPassEntity )
    {
        m_pPassEnt = pPassEntity;
    }

	virtual void SetCollisionGroup( int iCollisionGroup )
    {
        m_collisionGroup = iCollisionGroup;
    }

	const IHandleEntity *GetPassEntity( void )
    {
        return m_pPassEnt;
    }

private:
	IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
	ShouldHitFunc_t m_pExtraShouldHitCheckFunction;
};

class CTraceFilterNoNPC_OrPlayer : public CTraceFilterSimples
{
public:
	CTraceFilterNoNPC_OrPlayer( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimples( (IHandleEntity*)passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );
};

class NextBotTraceFilterOnlyActors : public CTraceFilterSimples
{
public:
	NextBotTraceFilterOnlyActors( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimples( (IHandleEntity*)passentity, collisionGroup )
	{
	}

	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask );
};

bool VisionTraceFilterFunction( IHandleEntity *pServerEntity, int contentsMask );

class NextBotVisionTraceFilter : public CTraceFilterSimples
{
public:
	NextBotVisionTraceFilter( const IHandleEntity *passentity, int collisionGroup )	: 
    CTraceFilterSimples( const_cast<IHandleEntity*>(passentity), collisionGroup, VisionTraceFilterFunction )
	{
	}
};

class CTraceFilterNavig : public CTraceFilterSimples
{
public:
    CTraceFilterNavig(const CBaseEntity *pProber, bool bIgnoreTransientEntities, IHandleEntity *passedict, int collisionGroup, bool bAllowPlayerAvoid = true );
	bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask );

private:
    const CBaseEntity *m_pProder;
    bool m_bIgnoreTransientEntitys;
    bool m_bCheckCollisionTable;
    bool m_bAllowPlayerAvoid;
    IPhysicsObjectPairHash *g_pCollisionHash;
};

class NextBotTraceFilterIgnoreActors : public CTraceFilterSimples
{
public:
	NextBotTraceFilterIgnoreActors( const IHandleEntity *passentity, int collisionGroup ) : CTraceFilterSimples( (IHandleEntity *)passentity, collisionGroup) { }

    virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask ); 
};

class CTraceFilterChain : public CTraceFilter
{
public:
	CTraceFilterChain( ITraceFilter *pTraceFilter1, ITraceFilter *pTraceFilter2 );
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );

private:
	ITraceFilter	*m_pTraceFilter1;
	ITraceFilter	*m_pTraceFilter2;
};

inline void util_TraceRay(const Ray_t &ray, unsigned int mask, IHandleEntity *ignore, int collisionGroup, trace_t *ptr )
{
    CTraceFilterSimples traceFilter(ignore, collisionGroup);
    g_pTarce->TraceRay(ray, mask, &traceFilter, ptr);
}

inline void util_TraceLine( const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, const IHandleEntity *ignore, int collisionGroup, trace_t *ptr )
{
    Ray_t ray;
    ray.Init(vecAbsStart, vecAbsEnd);
    CTraceFilterSimples traceFilter((IHandleEntity*)ignore, collisionGroup);
    g_pTarce->TraceRay(ray, mask, &traceFilter, ptr);
}

inline void util_TraceLine(const Ray_t &ray, unsigned int mask, IHandleEntity *ignore, int collisionGroup, trace_t *ptr)
{
    CTraceFilterSimples traceFilter(ignore, collisionGroup);
    g_pTarce->TraceRay(ray, mask, &traceFilter, ptr);
}

inline void util_TraceLine(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* pFilter, trace_t *ptr)
{
    Ray_t ray;
    ray.Init(vecAbsStart, vecAbsEnd);

    g_pTarce->TraceRay(ray, mask, pFilter, ptr);
}

inline void util_TraceHull(const Vector& vecAbsStart, const Vector& vecAbsEnd, const Vector& hullMin, const Vector& hullMax, unsigned int mask, ITraceFilter* pFilter, trace_t *ptr)
{
    Ray_t ray;
    ray.Init(vecAbsStart, vecAbsEnd, hullMin, hullMax);

    g_pTarce->TraceRay(ray, mask, pFilter, ptr);
}

void util_TraceLineFilterEntity(CBaseEntity* pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, int nCollisionGroup, trace_t *ptr);

#endif