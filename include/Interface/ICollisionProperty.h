#ifndef _HEADER_COLLISION_PROPERTY_INLUDE_
#define _HEADER_COLLISION_PROPERTY_INLUDE_

#include "HL2.h"
#include <networkvar.h>
#include <engine/ICollideable.h>
#include <mathlib/vector.h>
#include <ispatialpartition.h>

class IBaseEntity;

enum SurroundingBoundsType_t
{
	USE_OBB_COLLISION_BOUNDS = 0,
	USE_BEST_COLLISION_BOUNDS,
	USE_HITBOXES,
	USE_SPECIFIED_BOUNDS,
	USE_GAME_CODE,
	USE_ROTATION_EXPANDED_BOUNDS,
	USE_COLLISION_BOUNDS_NEVER_VPHYSICS,

	SURROUNDING_TYPE_BIT_COUNT = 3
};

class ICollisionProperty : public ICollideable
{
public:
	typedef ICollisionProperty ThisClass;

	virtual void 				NetworkStateChanged() {}
	virtual void 				NetworkStateChanged( void *pProp ) {}
	virtual datamap_t 			*GetDataDescMap( void ) { return NULL; };

public:
	ICollisionProperty();
	~ICollisionProperty();

	void Init( IBaseEntity *pEntity );

	// Methods of ICollideable
	virtual IHandleEntity		*GetEntityHandle();
 	virtual const Vector&		OBBMins( ) const;
	virtual const Vector&		OBBMaxs( ) const;
	virtual void				WorldSpaceTriggerBounds( Vector *pVecWorldMins, Vector *pVecWorldMaxs ) const;
	virtual bool				TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	virtual bool				TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	virtual int					GetCollisionModelIndex();
	virtual const model_t*		GetCollisionModel();
	virtual const Vector&		GetCollisionOrigin() const;
	virtual const QAngle&		GetCollisionAngles() const;
	virtual const matrix3x4_t&	CollisionToWorldTransform() const;
	virtual SolidType_t			GetSolid() const;
	virtual int					GetSolidFlags() const;
	virtual IClientUnknown*		GetIClientUnknown();
	virtual int					GetCollisionGroup() const;
	virtual void				WorldSpaceSurroundingBounds( Vector *pVecMins, Vector *pVecMaxs );
	virtual bool				ShouldTouchTrigger( int triggerSolidFlags ) const;
	virtual const matrix3x4_t 	*GetRootParentToWorldTransform() const;

public:
	void						CreatePartitionHandle();
	void						DestroyPartitionHandle();
	SpatialPartitionHandle_t	GetPartitionHandle() const;
	void						MarkPartitionHandleDirty();
	void						SetCollisionBounds( const Vector& mins, const Vector &maxs );
	void						UseTriggerBounds( bool bEnable, float flBloat = 0.0f );
	void						SetSurroundingBoundsType( SurroundingBoundsType_t type, const Vector *pMins = NULL, const Vector *pMaxs = NULL );
	void						SetSolid( SolidType_t val );
	const Vector&				OBBSize( ) const;
	float						BoundingRadius() const;
	float						BoundingRadius2D() const;
	const Vector&				OBBCenter( ) const;
	const Vector&				WorldSpaceCenter( ) const;
	void						ClearSolidFlags( void );	
	void						RemoveSolidFlags( int flags );
	void						AddSolidFlags( int flags );
	bool						IsSolidFlagSet( int flagMask ) const;
	void		 				SetSolidFlags( int flags );
	bool						IsSolid() const;
	void						UpdatePartition( );
	bool						IsBoundsDefinedInEntitySpace() const;
	const Vector&				CollisionToWorldSpace( const Vector &in, Vector *pResult ) const;
	const Vector&				WorldToCollisionSpace( const Vector &in, Vector *pResult ) const;
	const Vector&				WorldDirectionToCollisionSpace( const Vector &in, Vector *pResult ) const;
	void						RandomPointInBounds( const Vector &vecNormalizedMins, const Vector &vecNormalizedMaxs, Vector *pPoint) const;
	bool						IsPointInBounds( const Vector &vecWorldPt ) const;
	void						WorldSpaceAABB( Vector *pWorldMins, Vector *pWorldMaxs ) const;
	const Vector&				NormalizedToCollisionSpace( const Vector &in, Vector *pResult ) const;
	const Vector&				NormalizedToWorldSpace( const Vector &in, Vector *pResult ) const;
	const Vector&				WorldToNormalizedSpace( const Vector &in, Vector *pResult ) const;
	const Vector&				CollisionToNormalizedSpace( const Vector &in, Vector *pResult ) const;
	void						CalcNearestPoint( const Vector &vecWorldPt, Vector *pVecNearestWorldPt ) const;
	float						CalcDistanceFromPoint( const Vector &vecWorldPt ) const;
	bool						DoesRotationInvalidateSurroundingBox( ) const;
	bool						DoesVPhysicsInvalidateSurroundingBox( ) const;
	void						MarkSurroundingBoundsDirty();
	float						ComputeSupportMap( const Vector &vecDirection ) const;

private:
	void CollisionAABBToWorldAABB( const Vector &entityMins, const Vector &entityMaxs, Vector *pWorldMins, Vector *pWorldMaxs ) const;
	void ComputeVPhysicsSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	bool ComputeHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	bool ComputeEntitySpaceHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	void ComputeCollisionSurroundingBox( bool bUseVPhysics, Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	void ComputeRotationExpandedBounds( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	void ComputeSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	void CheckForUntouch();
	void UpdateServerPartitionMask( );
    
	IBaseEntity *GetOuter();
	const IBaseEntity *GetOuter() const;

private:
	IBaseEntity					*m_pOuter;						// byte 4
	CNetworkVector(m_vecMins);									// byte 8
	CNetworkVector(m_vecMaxs);									// byte 20
	CNetworkVar(unsigned short, m_usSolidFlags);				// byte 32
	CNetworkVar(unsigned char,  m_nSolidType);					// byte 34
	CNetworkVar(unsigned char,  m_triggerBloat);				// byte 35
	float						m_flRadius;						// byte 36
	SpatialPartitionHandle_t	m_Partition;					// byte 40
	CNetworkVar(unsigned char,	m_nSurroundType);				// byte 42
	CNetworkVector(m_vecSpecifiedSurroundingMins);				// byte 44
	CNetworkVector(m_vecSpecifiedSurroundingMaxs);				// byte 56
	Vector						m_vecSurroundingMins;			// byte 68
	Vector						m_vecSurroundingMaxs;			// byte 80
};

inline IBaseEntity *ICollisionProperty::GetOuter()
{
	return m_pOuter;
}

inline const IBaseEntity *ICollisionProperty::GetOuter() const
{
	return m_pOuter;
}

inline SpatialPartitionHandle_t ICollisionProperty::GetPartitionHandle() const
{
	return m_Partition;
}

inline const Vector& ICollisionProperty::OBBSize( ) const
{
	Vector &temp = AllocTempVector();
	VectorSubtract( m_vecMaxs, m_vecMins, temp );
	return temp;
}

inline float ICollisionProperty::BoundingRadius() const
{
	return m_flRadius;
}

inline bool ICollisionProperty::IsBoundsDefinedInEntitySpace() const
{
	return (( m_usSolidFlags & FSOLID_FORCE_WORLD_ALIGNED ) == 0 ) && ( m_nSolidType != SOLID_BBOX ) && ( m_nSolidType != SOLID_NONE );
}

inline void ICollisionProperty::ClearSolidFlags( void )
{
	SetSolidFlags( 0 );
}

inline void ICollisionProperty::RemoveSolidFlags( int flags )
{
	SetSolidFlags( m_usSolidFlags & ~flags );
}

inline void ICollisionProperty::AddSolidFlags( int flags )
{
	SetSolidFlags( m_usSolidFlags | flags );
}

inline int ICollisionProperty::GetSolidFlags( void ) const
{
	Msg("[ICollisionProperty::GetSolidFlags]\n");
	return m_usSolidFlags;
}

inline bool ICollisionProperty::IsSolidFlagSet( int flagMask ) const
{
	return (m_usSolidFlags & flagMask) != 0;
}

inline bool ICollisionProperty::IsSolid() const
{
	return ::IsSolid( (SolidType_t)(unsigned char)m_nSolidType, m_usSolidFlags );
}

inline const Vector& ICollisionProperty::OBBCenter( ) const
{
	Vector &vecResult = AllocTempVector();
	VectorLerp( m_vecMins, m_vecMaxs, 0.5f, vecResult );
	return vecResult;
}

inline const Vector &ICollisionProperty::WorldSpaceCenter( ) const 
{
	Vector &vecResult = AllocTempVector();
	CollisionToWorldSpace( OBBCenter(), &vecResult );
	return vecResult;
}

inline const Vector &ICollisionProperty::CollisionToWorldSpace( const Vector &in, Vector *pResult ) const 
{
	if ( !IsBoundsDefinedInEntitySpace() || ( GetCollisionAngles() == vec3_angle ) )
	{
		VectorAdd( in, GetCollisionOrigin(), *pResult );
	}
	else
	{
		VectorTransform( in, CollisionToWorldTransform(), *pResult );
	}
	return *pResult;
}

inline const Vector &ICollisionProperty::WorldToCollisionSpace( const Vector &in, Vector *pResult ) const
{
	if ( !IsBoundsDefinedInEntitySpace() || ( GetCollisionAngles() == vec3_angle ) )
	{
		VectorSubtract( in, GetCollisionOrigin(), *pResult );
	}
	else
	{
		VectorITransform( in, CollisionToWorldTransform(), *pResult );
	}
	return *pResult;
}

inline const Vector & ICollisionProperty::WorldDirectionToCollisionSpace( const Vector &in, Vector *pResult ) const
{
	if ( !IsBoundsDefinedInEntitySpace() || ( GetCollisionAngles() == vec3_angle ) )
	{
		*pResult = in;
	}
	else
	{
		VectorIRotate( in, CollisionToWorldTransform(), *pResult );
	}
	return *pResult;
}

inline void ICollisionProperty::WorldSpaceAABB( Vector *pWorldMins, Vector *pWorldMaxs ) const
{
	CollisionAABBToWorldAABB( m_vecMins, m_vecMaxs, pWorldMins, pWorldMaxs );
}

inline bool ICollisionProperty::DoesRotationInvalidateSurroundingBox( ) const
{
	if ( IsSolidFlagSet(FSOLID_ROOT_PARENT_ALIGNED) )
		return true;

	switch ( m_nSurroundType )
	{
	case USE_COLLISION_BOUNDS_NEVER_VPHYSICS:
	case USE_OBB_COLLISION_BOUNDS:
	case USE_BEST_COLLISION_BOUNDS:
		return IsBoundsDefinedInEntitySpace();

	case USE_HITBOXES:
	case USE_GAME_CODE:
		return true;

	case USE_ROTATION_EXPANDED_BOUNDS:
	case USE_SPECIFIED_BOUNDS:
		return false;

	default:
		Assert(0);
		return true;
	}
}


#endif