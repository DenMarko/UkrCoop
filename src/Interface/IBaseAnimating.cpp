#include "IBaseAnimating.h"
#include "IEntityDissolve.h"
#include "datamanager.h"

int IBaseAnimating::LookupAttachment(const char *szName)
{
    CStudioHdr *pStudioHdr = GetModelPtr();
    if(pStudioHdr)
    {
        return StudioFindAttachment(pStudioHdr, szName) + 1;
    }
    return 0;
}

int IBaseAnimating::LookupBone(const char *szName)
{
    CStudioHdr *pStudioHdr = GetModelPtr();
    if(pStudioHdr)
    {
        return StudioBoneIndexByName(pStudioHdr, szName);
    }

    return 0;
}

CStudioHdr *IBaseAnimating::GetModelPtr()
{
    if(!m_pStudioHdr && GetModel())
    {
        LockStudioHdr();
    }

    return (m_pStudioHdr && m_pStudioHdr->IsValid()) ? m_pStudioHdr : nullptr;
}

int IBaseAnimating::LookupPoseParameter(CStudioHdr *pStudioHdr, const char *szName)
{
    if(!pStudioHdr && !SeqencesAvailable(pStudioHdr))
    {
        return 0;
    }

    for(int i = 0; i < GetNumPoseParameters(pStudioHdr); i++)
    {
        auto &mPoseParam = pPoseParameter(pStudioHdr, i);
        if(V_stricmp(mPoseParam.pszName(), szName) == 0)
        {
            return i;
        }
    }

    return -1;
}

void IBaseAnimating::SetSequence(int nSequence)
{
    m_nSequence = nSequence;
}

void IBaseAnimating::ResetSequence(int nSequence)
{
    if(!SequenceLoops())
    {
        SetCycle(0);
    }

    bool changed = nSequence != GetSequence() ? true : false;
    SetSequence(nSequence);
    if(changed || !SequenceLoops())
    {
        ResetSequenceInfo();
    }
}

void IBaseAnimating::ResetSequenceInfo()
{
    if(GetSequence() == -1)
    {
        SetSequence(0);
    }

    CStudioHdr* pStudioHdr = GetModelPtr();
    m_flGroundSpeed = GetSequenceGroundSpeed(pStudioHdr, GetSequence());
    m_bSequenceLoops = ((GetSequenceFlags(pStudioHdr, GetSequence()) & STUDIO_LOOPING) != 0);
	m_flPlaybackRate = 1.f;
    m_bSequenceFinished = false;
    m_flLastEventCheck = 0.f;
	m_nNewSequenceParity = (m_nNewSequenceParity + 1) & EF_PARITY_MASK;
	m_nResetEventsParity = (m_nResetEventsParity + 1) & EF_PARITY_MASK;

    if(pStudioHdr)
    {
        SetEventIndexForSequence(pSeqdesc(pStudioHdr, GetSequence()));
    }
}

FORWARD_DECLARE_HANDLE( memhandle_t );
struct bonecacheparams_t
{
	CStudioHdr		*pStudioHdr;
	matrix3x4_t		*pBoneToWorld;
	float			curtime;
	int				boneMask;
};

class IBoneCache
{
public:

	static IBoneCache *CreateResource( const bonecacheparams_t &params );
	static unsigned int EstimatedSize( const bonecacheparams_t &params );

	void			DestroyResource();
	IBoneCache		*GetData() { return this; }
	unsigned int	Size() { return m_size; }

	IBoneCache();

	void			Init( const bonecacheparams_t &params, unsigned int size, short *pStudioToCached, short *pCachedToStudio, int cachedBoneCount );
	
	void			UpdateBones( const matrix3x4_t *pBoneToWorld, int numbones, float curtime );
	matrix3x4_t		*GetCachedBone( int studioIndex );
	void			ReadCachedBones( matrix3x4_t *pBoneToWorld );
	void			ReadCachedBonePointers( matrix3x4_t **bones, int numbones );

	bool			IsValid( float curtime, float dt = 0.1f );

public:
	float			m_timeValid;
	int				m_boneMask;

private:
	matrix3x4_t		*BoneArray();
	short			*StudioToCached();
	short			*CachedToStudio();

	unsigned int	m_size;
	unsigned short	m_cachedBoneCount;
	unsigned short	m_matrixOffset;
	unsigned short	m_cachedToStudioOffset;
	unsigned short	m_boneOutOffset;
};

IBoneCache *IBoneCache::CreateResource( const bonecacheparams_t &params )
{
	short studioToCachedIndex[MAXSTUDIOBONES];
	short cachedToStudioIndex[MAXSTUDIOBONES];
	int cachedBoneCount = 0;
	DWORD *v1 = (DWORD*)params.pStudioHdr;

	for ( int i = 0; i < params.pStudioHdr->numbones(); i++ )
	{
		if (i != 0 && !(*(DWORD *)(v1[11] + 4 * i) & params.boneMask))
		{
			studioToCachedIndex[i] = -1;
			continue;
		}
		studioToCachedIndex[i] = cachedBoneCount;
		cachedToStudioIndex[cachedBoneCount] = i;
		cachedBoneCount++;
	}
	int tableSizeStudio = sizeof(short) * params.pStudioHdr->numbones();
	int tableSizeCached = sizeof(short) * cachedBoneCount;
	int matrixSize = sizeof(matrix3x4_t) * cachedBoneCount;
	int size = ( sizeof(IBoneCache) + tableSizeStudio + tableSizeCached + matrixSize + 3 ) & ~3;
	
	IBoneCache *pMem = (IBoneCache *)malloc( size );
	Construct( pMem );
	pMem->Init( params, size, studioToCachedIndex, cachedToStudioIndex, cachedBoneCount );
	return pMem;
}

unsigned int IBoneCache::EstimatedSize( const bonecacheparams_t &params )
{
	return ( params.pStudioHdr->numbones() * (sizeof(short) + sizeof(short) + sizeof(matrix3x4_t)) + 3 ) & ~3;
}

void IBoneCache::DestroyResource()
{
	free( this );
}

IBoneCache::IBoneCache()
{
	m_size = 0;
	m_cachedBoneCount = 0;
}

void IBoneCache::Init( const bonecacheparams_t &params, unsigned int size, short *pStudioToCached, short *pCachedToStudio, int cachedBoneCount ) 
{
	m_cachedBoneCount = cachedBoneCount;
	m_size = size;
	m_timeValid = params.curtime;
	m_boneMask = params.boneMask;

	int studioTableSize = params.pStudioHdr->numbones() * sizeof(short);
	m_cachedToStudioOffset = studioTableSize;
	memcpy( StudioToCached(), pStudioToCached, studioTableSize );

	int cachedTableSize = cachedBoneCount * sizeof(short);
	memcpy( CachedToStudio(), pCachedToStudio, cachedTableSize );

	m_matrixOffset = ( m_cachedToStudioOffset + cachedTableSize + 3 ) & ~3;
	
	UpdateBones( params.pBoneToWorld, params.pStudioHdr->numbones(), params.curtime );
}

void IBoneCache::UpdateBones( const matrix3x4_t *pBoneToWorld, int numbones, float curtime )
{
	matrix3x4_t *pBones = BoneArray();
	const short *pCachedToStudio = CachedToStudio();

	for ( int i = 0; i < m_cachedBoneCount; i++ )
	{
		int index = pCachedToStudio[i];
		MatrixCopy( pBoneToWorld[index], pBones[i] );
	}
	m_timeValid = curtime;
}

matrix3x4_t *IBoneCache::GetCachedBone( int studioIndex )
{
	int cachedIndex = StudioToCached()[studioIndex];
	if ( cachedIndex >= 0 )
	{
		return BoneArray() + cachedIndex;
	}
	return NULL;
}

void IBoneCache::ReadCachedBones( matrix3x4_t *pBoneToWorld )
{
	matrix3x4_t *pBones = BoneArray();
	const short *pCachedToStudio = CachedToStudio();
	for ( int i = 0; i < m_cachedBoneCount; i++ )
	{
		MatrixCopy( pBones[i], pBoneToWorld[pCachedToStudio[i]] );
	}
}

void IBoneCache::ReadCachedBonePointers( matrix3x4_t **bones, int numbones )
{
	memset( bones, 0, sizeof(matrix3x4_t *) * numbones );
	matrix3x4_t *pBones = BoneArray();
	const short *pCachedToStudio = CachedToStudio();
	for ( int i = 0; i < m_cachedBoneCount; i++ )
	{
		bones[pCachedToStudio[i]] = pBones + i;
	}
}

bool IBoneCache::IsValid( float curtime, float dt )
{
	if ( curtime - m_timeValid <= dt )
		return true;
	return false;
}

matrix3x4_t *IBoneCache::BoneArray()
{
	return (matrix3x4_t *)( (char *)(this+1) + m_matrixOffset );
}

short *IBoneCache::StudioToCached()
{
	return (short *)( (char *)(this+1) );
}

short *IBoneCache::CachedToStudio()
{
	return (short *)( (char *)(this+1) + m_cachedToStudioOffset );
}

bool IBaseAnimating::ComputeHitboxSurroundingBox(Vector *pVecWorldMins, Vector *pVecWorldMaxs)
{
    auto pStudio = GetModelPtr();
    if(!pStudio)
	{
        return false;
	}
	
    mstudiohitboxset_t *set = pStudio->pHitboxSet(m_nHitboxSet);
    if(!set || !set->numhitboxes)
    {
        return false;
    }
    
	IBoneCache *pCache = GetBoneCache();

	pVecWorldMins->Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pVecWorldMaxs->Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	Vector vecBoxAbsMins, vecBoxAbsMaxs;
	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox(i);
		matrix3x4_t *pMatrix = pCache->GetCachedBone(pbox->bone);

		if ( pMatrix )
		{
			TransformAABB( *pMatrix, pbox->bbmin, pbox->bbmax, vecBoxAbsMins, vecBoxAbsMaxs );
			VectorMin( *pVecWorldMins, vecBoxAbsMins, *pVecWorldMins );
			VectorMax( *pVecWorldMaxs, vecBoxAbsMaxs, *pVecWorldMaxs );
		}
	}
	return true;
}

bool IBaseAnimating::ComputeEntitySpaceHitboxSurroundingBox(Vector *pVecWorldMins, Vector *pVecWorldMaxs)
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
	{
		return false;
	}

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( m_nHitboxSet );
	if ( !set || !set->numhitboxes )
		return false;

	IBoneCache *pCache = GetBoneCache();
	matrix3x4_t *hitboxbones[MAXSTUDIOBONES];
	pCache->ReadCachedBonePointers( hitboxbones, pStudioHdr->numbones() );

	pVecWorldMins->Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pVecWorldMaxs->Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	matrix3x4_t worldToEntity, boneToEntity;
	MatrixInvert( EntityToWorldTransform(), worldToEntity );

	Vector vecBoxAbsMins, vecBoxAbsMaxs;
	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox(i);

		ConcatTransforms( worldToEntity, *hitboxbones[pbox->bone], boneToEntity );
		TransformAABB( boneToEntity, pbox->bbmin, pbox->bbmax, vecBoxAbsMins, vecBoxAbsMaxs );
		VectorMin( *pVecWorldMins, vecBoxAbsMins, *pVecWorldMins );
		VectorMax( *pVecWorldMaxs, vecBoxAbsMaxs, *pVecWorldMaxs );
	}
	return true;
}

static CDataManager<IBoneCache, bonecacheparams_t, IBoneCache *, CThreadFastMutex> g_StudioBoneCache( 24 * 1024L );

NOINLINE IBoneCache *Studio_GetBoneCache( memhandle_t cacheHandle )
{
	AUTO_LOCK( g_StudioBoneCache.AccessMutex() );
	return g_StudioBoneCache.GetResource_NoLock( cacheHandle );
}

NOINLINE memhandle_t Studio_CreateBoneCache( bonecacheparams_t &params )
{
	AUTO_LOCK( g_StudioBoneCache.AccessMutex() );
	return g_StudioBoneCache.CreateResource( params );
}

NOINLINE void Studio_DestroyBoneCache( memhandle_t cacheHandle )
{
	AUTO_LOCK( g_StudioBoneCache.AccessMutex() );
	g_StudioBoneCache.DestroyResource( cacheHandle );
}

NOINLINE void Studio_InvalidateBoneCache( memhandle_t cacheHandle )
{
	AUTO_LOCK( g_StudioBoneCache.AccessMutex() );
	IBoneCache *pCache = g_StudioBoneCache.GetResource_NoLock( cacheHandle );
	if ( pCache )
	{
		pCache->m_timeValid = -1.0f;
	}
}

IBoneCache *IBaseAnimating::GetBoneCache()
{
	Msg("[GetBoneCache]\n");
	CStudioHdr *pStudioHdr = GetModelPtr( );
	Assert(pStudioHdr);

	IBoneCache *pcache = Studio_GetBoneCache( m_boneCacheHandle );
	int boneMask = BONE_USED_BY_HITBOX | BONE_USED_BY_ATTACHMENT;

	if ( pcache )
	{
		if ( pcache->IsValid( g_pGlobals->curtime ) && (pcache->m_boneMask & boneMask) == boneMask && pcache->m_timeValid <= g_pGlobals->curtime)
		{
			return pcache;
		}

		if ( (pcache->m_boneMask & boneMask) != boneMask )
		{
			Studio_DestroyBoneCache( m_boneCacheHandle );
			m_boneCacheHandle = 0;
			pcache = NULL;
		}
	}

	matrix3x4_t bonetoworld[MAXSTUDIOBONES];
	SetupBones( bonetoworld, boneMask );

	if ( pcache )
	{
		pcache->UpdateBones( bonetoworld, pStudioHdr->numbones(), g_pGlobals->curtime );
	}
	else
	{
		bonecacheparams_t params;
		params.pStudioHdr = pStudioHdr;
		params.pBoneToWorld = bonetoworld;
		params.curtime = g_pGlobals->curtime;
		params.boneMask = boneMask;

		m_boneCacheHandle = Studio_CreateBoneCache( params );
		pcache = Studio_GetBoneCache( m_boneCacheHandle );
	}
	return pcache;
}

float IBaseAnimating::GetModelScale() const
{
    return 1.0f;
}

int IBaseAnimating::SelectWeightedSequence(Activity activity)
{
    return g_CallHelper->Select_Weighted_Sequence(GetModelPtr(), activity, GetSequence());
}

int IBaseAnimating::SelectWeightedSequence(Activity activity, int curSequence)
{
    return g_CallHelper->Select_Weighted_Sequence(GetModelPtr(), activity, curSequence);
}

float IBaseAnimating::SequenceDuration(CStudioHdr *pStudioHdr, int iSequence)
{
	if ( !pStudioHdr )
	{
		DevWarning( 2, "CBaseAnimating::SequenceDuration( %d ) NULL pstudiohdr on %s!\n", iSequence, GetClassname().ToCStr() );
		return 0.1;
	}
	if ( !SeqencesAvailable(pStudioHdr) )
	{
		return 0.1;
	}
	if (iSequence >= GetNumSeq(pStudioHdr) || iSequence < 0 )
	{
		DevWarning( 2, "CBaseAnimating::SequenceDuration( %d ) out of range\n", iSequence );
		return 0.1;
	}

	return Studio_Duration(pStudioHdr, iSequence, GetPoseParameterArray());
}

const float *IBaseAnimating::GetPoseParameterArray()
{
    return m_flPoseParameter.Base();
}

LocalFlexController_t IBaseAnimating::GetNumFlexControllers(void)
{
	CStudioHdr *pstudiohdr = GetModelPtr();
	if(!pstudiohdr)
    	return LocalFlexController_t(0);

	return pstudiohdr->numflexcontrollers();
}

const char *IBaseAnimating::GetFlexControllerName(LocalFlexController_t iFlexController)
{
	CStudioHdr *pstudiohdr = GetModelPtr();
	if(!pstudiohdr)
    	return 0;

	mstudioflexcontroller_t* pflexcontroller = pstudiohdr->pFlexcontroller(iFlexController);
	return pflexcontroller->pszName();
}

void IBaseAnimating::GetBonePosition(int iBone, Vector &origin, QAngle &angles)
{
	CStudioHdr* pStudioHdr = GetModelPtr();
	if(!pStudioHdr)
	{
		Msg("CBaseAnimating::GetBonePosition: model missing\n");
		return;
	}

	if(iBone < 0 || iBone >= pStudioHdr->numbones())
	{
		Msg("CBaseAnimating::GetBonePosition: invalid bone index\n");
		return;
	}

	matrix3x4_t bonetoworld;
	GetBoneTransform(iBone, bonetoworld);
	MatrixAngles(bonetoworld, angles, origin);
}

bool IBaseAnimating::GetAttachment(const char *szName, Vector &absOrigin, Vector *forward, Vector *right, Vector *up)
{
	return GetAttachment(LookupAttachment(szName), absOrigin, forward, right, up);
}

bool IBaseAnimating::GetAttachment(int iAttachment, Vector &absOrigin, Vector *forward, Vector *right, Vector *up)
{
	matrix3x4_t attachmentToWorld;
	bool bRet = GetAttachment(iAttachment, attachmentToWorld);
	MatrixPosition(attachmentToWorld, absOrigin);
	if(forward)
		MatrixGetColumn(attachmentToWorld, 0, forward);

	if(right)
		MatrixGetColumn(attachmentToWorld, 1, right);

	if(up)
		MatrixGetColumn(attachmentToWorld, 2, up);

	return bRet;
}

bool IBaseAnimating::GetAttachment(const char *szName, Vector &absOrigin, QAngle &absAngles)
{
    return GetAttachment(LookupAttachment(szName), absOrigin, absAngles);
}

bool IBaseAnimating::GetAttachment(int iAttachment, Vector &absOrigin, QAngle &absAngles)
{
	matrix3x4_t attachmentToWorld;

	bool bRet = GetAttachment( iAttachment, attachmentToWorld );
	MatrixAngles( attachmentToWorld, absAngles, absOrigin );
	return bRet;
}

float IBaseAnimating::GetSequenceCycleRate(CStudioHdr *pStudioHdr, int iSequence)
{
	float t = SequenceDuration(pStudioHdr, iSequence);
	if(t > 0.f)
	{
    	return 1.f / t;
	}
	else
	{
		return 1.f / 0.1f;
	}
}

float IBaseAnimating::GetAnimTimeInterval(void) const
{
	float flInterval;
	if(m_flAnimTime < g_pGlobals->curtime)
	{
		flInterval = clamp(g_pGlobals->curtime - m_flAnimTime, 0, 0.2f);
	}
	else
	{
		flInterval = clamp(m_flAnimTime - m_flPrevAnimTime, 0, 0.2f);
	}

    return flInterval;
}

bool IBaseAnimating::Dissolve(const char *pMaterialName, float flStartTime, bool bNPCOnly, int nDissolveType, Vector vDissolveOrigin, int iMagnitude)
{
	if(bNPCOnly && !(GetFlags() & FL_NPC))
		return false;

	if(IsDissolving())
		return false;

	bool bRagdollCreated = false;
	IEntityDissolve *pDissolve = EntDissolve::Create(this, pMaterialName, flStartTime, nDissolveType, &bRagdollCreated);
	if(pDissolve)
	{
		SetEffectEntity(pDissolve);
		AddFlag(FL_DISSOLVING);
		m_flDissolveStartTime = flStartTime;
		pDissolve->SetDissolverOrigin(vDissolveOrigin);
		pDissolve->SetMagnitude(iMagnitude);
	}

	if((CLASS_NONE == Classify()) && (ClassMatches("prop_ragdoll")))
	{
		IGameEvent* event = gameevents->CreateEvent("ragdool_dissolved");
		if(event)
		{
			event->SetInt("entindex", entindex());
			gameevents->FireEvent(event);
		}
	}

    return bRagdollCreated;
}

float IBaseAnimating::SetPoseParameter( CStudioHdr *pStudioHdr, const char *szName, float flValue )
{
	int poseParam = LookupPoseParameter( pStudioHdr, szName );
	return SetPoseParameter( pStudioHdr, poseParam, flValue );
}

float IBaseAnimating::SetPoseParameter( CStudioHdr *pStudioHdr, int iParameter, float flValue )
{
	if ( !pStudioHdr )
	{
		return flValue;
	}

	if (iParameter >= 0)
	{
		float flNewValue;
		flValue = Studio_SetPoseParameter( pStudioHdr, iParameter, flValue, flNewValue );
		m_flPoseParameter.Set( iParameter, flNewValue);
	}

	return flValue;
}

float IBaseAnimating::GetPoseParameter( const char *szName )
{
	return GetPoseParameter( LookupPoseParameter( szName ) );
}

float IBaseAnimating::GetPoseParameter( int iParameter )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );

	if ( !pstudiohdr )
	{
		return 0.0;
	}

	if ( !SeqencesAvailable(pstudiohdr) )
	{
		return 0;
	}

	if (iParameter >= 0)
	{
		return Studio_GetPoseParameter( pstudiohdr, iParameter, m_flPoseParameter[ iParameter ] );
	}

	return 0.0;
}

bool IBaseAnimating::HasPoseParameter(int iSequence, const char *szName)
{
	int iParameter = LookupPoseParameter( szName );
	if(iParameter == -1)
	{
    	return false;
	}

	return HasPoseParameter(iSequence, iParameter);
}

bool IBaseAnimating::HasPoseParameter(int iSequence, int iParameter)
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if(!pstudiohdr)
	{
		return false;
	}

	if(!SeqencesAvailable(pstudiohdr))
	{
		return false;
	}

	if(iSequence < 0 || iSequence >= GetNumSeq(pstudiohdr))
	{
		return false;
	}

	mstudioseqdesc_t &seqdesc = pSeqdesc(pstudiohdr, iSequence);
	if(GetSharedPoseParameter(pstudiohdr, iSequence, seqdesc.paramindex[0]) == iParameter ||
	   GetSharedPoseParameter(pstudiohdr, iSequence, seqdesc.paramindex[1]) == iParameter)
	{
		return true;
	}

    return false;
}

int IBaseAnimating::FindBodygroupByName( const char *szName )
{
	return ::FindBodygroupByName( GetModelPtr(), szName );
}

int IBaseAnimating::GetNumBodyGroups( void )
{
	return ::GetNumBodyGroups( GetModelPtr() );
}

int IBaseAnimating::GetBodygroup( int iGroup )
{
	return ::GetBodygroup( GetModelPtr(), m_nBody, iGroup );
}

const char *IBaseAnimating::GetBodygroupPartName(int iGroup, int iPart)
{
    return ::GetBodygroupPartName( GetModelPtr(), iGroup, iPart );
}

int IBaseAnimating::GetBodygroupCount( int iGroup )
{
	return ::GetBodygroupCount( GetModelPtr(), iGroup );
}

void IBaseAnimating::SetBodygroup( int iGroup, int iValue )
{
	int newBody = m_nBody;
	::SetBodygroup( GetModelPtr(), newBody, iGroup, iValue );
	m_nBody = newBody;
}

Activity IBaseAnimating::GetSequenceActivity(int iSequence)
{
	if(iSequence == -1)
		return ACT_INVALID;

	if(!GetModelPtr())
		return ACT_INVALID;

    return (Activity)::GetSequenceActivity(GetModelPtr(), iSequence);
}

void IBaseAnimating::SetFadeDistance(float minFadeDist, float maxFadeDist)
{
	m_fadeMinDist = minFadeDist;
	m_fadeMaxDist = maxFadeDist;
}
