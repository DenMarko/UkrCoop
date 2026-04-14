#include "ICollisionProperty.h"
#include "IBaseAnimating.h"
#include "IServerNetworkPropperty.h"
#include "Interface/IGameRules.h"
#include "tier0/tslist.h"

CThreadLocalBase::CThreadLocalBase()
{
	if ( pthread_key_create( &m_index, NULL ) != 0 )
		Error( "Out of thread local storage!\n" );
}

//---------------------------------------------------------

CThreadLocalBase::~CThreadLocalBase()
{
	pthread_key_delete( m_index );
}

//---------------------------------------------------------

void * CThreadLocalBase::Get() const
{
	void *value = pthread_getspecific( m_index );
	return value;
}

//---------------------------------------------------------

void CThreadLocalBase::Set( void *value )
{
	if ( pthread_setspecific( m_index, value ) != 0 )
		AssertMsg( 0, "Bad thread local" );
}

class IDirtySpatialPartitionEntityList : public IAutoGameSystem, public IPartitionQueryCallback
{
public:
	IDirtySpatialPartitionEntityList( char const *name );

	// Members of IGameSystem
	virtual bool Init();
	virtual void Shutdown();
	virtual void LevelShutdownPostEntity();

	// Members of IPartitionQueryCallback
	virtual void OnPreQuery_V1()	{}
	virtual void OnPreQuery( SpatialPartitionListMask_t listMask );
	virtual void OnPostQuery( SpatialPartitionListMask_t listMask );

	void AddEntity( IBaseEntity *pEntity );
	void LockPartitionForRead()
	{
		if ( m_readLockCount == 0 )
		{
			m_partitionMutex.LockForRead();
		}
		m_readLockCount++;
	}
	void UnlockPartitionForRead()
	{
		m_readLockCount--;
		if ( m_readLockCount == 0 )
		{
			m_partitionMutex.UnlockRead();
		}
	}
	
	~IDirtySpatialPartitionEntityList();


private:
	CTSListWithFreeList<CBaseHandle>	m_DirtyEntities;
	CThreadSpinRWLock					m_partitionMutex;
	uint32								m_partitionWriteId;
	CThreadLocalInt<>					m_readLockCount;
};


ICollisionProperty::ICollisionProperty()
{
    m_Partition = PARTITION_INVALID_HANDLE;
    Init(NULL);
}

ICollisionProperty::~ICollisionProperty()
{
    DestroyPartitionHandle();
}

void ICollisionProperty::Init(IBaseEntity *pEntity)
{
	m_pOuter = pEntity;
	m_vecMins.GetForModify().Init();
	m_vecMaxs.GetForModify().Init();
	m_flRadius = 0.0f;
	m_triggerBloat = 0;
	m_usSolidFlags = 0;
	m_nSolidType = SOLID_NONE;

	m_nSurroundType = USE_OBB_COLLISION_BOUNDS;
	m_vecSurroundingMins = vec3_origin;
	m_vecSurroundingMaxs = vec3_origin;
	m_vecSpecifiedSurroundingMins.GetForModify().Init();
	m_vecSpecifiedSurroundingMaxs.GetForModify().Init();
}

IHandleEntity *ICollisionProperty::GetEntityHandle()
{
    return m_pOuter;
}

int ICollisionProperty::GetCollisionGroup() const
{
    return m_pOuter->GetCollisionGroup();
}

bool ICollisionProperty::ShouldTouchTrigger( int triggerSolidFlags ) const
{
	if ( GetCollisionGroup() == COLLISION_GROUP_DEBRIS )
	{
		if ( triggerSolidFlags & FSOLID_TRIGGER_TOUCH_DEBRIS )
			return true;

		return false;
	}

	if ( IsSolidFlagSet( FSOLID_TRIGGER ) )
		return false;

	return true;
}

const matrix3x4_t *ICollisionProperty::GetRootParentToWorldTransform() const
{
	if ( IsSolidFlagSet( FSOLID_ROOT_PARENT_ALIGNED ) )
	{
		IBaseEntity *pEntity = m_pOuter->GetRootMoveParent();
		if ( pEntity )
		{
			return &pEntity->CollisionProp()->CollisionToWorldTransform();
		}
	}
	return NULL;
}

IClientUnknown* ICollisionProperty::GetIClientUnknown()
{
	return NULL;
}

void ICollisionProperty::CheckForUntouch()
{
	if ( !IsSolid() && !IsSolidFlagSet(FSOLID_TRIGGER))
	{
		if ( m_pOuter->IsCurrentlyTouching() )
		{
			m_pOuter->SetCheckUntouch( true );
		}
	}
}

static void GetAllChildren_r( IBaseEntity *pEntity, CUtlVector<IBaseEntity *> &list )
{
	for ( ; pEntity != NULL; pEntity = pEntity->NextMovePeer() )
	{
		list.AddToTail( pEntity );
		GetAllChildren_r( pEntity->FirstMoveChild(), list );
	}
}

int GetAllChildren( IBaseEntity *pParent, CUtlVector<IBaseEntity *> &list )
{
	if ( !pParent )
		return 0;

	GetAllChildren_r( pParent->FirstMoveChild(), list );
	return list.Count();
}

void ICollisionProperty::SetSolid( SolidType_t val )
{
	if ( m_nSolidType == val )
		return;

	bool bWasNotSolid = IsSolid();

	MarkSurroundingBoundsDirty();

	if ( val == SOLID_BSP )
	{
		if ( GetOuter()->GetMoveParent() )
		{
			if ( GetOuter()->GetRootMoveParent()->GetSolid() != SOLID_BSP )
			{
				val = SOLID_VPHYSICS;
			}
		}

		if ( !GetOuter()->GetMoveParent() )
		{
			CUtlVector<IBaseEntity *> list;
			GetAllChildren( GetOuter(), list );
			for ( int i = list.Count()-1; i>=0; --i )
			{
				list[i]->AddSolidFlags( FSOLID_ROOT_PARENT_ALIGNED );
			}
		}
	}

	m_nSolidType = val;

	m_pOuter->CollisionRulesChanged();

	UpdateServerPartitionMask( );

	if ( bWasNotSolid != IsSolid() )
	{
		CheckForUntouch();
	}
}

SolidType_t ICollisionProperty::GetSolid() const
{
	return (SolidType_t)m_nSolidType.Get();
}

void ICollisionProperty::SetSolidFlags( int flags )
{
	int oldFlags = m_usSolidFlags;
	m_usSolidFlags = (unsigned short)(flags & 0xFFFF);
	if ( oldFlags == m_usSolidFlags )
		return;

	if ( (oldFlags & (FSOLID_FORCE_WORLD_ALIGNED | FSOLID_USE_TRIGGER_BOUNDS)) != 
		 (m_usSolidFlags & (FSOLID_FORCE_WORLD_ALIGNED | FSOLID_USE_TRIGGER_BOUNDS)) )
	{
		MarkSurroundingBoundsDirty();
	}

	if ( (oldFlags & (FSOLID_NOT_SOLID|FSOLID_TRIGGER)) != (m_usSolidFlags & (FSOLID_NOT_SOLID|FSOLID_TRIGGER)) )
	{
		m_pOuter->CollisionRulesChanged();
	}

	if ( (oldFlags & (FSOLID_NOT_SOLID | FSOLID_TRIGGER)) != (m_usSolidFlags & (FSOLID_NOT_SOLID | FSOLID_TRIGGER)) )
	{
		UpdateServerPartitionMask( );
		CheckForUntouch();
	}
}

const Vector& ICollisionProperty::GetCollisionOrigin() const
{
	return m_pOuter->GetAbsOrigin();
}

const QAngle& ICollisionProperty::GetCollisionAngles() const
{
	if ( IsBoundsDefinedInEntitySpace() )
	{
		return m_pOuter->GetAbsAngles();
	}

	return vec3_angle;
}

const matrix3x4_t& ICollisionProperty::CollisionToWorldTransform() const
{
	static matrix3x4_t s_matTemp[4];
	static int s_nIndex = 0;

	matrix3x4_t &matResult = s_matTemp[s_nIndex];
	s_nIndex = (s_nIndex+1) & 0x3;

	if ( IsBoundsDefinedInEntitySpace() )
	{
		return m_pOuter->EntityToWorldTransform();
	}

	SetIdentityMatrix( matResult );
	MatrixSetColumn( GetCollisionOrigin(), 3, matResult );
	return matResult;
}

void ICollisionProperty::SetCollisionBounds( const Vector& mins, const Vector &maxs )
{
	if ( (m_vecMins == mins) && (m_vecMaxs == maxs) )
		return;

	m_vecMins = mins;
	m_vecMaxs = maxs;

	Vector vecSize;
	VectorSubtract( maxs, mins, vecSize );
	m_flRadius = vecSize.Length() * 0.5f;

	MarkSurroundingBoundsDirty();
}

float ICollisionProperty::BoundingRadius2D() const
{
	Vector vecSize;
	VectorSubtract( m_vecMaxs.Get(), m_vecMins.Get(), vecSize );

	vecSize.z = 0;	
	return vecSize.Length() * 0.5f;
}

const Vector& ICollisionProperty::OBBMins( ) const
{
	return m_vecMins.Get();
}

const Vector& ICollisionProperty::OBBMaxs( ) const
{
	return m_vecMaxs.Get();
}

void ICollisionProperty::WorldSpaceTriggerBounds( Vector *pVecWorldMins, Vector *pVecWorldMaxs ) const
{
	WorldSpaceAABB( pVecWorldMins, pVecWorldMaxs );
	if ( ( GetSolidFlags() & FSOLID_USE_TRIGGER_BOUNDS ) == 0 )
		return;

	pVecWorldMins->x -= m_triggerBloat;
	pVecWorldMins->y -= m_triggerBloat;

	pVecWorldMaxs->x += m_triggerBloat;
	pVecWorldMaxs->y += m_triggerBloat;
	pVecWorldMaxs->z += (float)m_triggerBloat * 0.5f;
}

void ICollisionProperty::UseTriggerBounds( bool bEnable, float flBloat )
{
	m_triggerBloat = (char )flBloat;
	if ( bEnable )
	{
		AddSolidFlags( FSOLID_USE_TRIGGER_BOUNDS );
		Assert( flBloat > 0.0f );
	}
	else
	{
		RemoveSolidFlags( FSOLID_USE_TRIGGER_BOUNDS );
	}
}

int ICollisionProperty::GetCollisionModelIndex()
{
	return m_pOuter->GetModelIndex();
}

const model_t* ICollisionProperty::GetCollisionModel()
{
	return m_pOuter->GetModel();
}

bool ICollisionProperty::TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	return m_pOuter->TestCollision( ray, fContentsMask, tr );
}

bool ICollisionProperty::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	return m_pOuter->TestHitboxes( ray, fContentsMask, tr );
}

const Vector &ICollisionProperty::NormalizedToCollisionSpace( const Vector &in, Vector *pResult ) const
{
	pResult->x = Lerp( in.x, m_vecMins.Get().x, m_vecMaxs.Get().x );
	pResult->y = Lerp( in.y, m_vecMins.Get().y, m_vecMaxs.Get().y );
	pResult->z = Lerp( in.z, m_vecMins.Get().z, m_vecMaxs.Get().z );
	return *pResult;
}

const Vector &	ICollisionProperty::CollisionToNormalizedSpace( const Vector &in, Vector *pResult ) const
{
	Vector vecSize = OBBSize( );
	pResult->x = ( vecSize.x != 0.0f ) ? ( in.x - m_vecMins.Get().x ) / vecSize.x : 0.5f;
	pResult->y = ( vecSize.y != 0.0f ) ? ( in.y - m_vecMins.Get().y ) / vecSize.y : 0.5f;
	pResult->z = ( vecSize.z != 0.0f ) ? ( in.z - m_vecMins.Get().z ) / vecSize.z : 0.5f;
	return *pResult;
}

const Vector & ICollisionProperty::NormalizedToWorldSpace( const Vector &in, Vector *pResult ) const
{
	Vector vecCollisionSpace;
	NormalizedToCollisionSpace( in, &vecCollisionSpace );
	CollisionToWorldSpace( vecCollisionSpace, pResult );
	return *pResult;
}

const Vector & ICollisionProperty::WorldToNormalizedSpace( const Vector &in, Vector *pResult ) const
{
	Vector vecCollisionSpace;
	WorldToCollisionSpace( in, &vecCollisionSpace );
	CollisionToNormalizedSpace( vecCollisionSpace, pResult );
	return *pResult;
}

void ICollisionProperty::RandomPointInBounds( const Vector &vecNormalizedMins, const Vector &vecNormalizedMaxs, Vector *pPoint) const
{
	Vector vecNormalizedSpace;
	vecNormalizedSpace.x = ::RandomFloat( vecNormalizedMins.x, vecNormalizedMaxs.x );
	vecNormalizedSpace.y = ::RandomFloat( vecNormalizedMins.y, vecNormalizedMaxs.y );
	vecNormalizedSpace.z = ::RandomFloat( vecNormalizedMins.z, vecNormalizedMaxs.z );
	NormalizedToWorldSpace( vecNormalizedSpace, pPoint );
}

void ICollisionProperty::CollisionAABBToWorldAABB( const Vector &entityMins, const Vector &entityMaxs, Vector *pWorldMins, Vector *pWorldMaxs ) const
{
	if ( !IsBoundsDefinedInEntitySpace() || (GetCollisionAngles() == vec3_angle) )
	{
		VectorAdd( entityMins, GetCollisionOrigin(), *pWorldMins );
		VectorAdd( entityMaxs, GetCollisionOrigin(), *pWorldMaxs );
	}
	else
	{
		TransformAABB( CollisionToWorldTransform(), entityMins, entityMaxs, *pWorldMins, *pWorldMaxs );
	}
}

bool ICollisionProperty::IsPointInBounds( const Vector &vecWorldPt ) const
{
	Vector vecLocalSpace;
	WorldToCollisionSpace( vecWorldPt, &vecLocalSpace );
	return ( ( vecLocalSpace.x >= m_vecMins.Get().x && vecLocalSpace.x <= m_vecMaxs.Get().x ) &&
			( vecLocalSpace.y >= m_vecMins.Get().y && vecLocalSpace.y <= m_vecMaxs.Get().y ) &&
			( vecLocalSpace.z >= m_vecMins.Get().z && vecLocalSpace.z <= m_vecMaxs.Get().z ) );
}

void ICollisionProperty::CalcNearestPoint( const Vector &vecWorldPt, Vector *pVecNearestWorldPt ) const
{
	Vector localPt, localClosestPt;
	WorldToCollisionSpace( vecWorldPt, &localPt );
	CalcClosestPointOnAABB( m_vecMins.Get(), m_vecMaxs.Get(), localPt, localClosestPt );
	CollisionToWorldSpace( localClosestPt, pVecNearestWorldPt );
}

float ICollisionProperty::CalcDistanceFromPoint( const Vector &vecWorldPt ) const
{
	Vector localPt, localClosestPt;
	WorldToCollisionSpace( vecWorldPt, &localPt );
	CalcClosestPointOnAABB( m_vecMins.Get(), m_vecMaxs.Get(), localPt, localClosestPt );
	return localPt.DistTo( localClosestPt );
}

float ICollisionProperty::ComputeSupportMap( const Vector &vecDirection ) const
{
	Vector vecCollisionDir;
	WorldDirectionToCollisionSpace( vecDirection, &vecCollisionDir );

	float flResult = DotProduct( GetCollisionOrigin(), vecDirection );
	flResult += (( vecCollisionDir.x >= 0.0f ) ? m_vecMaxs.Get().x : m_vecMins.Get().x) * vecCollisionDir.x;
	flResult += (( vecCollisionDir.y >= 0.0f ) ? m_vecMaxs.Get().y : m_vecMins.Get().y) * vecCollisionDir.y;
	flResult += (( vecCollisionDir.z >= 0.0f ) ? m_vecMaxs.Get().z : m_vecMins.Get().z) * vecCollisionDir.z;

	return flResult;
}

void ICollisionProperty::ComputeVPhysicsSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	bool bSetBounds = false;
	IPhysicsObject *pPhysicsObject = GetOuter()->VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		if ( pPhysicsObject->GetCollide() )
		{
			g_HL2->GetPhysicsCollision()->CollideGetAABB( pVecWorldMins, pVecWorldMaxs, pPhysicsObject->GetCollide(), GetCollisionOrigin(), GetCollisionAngles() );
			bSetBounds = true;
		}
		else if ( pPhysicsObject->GetSphereRadius( ) )
		{
			float flRadius = pPhysicsObject->GetSphereRadius( );
			Vector vecExtents( flRadius, flRadius, flRadius );
			VectorSubtract( GetCollisionOrigin(), vecExtents, *pVecWorldMins );
			VectorAdd( GetCollisionOrigin(), vecExtents, *pVecWorldMaxs );
			bSetBounds = true;
		}
	}

	if ( !bSetBounds )
	{
		*pVecWorldMins = GetCollisionOrigin();
		*pVecWorldMaxs = *pVecWorldMins;
	}

	if ( IsSolidFlagSet( FSOLID_USE_TRIGGER_BOUNDS ) )
	{
		Vector vecWorldTriggerMins, vecWorldTriggerMaxs;
		WorldSpaceTriggerBounds( &vecWorldTriggerMins, &vecWorldTriggerMaxs );
		VectorMin( vecWorldTriggerMins, *pVecWorldMins, *pVecWorldMins );
		VectorMax( vecWorldTriggerMaxs, *pVecWorldMaxs, *pVecWorldMaxs );
	}
}

bool ICollisionProperty::ComputeHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	IBaseAnimating *pAnim = (IBaseAnimating *)GetOuter()->GetBaseAnimating();
	if (pAnim)
	{
		return pAnim->ComputeHitboxSurroundingBox( pVecWorldMins, pVecWorldMaxs );
	}

	return false;
}

bool ICollisionProperty::ComputeEntitySpaceHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	IBaseAnimating *pAnim = (IBaseAnimating *)GetOuter()->GetBaseAnimating();
	if (pAnim)
	{
		return pAnim->ComputeEntitySpaceHitboxSurroundingBox( pVecWorldMins, pVecWorldMaxs );
	}

	return false;
}

#ifndef max
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

void ICollisionProperty::ComputeRotationExpandedBounds( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	if ( !IsBoundsDefinedInEntitySpace() )
	{
		*pVecWorldMins = m_vecMins.Get();
		*pVecWorldMaxs = m_vecMaxs.Get();
	}
	else
	{
		float flMaxVal;
		flMaxVal = max( FloatMakePositive(m_vecMins.Get().x), FloatMakePositive(m_vecMaxs.Get().x) );
		pVecWorldMins->x = -flMaxVal;
		pVecWorldMaxs->x = flMaxVal;

		flMaxVal = max( FloatMakePositive(m_vecMins.Get().y), FloatMakePositive(m_vecMaxs.Get().y) );
		pVecWorldMins->y = -flMaxVal;
		pVecWorldMaxs->y = flMaxVal;

		flMaxVal = max( FloatMakePositive(m_vecMins.Get().z), FloatMakePositive(m_vecMaxs.Get().z) );
		pVecWorldMins->z = -flMaxVal;
		pVecWorldMaxs->z = flMaxVal;
	}
}

void ICollisionProperty::ComputeCollisionSurroundingBox( bool bUseVPhysics, Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	Assert( GetSolid() != SOLID_CUSTOM );

	if ( bUseVPhysics )
	{
		ComputeVPhysicsSurroundingBox( pVecWorldMins, pVecWorldMaxs );
	}
	else
	{
		WorldSpaceTriggerBounds( pVecWorldMins, pVecWorldMaxs );
	}
}

void ICollisionProperty::ComputeSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	if (( GetSolid() == SOLID_CUSTOM ) && (m_nSurroundType != USE_GAME_CODE ))
	{
		*pVecWorldMins = GetCollisionOrigin();
		*pVecWorldMaxs = *pVecWorldMins;
		return;
	}

	switch( m_nSurroundType )
	{
	case USE_OBB_COLLISION_BOUNDS:
		{
			bool bUseVPhysics = false;
			if ( ( GetSolid() == SOLID_VPHYSICS ) && ( GetOuter()->GetMoveType() == MOVETYPE_VPHYSICS ) )
			{
				IPhysicsObject *pPhysics = GetOuter()->VPhysicsGetObject();
				bUseVPhysics = pPhysics && pPhysics->IsAsleep();
			}
			ComputeCollisionSurroundingBox( bUseVPhysics, pVecWorldMins, pVecWorldMaxs );
		}
		break;

	case USE_BEST_COLLISION_BOUNDS:
		ComputeCollisionSurroundingBox( (GetSolid() == SOLID_VPHYSICS), pVecWorldMins, pVecWorldMaxs );
		break;

	case USE_COLLISION_BOUNDS_NEVER_VPHYSICS:
		ComputeCollisionSurroundingBox( false, pVecWorldMins, pVecWorldMaxs );
		break;

	case USE_HITBOXES:
		ComputeHitboxSurroundingBox( pVecWorldMins, pVecWorldMaxs );
		break;

	case USE_ROTATION_EXPANDED_BOUNDS:
		ComputeRotationExpandedBounds( pVecWorldMins, pVecWorldMaxs );
		break;

	case USE_SPECIFIED_BOUNDS:
		VectorAdd( GetCollisionOrigin(), m_vecSpecifiedSurroundingMins, *pVecWorldMins );
		VectorAdd( GetCollisionOrigin(), m_vecSpecifiedSurroundingMaxs, *pVecWorldMaxs );
		break;

	case USE_GAME_CODE:
		GetOuter()->ComputeWorldSpaceSurroundingBox( pVecWorldMins, pVecWorldMaxs );
		return;
	}
}

void ICollisionProperty::SetSurroundingBoundsType( SurroundingBoundsType_t type, const Vector *pMins, const Vector *pMaxs )
{
	m_nSurroundType = type;
	if (type != USE_SPECIFIED_BOUNDS)
	{
		MarkSurroundingBoundsDirty();
	}
	else
	{
		Assert( pMins && pMaxs );
		m_vecSpecifiedSurroundingMins = *pMins;
		m_vecSpecifiedSurroundingMaxs = *pMaxs;
		m_vecSurroundingMins = *pMins;
		m_vecSurroundingMaxs = *pMaxs;
	}
}

void ICollisionProperty::MarkSurroundingBoundsDirty()
{
	GetOuter()->AddEFlags( EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS );
	MarkPartitionHandleDirty();

	GetOuter()->NetworkProp()->MarkPVSInformationDirty();
}

bool ICollisionProperty::DoesVPhysicsInvalidateSurroundingBox( ) const
{
	switch ( m_nSurroundType )
	{
	case USE_BEST_COLLISION_BOUNDS:
		return true;

	case USE_OBB_COLLISION_BOUNDS:
		return (GetSolid() == SOLID_VPHYSICS) && (GetOuter()->GetMoveType() == MOVETYPE_VPHYSICS) && GetOuter()->VPhysicsGetObject();

	case USE_GAME_CODE:
		return true;

	case USE_COLLISION_BOUNDS_NEVER_VPHYSICS:
	case USE_HITBOXES:
	case USE_ROTATION_EXPANDED_BOUNDS:
	case USE_SPECIFIED_BOUNDS:
		return false;

	default:
		return true;
	}
}

void ICollisionProperty::WorldSpaceSurroundingBounds( Vector *pVecMins, Vector *pVecMaxs )
{
	const Vector &vecAbsOrigin = GetCollisionOrigin();
	if ( GetOuter()->IsEFlagsSet( EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS ))
	{
		GetOuter()->RemoveEFlags( EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS );
		ComputeSurroundingBox( pVecMins, pVecMaxs );
		VectorSubtract( *pVecMins, vecAbsOrigin, m_vecSurroundingMins );
		VectorSubtract( *pVecMaxs, vecAbsOrigin, m_vecSurroundingMaxs );
	}
	else
	{
		VectorAdd( m_vecSurroundingMins, vecAbsOrigin, *pVecMins );
		VectorAdd( m_vecSurroundingMaxs, vecAbsOrigin, *pVecMaxs );
	}
}

void ICollisionProperty::CreatePartitionHandle()
{
	m_Partition = g_pPartition->CreateHandle( GetEntityHandle() );
}

void ICollisionProperty::DestroyPartitionHandle()
{
	if ( m_Partition != PARTITION_INVALID_HANDLE )
	{
		g_pPartition->DestroyHandle( m_Partition );
		m_Partition = PARTITION_INVALID_HANDLE;
	}
}

void ICollisionProperty::UpdateServerPartitionMask( )
{
	SpatialPartitionHandle_t handle = GetPartitionHandle();
	if ( handle == PARTITION_INVALID_HANDLE )
		return;

	g_pPartition->Remove( handle );

	if ( !m_pOuter->edict() )
		return;

	if ( m_pOuter->entindex() == 0 )
		return;		

	bool bIsSolid = IsSolid() || IsSolidFlagSet(FSOLID_TRIGGER);
	if ( bIsSolid || m_pOuter->IsEFlagsSet(EFL_USE_PARTITION_WHEN_NOT_SOLID) )
	{
		g_pPartition->Insert( PARTITION_ENGINE_NON_STATIC_EDICTS, handle );
	}

	if ( !bIsSolid )
		return;

	SpatialPartitionListMask_t mask = 0;
	if ( !IsSolidFlagSet(FSOLID_NOT_SOLID) )
	{
		mask |=	PARTITION_ENGINE_SOLID_EDICTS;
	}
	if ( IsSolidFlagSet(FSOLID_TRIGGER) )
	{
		mask |=	PARTITION_ENGINE_TRIGGER_EDICTS;
	}
	Assert( mask != 0 );
	g_pPartition->Insert( mask, handle );
}

void ICollisionProperty::MarkPartitionHandleDirty()
{
	if ( m_pOuter->entindex() == 0 )
	{
		return;
	}

	if ( !m_pOuter->IsEFlagsSet( EFL_DIRTY_SPATIAL_PARTITION ) )
	{
		m_pOuter->AddEFlags( EFL_DIRTY_SPATIAL_PARTITION );

		IDirtySpatialPartitionEntityList *m_DirtyKDTree = (IDirtySpatialPartitionEntityList *)g_HL2->GetDirtyKDTree();
		m_DirtyKDTree->AddEntity( m_pOuter );
	}
}

void ICollisionProperty::UpdatePartition( )
{
	if ( m_pOuter->IsEFlagsSet( EFL_DIRTY_SPATIAL_PARTITION ) )
	{
		m_pOuter->RemoveEFlags( EFL_DIRTY_SPATIAL_PARTITION );

		Assert( m_pOuter->entindex() != 0 );

		if ( !m_pOuter->edict() )
			return;

		if ( GetPartitionHandle() == PARTITION_INVALID_HANDLE )
		{
			CreatePartitionHandle();
			UpdateServerPartitionMask();
		}

		if ( IsSolid() || IsSolidFlagSet( FSOLID_TRIGGER ) || m_pOuter->IsEFlagsSet( EFL_USE_PARTITION_WHEN_NOT_SOLID ) )
		{
			if ( BoundingRadius() != 0.0f )
			{
				Vector vecSurroundMins, vecSurroundMaxs;
				WorldSpaceSurroundingBounds( &vecSurroundMins, &vecSurroundMaxs );
				vecSurroundMins -= Vector( 1, 1, 1 );
				vecSurroundMaxs += Vector( 1, 1, 1 );
				g_pPartition->ElementMoved( GetPartitionHandle(), vecSurroundMins,  vecSurroundMaxs );
			}
			else
			{
				g_pPartition->ElementMoved( GetPartitionHandle(), GetCollisionOrigin(),  GetCollisionOrigin() );
			}
		}
	}
}

IDirtySpatialPartitionEntityList::IDirtySpatialPartitionEntityList(char const *name)
{
    m_DirtyEntities.Purge();
	m_readLockCount = 0;
}

bool IDirtySpatialPartitionEntityList::Init()
{
    g_pPartition->InstallQueryCallback(this);
    return true;
}

void IDirtySpatialPartitionEntityList::Shutdown()
{
    g_pPartition->RemoveQueryCallback(this);
}

void IDirtySpatialPartitionEntityList::LevelShutdownPostEntity()
{
    m_DirtyEntities.RemoveAll();
}

void IDirtySpatialPartitionEntityList::OnPreQuery(SpatialPartitionListMask_t listMask)
{
	const int validMask = PARTITION_SERVER_GAME_EDICTS;

	if ( !( listMask & validMask ) )
		return;

	if ( m_partitionWriteId != 0 && m_partitionWriteId == ThreadGetCurrentId() )
		return;

	if ( m_DirtyEntities.Count() && !m_readLockCount )
	{
		CUtlVector< CBaseHandle > vecStillDirty;
		m_partitionMutex.LockForWrite();
		m_partitionWriteId = ThreadGetCurrentId();
		CTSListWithFreeList<CBaseHandle>::Node_t *pCurrent, *pNext;
		while ( ( pCurrent = m_DirtyEntities.Detach() ) != NULL )
		{
			while ( pCurrent )
			{
				CBaseHandle handle = pCurrent->elem;
				pNext = (CTSListWithFreeList<CBaseHandle>::Node_t *)pCurrent->Next;
				m_DirtyEntities.FreeNode( pCurrent );
				pCurrent = pNext;

				IBaseEntity *pEntity = (IBaseEntity *)g_HL2->GetBaseEntity(handle);

				if ( pEntity )
				{
					if ( !pEntity->IsEFlagsSet( EFL_SETTING_UP_BONES ) )
					{
						pEntity->CollisionProp()->UpdatePartition();
					}
					else
					{
						vecStillDirty.AddToTail( handle );
					}
				}
			}
		}
		if ( vecStillDirty.Count() > 0 )
		{
			for ( int i = 0; i < vecStillDirty.Count(); i++ )
			{
				m_DirtyEntities.PushItem( vecStillDirty[i] );
			}
		}
		m_partitionWriteId = 0;
		m_partitionMutex.UnlockWrite();
	}
	LockPartitionForRead();
}

void IDirtySpatialPartitionEntityList::OnPostQuery(SpatialPartitionListMask_t listMask)
{
	if ( !( listMask & PARTITION_SERVER_GAME_EDICTS ) )
		return;

	if ( m_partitionWriteId != 0 )
		return;

	UnlockPartitionForRead();
}

void IDirtySpatialPartitionEntityList::AddEntity(IBaseEntity *pEntity)
{
	m_DirtyEntities.PushItem( pEntity->GetRefEHandle() );
}

IDirtySpatialPartitionEntityList::~IDirtySpatialPartitionEntityList()
{
    m_DirtyEntities.Purge();
}
