#include "extension.h"
#include "INavMesh.h"
#include "CTrace.h"

enum BrushSolidities_e
{
    BRUSHSOLID_TOGGLE = 0,
    BRUSHSOLID_NEVER  = 1,
    BRUSHSOLID_ALWAYS = 2,
};


#define WALK_THRU_PROP_DOORS		0x01
#define WALK_THRU_FUNC_DOORS		0x02
#define WALK_THRU_DOORS				(WALK_THRU_PROP_DOORS | WALK_THRU_FUNC_DOORS)
#define WALK_THRU_BREAKABLES		0x04
#define WALK_THRU_TOGGLE_BRUSHES	0x08
#define WALK_THRU_EVERYTHING		(WALK_THRU_DOORS | WALK_THRU_BREAKABLES | WALK_THRU_TOGGLE_BRUSHES)
inline bool IsEntityWalkable( IBaseEntity *entity, unsigned int flags )
{
	ConVarRef nav_solid_props("nav_solid_props");
	if (entity->ClassMatches("worldspawn" ))
		return false;

	if (entity->ClassMatches("player" ))
		return false;

	// if we hit a door, assume its walkable because it will open when we touch it
	if (entity->ClassMatches("func_door*" ))
	{
		return (flags & WALK_THRU_FUNC_DOORS) ? true : false;
	}

	if (entity->ClassMatches("prop_door*" ))
	{
		return (flags & WALK_THRU_PROP_DOORS) ? true : false;
	}

	// if we hit a clip brush, ignore it if it is not BRUSHSOLID_ALWAYS
	if (entity->ClassMatches("func_brush" ))
	{
		switch ( access_member<BrushSolidities_e>(engine, 223*4) )
		{
		case BRUSHSOLID_ALWAYS:
			return false;
		case BRUSHSOLID_NEVER:
			return true;
		case BRUSHSOLID_TOGGLE:
			return (flags & WALK_THRU_TOGGLE_BRUSHES) ? true : false;
		}
	}

	// if we hit a breakable object, assume its walkable because we will shoot it when we touch it
	if (entity->ClassMatches("func_breakable" ) && entity->GetHealth() && entity->GetTakedamage() == DAMAGE_YES)
		return (flags & WALK_THRU_BREAKABLES) ? true : false;

	if (entity->ClassMatches("func_breakable_surf" ) && entity->GetTakedamage() == DAMAGE_YES)
		return (flags & WALK_THRU_BREAKABLES) ? true : false;

	if ( entity->ClassMatches("func_playerinfected_clip" ) == true )
		return true;

	if ( nav_solid_props.GetBool() && entity->ClassMatches("prop_*" ) )
		return true;

	return false;
}

class CTraceFilterNoNPCsOrPlayer : public CTraceFilterSimples
{
public:
	CTraceFilterNoNPCsOrPlayer( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimples( (IHandleEntity*)passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
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
private:
	bool IsStaticProp( CBaseHandle handle )
    {
        return (handle.GetSerialNumber() == (0x40000000 >> NUM_ENT_ENTRY_BITS));
    }

    IBaseEntity *GetEntityFromEntityHandle( IHandleEntity *pHandleEntity )
    {
        IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
		if(pUnk)
		{
			IBaseEntity *pEnt = reinterpret_cast<IBaseEntity*>(pUnk->GetBaseEntity());
			if (!IsStaticProp(pEnt->GetRefEHandle()))
			{
				return pEnt;
			}
		}
        return nullptr;
    }

};


class CTraceFilterWalkableEntities : public CTraceFilterNoNPCsOrPlayer
{
public:
	CTraceFilterWalkableEntities( const IHandleEntity *passentity, int collisionGroup, unsigned int flags )
		: CTraceFilterNoNPCsOrPlayer( passentity, collisionGroup ), m_flags( flags )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		if ( CTraceFilterNoNPCsOrPlayer::ShouldHitEntity(pServerEntity, contentsMask) )
		{
			IBaseEntity *pEntity = GetEntityFromEntityHandle( pServerEntity );
			return ( !IsEntityWalkable( pEntity, m_flags ) );
		}
		return false;
	}

private:
	bool IsStaticProp( CBaseHandle handle )
    {
        return (handle.GetSerialNumber() == (0x40000000 >> NUM_ENT_ENTRY_BITS));
    }

    IBaseEntity *GetEntityFromEntityHandle( IHandleEntity *pHandleEntity )
    {
        IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
		if(pUnk)
		{
			IBaseEntity *pEnt = reinterpret_cast<IBaseEntity*>(pUnk->GetBaseEntity());
			if (!IsStaticProp(pEnt->GetRefEHandle()))
			{
				return pEnt;
			}
		}
        return nullptr;
    }
	unsigned int m_flags;
};


class CTraceFilterGroundEntities : public CTraceFilterWalkableEntities
{
	typedef CTraceFilterWalkableEntities BaseClass;

public:
	CTraceFilterGroundEntities( const IHandleEntity *passentity, int collisionGroup, unsigned int flags )
		: BaseClass( passentity, collisionGroup, flags )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		IBaseEntity *pEntity = GetEntityFromEntityHandle( pServerEntity );
		if (pEntity->ClassMatches("prop_door" ) || 
			pEntity->ClassMatches("prop_door_rotating" ) || 
			pEntity->ClassMatches("func_breakable" ) )
		{
			return false;
		}

		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}
private:
	bool IsStaticProp( CBaseHandle handle )
    {
        return (handle.GetSerialNumber() == (0x40000000 >> NUM_ENT_ENTRY_BITS));
    }

    IBaseEntity *GetEntityFromEntityHandle( IHandleEntity *pHandleEntity )
    {
        IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
		if(pUnk)
		{
			IBaseEntity *pEnt = reinterpret_cast<IBaseEntity*>(pUnk->GetBaseEntity());
			if (!IsStaticProp(pEnt->GetRefEHandle()))
			{
				return pEnt;
			}
		}
        return nullptr;
    }
};

INavArea *INavMesh::GetNearestNavArea(const Vector &pos, bool anyZ, float maxDist, bool checkLOS, bool checkGround, int team) const
{
	if ( !m_grid.Count() )
	{
		return NULL;	
	}

	INavArea *close = NULL;
	float closeDistSq = maxDist * maxDist;

	if ( !checkLOS && !checkGround )
	{
		close = GetNavArea( pos );
		if ( close )
		{
			return close;
		}
	}

	Vector source;
	source.x = pos.x;
	source.y = pos.y;
	if ( GetGroundHeight( pos, &source.z ) == false )
	{
		if ( !checkGround )
		{
			source.z = pos.z;
		}
		else
		{
			return NULL;
		}
	}

	source.z += HalfHumanHeight;

	unsigned int *searchMarker = g_HL2->GetNavMesh_GetNerestNavArea_SearchMarker();

	++(*searchMarker);
	if ( (*searchMarker) == 0 ) {
		++(*searchMarker);
	}

	int originX = WorldToGridX( pos.x );
	int originY = WorldToGridY( pos.y );

	int shiftLimit = ceil(maxDist / m_gridCellSize);

	for( int shift=0; shift <= shiftLimit; ++shift )
	{
		for( int x = originX - shift; x <= originX + shift; ++x )
		{
			if ( x < 0 || x >= m_gridSizeX )
				continue;

			for( int y = originY - shift; y <= originY + shift; ++y )
			{
				if ( y < 0 || y >= m_gridSizeY )
					continue;

				if ( x > originX - shift &&
					 x < originX + shift &&
					 y > originY - shift &&
					 y < originY + shift )
					continue;

				CUtlVector< INavArea * > *areaVector = &m_grid[ x + y*m_gridSizeX ];

				for ( int it = 0; ((*areaVector)).IsUtlVector && it < ((*areaVector)).Count(); it++ )
				{
					INavArea *area = (INavArea*)(*areaVector)[ it ];

					if ( area->GetNavSearchMarker() == (*searchMarker) )
						continue;

					if ( area->IsBlocked( team ) )
						continue;

					area->SetNavSearchMarker((*searchMarker));

					Vector areaPos;
					area->GetClosestPointOnArea( source, &areaPos );

					float distSq = ( areaPos - pos ).LengthSqr();

					if ( distSq >= closeDistSq )
						continue;

					if ( checkLOS )
					{
						trace_t result;

						Vector safePos;

						util_TraceLine( pos, pos + Vector( 0, 0, StepHeight ), 147979, NULL, COLLISION_GROUP_NONE, &result );
						if ( result.startsolid )
						{
							safePos = result.endpos + Vector( 0, 0, 1.0f );
						}
						else
						{
							safePos = pos;
						}

						float heightDelta = fabs(areaPos.z - safePos.z);
						if ( heightDelta > StepHeight )
						{
							util_TraceLine( areaPos + Vector( 0, 0, StepHeight ), Vector( areaPos.x, areaPos.y, safePos.z ), 147979, NULL, COLLISION_GROUP_NONE, &result );
							
							if ( result.fraction != 1.0f )
							{
								continue;
							}
						}

						util_TraceLine( safePos, Vector( areaPos.x, areaPos.y, safePos.z + StepHeight ), 147979, NULL, COLLISION_GROUP_NONE, &result );

						if ( result.fraction != 1.0f )
						{
							continue;
						}
					}

					closeDistSq = distSq;
					close = area;

					shiftLimit = shift+1;
				}
			}
		}
	}

	return close;
}

INavArea *INavMesh::GetNearestNavArea(IBaseEntity *pEntity, int nFlags, float maxDist) const
{
	if ( !m_grid.Count() )
		return NULL;

	INavArea *pClose = GetNavArea( pEntity, nFlags );
	if ( pClose )
		return pClose;

	bool bCheckLOS = ( nFlags & GETNAVAREA_CHECK_LOS ) != 0;
	bool bCheckGround = ( nFlags & GETNAVAREA_CHECK_GROUND ) != 0;
	return GetNearestNavArea( pEntity->GetAbsOrigin(), false, maxDist, bCheckLOS, bCheckGround );
}

INavArea *INavMesh::GetNavArea( const Vector &pos, float beneathLimit ) const
{
	if ( !m_grid.Count() )
	{
		return NULL;
	}

	int x = WorldToGridX( pos.x );
	int y = WorldToGridY( pos.y );
	CUtlVector< INavArea * > *areaVector = &m_grid[ x + y*m_gridSizeX ];

	INavArea *use = NULL;
	float useZ = -99999999.9f;
	Vector testPos = pos + Vector( 0, 0, 5 );

	for ( int it = 0; ((*areaVector)).IsUtlVector && it < ((*areaVector)).Count(); it++ )
	{
		INavArea *area = (INavArea*)(*areaVector)[ it ];

		if (area->IsOverlapping( testPos ))
		{
			float z = area->GetZ( testPos );

			if (z > testPos.z)
				continue;

			if (z < pos.z - beneathLimit)
				continue;

			if (z > useZ)
			{
				use = area;
				useZ = z;
			}
		}
	}

	return use;
}

INavArea *INavMesh::GetNavArea(IBaseEntity *pEntity, int nFlags, float flBeneathLimit ) const
{
	if ( !m_grid.Count() )
		return NULL;

	Vector testPos = pEntity->GetAbsOrigin();

	float flStepHeight = 1e-3;
	IBaseCombatCharacter *pBCC = (IBaseCombatCharacter *)pEntity->MyCombatCharacterPointer();
	if ( pBCC )
	{
		INavArea *pLastNavArea = (INavArea *)pBCC->GetLastKnownArea();
		if ( pLastNavArea && pLastNavArea->IsOverlapping( testPos ) )
		{
			float flZ = pLastNavArea->GetZ( testPos );
			if ( ( flZ <= testPos.z + StepHeight ) && ( flZ >= testPos.z - StepHeight ) )
				return pLastNavArea;
		}
		flStepHeight = StepHeight;
	}

	int x = WorldToGridX( testPos.x );
	int y = WorldToGridY( testPos.y );
	CUtlVector< INavArea * > *areaVector = &m_grid[ x + y*m_gridSizeX ];

	INavArea *use = NULL;
	float useZ = -99999999.9f;

	bool bSkipBlockedAreas = ( ( nFlags & GETNAVAREA_ALLOW_BLOCKED_AREAS ) == 0 );
	for ( int it = 0; ((*areaVector)).IsUtlVector && it < ((*areaVector)).Count(); it++ )
	{
		INavArea *pArea = (*areaVector)[ it ];

		if ( !pArea->IsOverlapping( testPos ) )
			continue;

		if ( bSkipBlockedAreas && pArea->IsBlocked( -1 ) )
			continue;

		float z = pArea->GetZ( testPos );

		if ( z > testPos.z + flStepHeight )
			continue;

		if ( z < testPos.z - flBeneathLimit )
			continue;

		if ( z <= useZ )
			continue;

		use = pArea;
		useZ = z;
	}

	if ( use && ( nFlags && GETNAVAREA_CHECK_LOS ) && ( useZ < testPos.z - flStepHeight ) )
	{
		trace_t result;
		util_TraceLine( testPos, Vector( testPos.x, testPos.y, useZ ), MASK_NPCSOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &result );
		if ( ( result.fraction != 1.0f ) && ( fabs( result.endpos.z - useZ ) > flStepHeight ) )
			return NULL;
	}
	return use;
}

INavArea *INavMesh::GetNavAreaByID(unsigned int id) const
{
	if(id == 0)
		return nullptr;

	int key = ComputeHashKey(id);
	for(INavArea* area = m_hashTable[key]; area; area = area->GetNextHash())
	{
		if(area->GetID() == id)
		{
			return area;
		}
	}

    return nullptr;
}

bool INavMesh::GetGroundHeight(const Vector &pos, float *height, Vector *normal) const
{
	const float flMaxOffset = 100.0f;

	CTraceFilterGroundEntities filter( NULL, COLLISION_GROUP_NONE, WALK_THRU_EVERYTHING );

	trace_t result;
	Vector to( pos.x, pos.y, pos.z - 10000.0f );
	Vector from( pos.x, pos.y, pos.z + HalfHumanHeight + 1e-3 );
	while( to.z - pos.z < flMaxOffset ) 
	{
		util_TraceLine( from, to, MASK_NPCSOLID_BRUSHONLY, &filter, &result );
		if ( !result.startsolid && (( result.fraction == 1.0f ) || ( ( from.z - result.endpos.z ) >= HalfHumanHeight ) ) )
		{
			*height = result.endpos.z;
			if ( normal )
			{
				*normal = !result.plane.normal.IsZero() ? result.plane.normal : Vector( 0, 0, 1 );
			}
			return true;
		}	  
		to.z = ( result.startsolid ) ? from.z : result.endpos.z;
		from.z = to.z + HalfHumanHeight + 1e-3;
	}

	*height = 0.0f;
	if ( normal )
	{
		normal->Init( 0.0f, 0.0f, 1.0f );
	}
	return false;
}

int INavMesh::WorldToGridX( float wx ) const
{
	int x = (int)( (wx - m_minX) / m_gridCellSize );

	if (x < 0)
		x = 0;
	else if (x >= m_gridSizeX)
		x = m_gridSizeX-1;
	
	return x;
}

//--------------------------------------------------------------------------------------------------------------
int INavMesh::WorldToGridY( float wy ) const
{ 
	int y = (int)( (wy - m_minY) / m_gridCellSize );

	if (y < 0)
		y = 0;
	else if (y >= m_gridSizeY)
		y = m_gridSizeY-1;
	
	return y;
}
