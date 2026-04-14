#include "INavArea.h"
#include "IBasePlayer.h"
#include "INextBotMeneger.h"
#include "INavMesh.h"
#include "tier1/checksum_crc.h"

// unsigned int INavArea::m_masterMarker = 1;

INavArea *INavArea::m_openList = NULL;
INavArea *INavArea::m_openListTail = NULL;


/*
class AreaDestroyNotification
{
	INavArea *m_area;

public:
	AreaDestroyNotification( INavArea *area )
	{
		m_area = area;
	}

	bool operator()( CNavLadder *ladder )
	{
		ladder->OnDestroyNotify( m_area );
		return true;
	}

	bool operator()( INavArea *area )
	{
		if ( area != m_area )
		{
			area->OnDestroyNotify( (CNavArea*)m_area );
		}
		return true;
	}
};

class ForgetArea
{
public:
	ForgetArea( INavArea *area )
	{
		m_area = (CNavArea*)area;
	}
	
	bool operator() ( IBasePlayer *player )
	{
		player->OnNavAreaRemoved( m_area );
		
		return true;
	}

	bool operator() ( IBaseCombatCharacter *player )
	{
		player->OnNavAreaRemoved( m_area );

		return true;
	}

	CNavArea *m_area;
};

static Vector FindPositionInArea( INavArea *area, NavCornerType corner )
{
	int multX = 1, multY = 1;
	switch ( corner )
	{
	case NORTH_WEST:
		break;
	case NORTH_EAST:
		multX = -1;
		break;
	case SOUTH_WEST:
		multY = -1;
		break;
	case SOUTH_EAST:
		multX = -1;
		multY = -1;
		break;
	}

	const float offset = 12.5f;
	Vector cornerPos = area->GetCorner( corner );

	// Try the basic inset
	Vector pos = cornerPos + Vector(  offset*multX,  offset*multY, 0.0f );
	if ( !area->IsOverlapping( pos ) )
	{
		// Try pulling the Y offset to the area's center
		pos = cornerPos + Vector(  offset*multX,  area->GetSizeY()*0.5f*multY, 0.0f );
		if ( !area->IsOverlapping( pos ) )
		{
			// Try pulling the X offset to the area's center
			pos = cornerPos + Vector(  area->GetSizeX()*0.5f*multX,  offset*multY, 0.0f );
			if ( !area->IsOverlapping( pos ) )
			{
				// Try pulling the X and Y offsets to the area's center
				pos = cornerPos + Vector(  area->GetSizeX()*0.5f*multX,  area->GetSizeY()*0.5f*multY, 0.0f );
				if ( !area->IsOverlapping( pos ) )
				{
					// Just pull the position to a small offset
					pos = cornerPos + Vector(  1.0f*multX,  1.0f*multY, 0.0f );
					if ( !area->IsOverlapping( pos ) )
					{
						// Nothing is working (degenerate area?), so just put it directly on the corner
						pos = cornerPos;
					}
				}
			}
		}
	}

	return pos;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(value, minVal, maxVal) (MIN(MAX(value, minVal), maxVal))

int GetTerrainAmbientLightAtPoint(const Vector &position, Vector &ambientLightOut)
{
    Vector initialLight;

    // Отримання освітлення в точці
    if (!engine->GetLightForPointListenServerOnly(position, true, &initialLight)) {
        return 0;
    }

    // Обчислення максимальної інтенсивності освітлення з обмеженням до 1.0
    float maxIntensity = MAX(MAX(initialLight.x, initialLight.y), initialLight.z) * 3.0f;
    maxIntensity = MIN(maxIntensity, 1.0f);

    // Позиції для трасування світла
    Vector startPos = position;
    Vector endPos = position;
    endPos.z -= 150.0f;

    Vector traceLight, traceResult;

    // Трасування світла
    if (!engine->TraceLightingListenServerOnly(startPos, endPos, &traceLight, &traceResult)) {
        return 0;
    }

    // Інтенсивність світла
    float lightMagnitude = traceLight.Length();

    // Коригувальний фактор у діапазоні від 0.25 до 0.75
    float adjustmentFactor = CLAMP(lightMagnitude, 0.25f, 0.75f);

    // Кінцеве освітлення
    ambientLightOut.x = traceLight.x * adjustmentFactor * 15.0f;
    ambientLightOut.y = traceLight.y * adjustmentFactor * 15.0f;
    ambientLightOut.z = traceLight.z * adjustmentFactor * 15.0f;

    return 1;
}

bool INavArea::ComputeLighting(void)
{
	if ( engine->IsDedicatedServer() )
	{
		for ( int i=0; i<NUM_CORNERS; ++i )
		{
			m_lightIntensity[i] = 1.0f;
		}

		return true;
	}

	// Calculate light at the corners
	for ( int i=0; i<NUM_CORNERS; ++i )
	{
		Vector pos = FindPositionInArea( this, (NavCornerType)i );
		pos.z = GetZ( pos ) + HalfHumanHeight - StepHeight;	// players light from their centers, and we light from slightly below that, to allow for low ceilings
		float height;
		if ( TheNavMesh->GetGroundHeight( pos, &height ) )
		{
			pos.z = height + HalfHumanHeight - StepHeight;	// players light from their centers, and we light from slightly below that, to allow for low ceilings
		}

		Vector light( 0, 0, 0 );
		// FIXMEL4DTOMAINMERGE
		if ( !engine->GetLightForPointListenServerOnly( pos, false, &light ) )
		{
			//NDebugOverlay::Line( pos, pos + Vector( 0, 0, -100 ), 255, 0, 0, false, 100.0f );
			return false;
		}

		Vector ambientColor;
		// FIXMEL4DTOMAINMERGE
		if ( !GetTerrainAmbientLightAtPoint( pos, ambientColor ) )
		{
			//NDebugOverlay::Line( pos, pos + Vector( 0, 0, -100 ), 255, 127, 0, false, 100.0f );
			return false;
		}

		//NDebugOverlay::Line( pos, pos + Vector( 0, 0, -100 ), 0, 255, 127, false, 100.0f );

		float ambientIntensity = ambientColor.x + ambientColor.y + ambientColor.z;
		float lightIntensity = light.x + light.y + light.z;
		lightIntensity = clamp( lightIntensity, 0.f, 1.f );	// sum can go well over 1.0, but it's the lower region we care about.  if it's bright, we don't need to know *how* bright.

		lightIntensity = MAX( lightIntensity, ambientIntensity );

		m_lightIntensity[i] = lightIntensity;
	}

	return true;
}

void INavArea::InheritAttributes(CNavArea *first, CNavArea *second)
{
	if ( first && second )
	{
		SetAttributes( reinterpret_cast<INavArea*>(first)->GetAttributes() | reinterpret_cast<INavArea*>(second)->GetAttributes() );

		if ( reinterpret_cast<INavArea*>(first)->GetPlace() == reinterpret_cast<INavArea*>(second)->GetPlace() )
		{
			SetPlace( reinterpret_cast<INavArea*>(first)->GetPlace() );
		}
		else if ( reinterpret_cast<INavArea*>(first)->GetPlace() == UNDEFINED_PLACE )
		{
			SetPlace( reinterpret_cast<INavArea*>(second)->GetPlace() );
		}
		else if ( reinterpret_cast<INavArea*>(second)->GetPlace() == UNDEFINED_PLACE )
		{
			SetPlace( reinterpret_cast<INavArea*>(first)->GetPlace() );
		}
		else
		{
			if ( RandomInt( 0, 100 ) < 50 )
				SetPlace( reinterpret_cast<INavArea*>(first)->GetPlace() );
			else
				SetPlace( reinterpret_cast<INavArea*>(second)->GetPlace() );
		}
	}
	else if ( first )
	{
		SetAttributes( GetAttributes() | reinterpret_cast<INavArea*>(first)->GetAttributes() );
		if ( GetPlace() == UNDEFINED_PLACE )
		{
			SetPlace( reinterpret_cast<INavArea*>(first)->GetPlace() );
		}
	}
}

INavArea::INavArea(void)
{
    m_marker = 0;
	m_nearNavSearchMarker = 0;
	m_damagingTickCount = 0;
	m_openMarker = 0;

	m_parent = NULL;
	m_parentHow = GO_NORTH;
	m_attributeFlags = 0;
	m_place = TheNavMesh->GetNavPlace();
	m_isUnderwater = false;
	m_avoidanceObstacleHeight = 0.0f;

	m_totalCost = 0.0f;
	m_costSoFar = 0.0f;
	m_pathLengthSoFar = 0.0f;

	ResetNodes();

	int i;
	for ( i=0; i<MAX_NAV_TEAMS; ++i )
	{
		m_isBlocked[i] = false;

		m_danger[i] = 0.0f;
		m_dangerTimestamp[i] = 0.0f;

		m_clearedTimestamp[i] = 0.0f;

		m_earliestOccupyTime[i] = 0.0f;
	
		m_playerCount[i] = 0;
	}

	m_id = m_nextID++;
	m_debugid = 0;

	m_prevHash = NULL;
	m_nextHash = NULL;

	m_isBattlefront = false;

	for( i = 0; i<NUM_DIRECTIONS; ++i )
	{
		m_connect[i].RemoveAll();
	}

	for( i=0; i < CNavLadder::NUM_LADDER_DIRECTIONS; ++i )
	{
		m_ladder[i].RemoveAll();
	}

	for ( i=0; i<NUM_CORNERS; ++i )
	{
		m_lightIntensity[i] = 1.0f;
	}

	m_elevator = NULL;
	m_elevatorAreas.RemoveAll();

	m_invDxCorners = 0;
	m_invDyCorners = 0;

	m_inheritVisibilityFrom.area = NULL;
	m_isInheritedFrom = false;

	m_funcNavCostVector.RemoveAll();

	m_nVisTestCounter = (uint32)-1;
}

INavArea::~INavArea()
{
	// spot encounters aren't owned by anything else, so free them up here
	m_spotEncounters.PurgeAndDeleteElements();

	// if we are resetting the system, don't bother cleaning up - all areas are being destroyed
	if (m_isReset)
		return;

	// tell the other areas and ladders we are going away
	AreaDestroyNotification notification( this );
	TheNavMesh->ForAllAreas( notification );
	TheNavMesh->ForAllLadders( notification );

	// remove the area from the grid
	TheNavMesh->RemoveNavArea( this );
	
	// make sure no players keep a pointer to this area
	ForgetArea forget( this );
	ForEachActor( forget );
}

void INavArea::OnServerActivate(void)
{
    ConnectElevators();
    m_damagingTickCount = 0;
    ClearAllNavCostEntities();
}

void INavArea::OnRoundRestart(void)
{
    ConnectElevators();
    m_damagingTickCount = 0;
    ClearAllNavCostEntities();
}

void INavArea::OnDestroyNotify(CNavArea *dead)
{
	NavConnect con;
	con.area = dead;
	for( int d=0; d<NUM_DIRECTIONS; ++d )
	{
		m_connect[ d ].FindAndRemove( con );
		m_incomingConnect[ d ].FindAndRemove( con );
	}

	m_inheritVisibilityFrom.area = NULL;
	m_potentiallyVisibleAreas.RemoveAll();
	m_isInheritedFrom = false;

}

void INavArea::OnDestroyNotify( CNavLadder *dead )
{
    Disconnect(dead);
}

void INavArea::DrawFilled(int r, int g, int b, int a, float deltaT, bool noDepthTest, float margin) const
{
}

void INavArea::DrawSelectedSet(const Vector &shift) const
{
}*/

float INavArea::GetZ(float x, float y) const
{
	// guard against division by zero due to degenerate areas
	if (m_invDxCorners == 0.0f || m_invDyCorners == 0.0f)
		return m_neZ;

	float u = (x - m_nwCorner.x) * m_invDxCorners;
	float v = (y - m_nwCorner.y) * m_invDyCorners;

	// clamp Z values to (x,y) volume
	
	u = fsel( u, u, 0 );			// u >= 0 ? u : 0
	u = fsel( u - 1.0f, 1.0f, u );	// u >= 1 ? 1 : u

	v = fsel( v, v, 0 );			// v >= 0 ? v : 0
	v = fsel( v - 1.0f, 1.0f, v );	// v >= 1 ? 1 : v

	float northZ = m_nwCorner.z + u * (m_neZ - m_nwCorner.z);
	float southZ = m_swZ + u * (m_seCorner.z - m_swZ);

	return northZ + v * (southZ - northZ);
}

class COverlapCheck
{
public:
	COverlapCheck( const INavArea *me, const Vector &pos ) : m_pos( pos )
	{
		m_me = me;
		m_myZ = me->GetZ( pos );
	}

	bool operator() ( INavArea *area )
	{
		// skip self
		if ( area == m_me )
			return true;

		// check 2D overlap
		if ( !area->IsOverlapping( m_pos ) )
			return true;

		float theirZ = area->GetZ( m_pos );
		if ( theirZ > m_pos.z )
		{
			// they are above the point
			return true;
		}

		if ( theirZ > m_myZ )
		{
			// we are below an area that is beneath the given position
			return false;
		}

		return true;
	}

	const INavArea *m_me;
	float m_myZ;
	const Vector &m_pos;
};

void INavArea::CheckWaterLevel(void)
{
	INavMesh *pMesh = nullptr;
	if((pMesh = g_HL2->GetTheNavMesh()) == nullptr)
	{
		m_isUnderwater = false;
		return;
	}

	Vector pos( GetCenter() );
	if ( !pMesh->GetGroundHeight( pos, &pos.z ) )
	{
		m_isUnderwater = false;
		return;
	}

	pos.z += 1;
	m_isUnderwater = (g_pTarce->GetPointContents( pos ) & MASK_WATER ) != 0;
}

bool INavArea::IsOverlapping(const Vector &pos, float tolerance) const
{
	if (pos.x + tolerance >= m_nwCorner.x && pos.x - tolerance <= m_seCorner.x &&
		pos.y + tolerance >= m_nwCorner.y && pos.y - tolerance <= m_seCorner.y)
		return true;

	return false;
}

bool INavArea::IsOverlapping(const INavArea *area) const
{
	if (area->m_nwCorner.x < m_seCorner.x && area->m_seCorner.x > m_nwCorner.x && 
		area->m_nwCorner.y < m_seCorner.y && area->m_seCorner.y > m_nwCorner.y)
		return true;

	return false;
}

bool INavArea::IsOverlapping(const Extent &extent) const
{
	return ( extent.lo.x < m_seCorner.x && extent.hi.x > m_nwCorner.x && 
			 extent.lo.y < m_seCorner.y && extent.hi.y > m_nwCorner.y );
}

bool INavArea::IsOverlappingX(const INavArea *area) const
{
	if (area->m_nwCorner.x < m_seCorner.x && area->m_seCorner.x > m_nwCorner.x)
		return true;

	return false;
}

bool INavArea::IsOverlappingY(const INavArea *area) const
{
	if (area->m_nwCorner.y < m_seCorner.y && area->m_seCorner.y > m_nwCorner.y)
		return true;

	return false;
}

bool INavArea::Contains(const Vector &pos) const
{
	// check 2D overlap
	if (!IsOverlapping( pos ))
		return false;

	// the point overlaps us, check that it is above us, but not above any areas that overlap us
	float myZ = GetZ( pos );

	// if the nav area is above the given position, fail
	// allow nav area to be as much as a step height above the given position
	if (myZ - StepHeight > pos.z)
		return false;

	Extent areaExtent;
	GetExtent( &areaExtent );

	INavMesh* pMesh = g_HL2->GetTheNavMesh();
	if(pMesh )
	{
		COverlapCheck overlap( this, pos );
		return pMesh->ForAllAreasOverlappingExtent( overlap, areaExtent );
	}

	DevMsg("[INavArea::Contains] INavMesh is null\n");
	return false;
}

bool INavArea::Contains(const INavArea *area) const
{
	return ( ( m_nwCorner.x <= area->m_nwCorner.x ) && ( m_seCorner.x >= area->m_seCorner.x ) &&
		( m_nwCorner.y <= area->m_nwCorner.y ) && ( m_seCorner.y >= area->m_seCorner.y ) &&
		( m_nwCorner.z <= area->m_nwCorner.z ) && ( m_seCorner.z >= area->m_seCorner.z ) );
}

void INavArea::GetClosestPointOnArea(const Vector *pPos, Vector *close) const
{
	float x, y, z;

	// Using fsel rather than compares, as much faster on 360 [7/28/2008 tom]
	x = fsel( pPos->x - m_nwCorner.x, pPos->x, m_nwCorner.x );
	x = fsel( x - m_seCorner.x, m_seCorner.x, x );

	y = fsel( pPos->y - m_nwCorner.y, pPos->y, m_nwCorner.y );
	y = fsel( y - m_seCorner.y, m_seCorner.y, y );

	z = GetZ( x, y );

	close->Init( x, y, z );
}

float INavArea::GetDistanceSquaredToPoint(const Vector &pos) const
{
	if (pos.x < m_nwCorner.x)
	{
		if (pos.y < m_nwCorner.y)
		{
			// position is north-west of area
			return (m_nwCorner - pos).LengthSqr();
		}
		else if (pos.y > m_seCorner.y)
		{
			// position is south-west of area
			Vector d;
			d.x = m_nwCorner.x - pos.x;
			d.y = m_seCorner.y - pos.y;
			d.z = m_swZ - pos.z;
			return d.LengthSqr();
		}
		else
		{
			// position is west of area
			float d = m_nwCorner.x - pos.x;
			return d * d;
		}
	}
	else if (pos.x > m_seCorner.x)
	{
		if (pos.y < m_nwCorner.y)
		{
			// position is north-east of area
			Vector d;
			d.x = m_seCorner.x - pos.x;
			d.y = m_nwCorner.y - pos.y;
			d.z = m_neZ - pos.z;
			return d.LengthSqr();
		}
		else if (pos.y > m_seCorner.y)
		{
			// position is south-east of area
			return (m_seCorner - pos).LengthSqr();
		}
		else
		{
			// position is east of area
			float d = pos.x - m_seCorner.x;
			return d * d;
		}
	}
	else if (pos.y < m_nwCorner.y)
	{
		// position is north of area
		float d = m_nwCorner.y - pos.y;
		return d * d;
	}
	else if (pos.y > m_seCorner.y)
	{
		// position is south of area
		float d = pos.y - m_seCorner.y;
		return d * d;
	}
	else
	{
		// position is inside of 2D extent of area - find delta Z
		float z = GetZ( pos );
		float d = z - pos.z;
		return d * d;
	}
}

INavArea *INavArea::GetRandomAdjacentArea(Nav_DirType dir) const
{
	int count = m_connect[ dir ].Count();
	int which = RandomInt( 0, count-1 );

	int i = 0;
	_FOR_EACH_VEC_( m_connect[ dir ], it )
	{
		if (i == which)
			return m_connect[ dir ][ it ].area;

		++i;
	}

	return NULL;
}

void INavArea::CollectAdjacentAreas(CUtlVector<INavArea *> *adjVector) const
{
	for( int d=0; d<NUM_DIRECTIONS; ++d )
	{
		for( int i=0; i<m_connect[d].Count(); ++i )
		{
			adjVector->AddToTail( m_connect[d].Element(i).area );
		}
	}
}

bool INavArea::IsContiguous(const INavArea *other) const
{
	int dir;
	for( dir=0; dir<NUM_DIRECTIONS; ++dir )
	{
		if ( IsConnected( other, (Nav_DirType)dir ) )
			break;
	}

	if ( dir == NUM_DIRECTIONS )
		return false;

	Vector myEdge;
	float halfWidth;
	ComputePortal( other, (Nav_DirType)dir, &myEdge, &halfWidth );

	Vector otherEdge;
	other->ComputePortal( this, OppositeDirection( (Nav_DirType)dir ), &otherEdge, &halfWidth );

	// must use stepheight because rough terrain can have gaps/cracks between adjacent nav areas
	return ( myEdge - otherEdge ).IsLengthLessThan( StepHeight );
}

float INavArea::ComputeAdjacentConnectionHeightChange(const INavArea *destinationArea) const
{
	int dir;
	for( dir=0; dir<NUM_DIRECTIONS; ++dir )
	{
		if ( IsConnected( destinationArea, (Nav_DirType)dir ) )
			break;
	}

	if ( dir == NUM_DIRECTIONS )
		return FLT_MAX;

	Vector myEdge;
	float halfWidth;
	ComputePortal( destinationArea, (Nav_DirType)dir, &myEdge, &halfWidth );

	Vector otherEdge;
	destinationArea->ComputePortal( this, OppositeDirection( (Nav_DirType)dir ), &otherEdge, &halfWidth );

	return otherEdge.z - myEdge.z;
}

bool INavArea::IsEdge(Nav_DirType dir) const
{
	FOR_EACH_VEC( m_connect[ dir ], it )
	{
		const NavConnect connect = m_connect[ dir ][ it ];

		if (reinterpret_cast<INavArea *>(connect.area)->IsConnected( this, OppositeDirection( dir ) ))
			return false;
	}

	return true;
}

bool INavArea::IsConnected(const INavArea *area, Nav_DirType dir) const
{
	if (area == this)
		return true;

	if (dir == NUM_DIRECTIONS)
	{
		// search all directions
		for( int d=0; d<NUM_DIRECTIONS; ++d )
		{
			FOR_EACH_VEC( m_connect[ d ], it )
			{
				if (area == (INavArea*)m_connect[ d ][ it ].area)
					return true;
			}
		}

		// check ladder connections
		FOR_EACH_VEC( m_ladder[ Ladder_DirectionType::LADDER_UP ], it )
		{
			INavLadder *ladder = (INavLadder*)m_ladder[ Ladder_DirectionType::LADDER_UP ][ it ].ladder;

			if (ladder->m_topBehindArea == area ||
				ladder->m_topForwardArea == area ||
				ladder->m_topLeftArea == area ||
				ladder->m_topRightArea == area)
				return true;
		}

		FOR_EACH_VEC( m_ladder[ Ladder_DirectionType::LADDER_DOWN ], dit )
		{
			INavLadder *ladder = (INavLadder*)m_ladder[ Ladder_DirectionType::LADDER_DOWN ][ dit ].ladder;

			if (ladder->m_bottomArea == area)
				return true;
		}
	}
	else
	{
		// check specific direction
		FOR_EACH_VEC( m_connect[ dir ], it )
		{
			if (area == (INavArea*)m_connect[ dir ][ it ].area)
				return true;
		}
	}

	return false;
}

bool INavArea::IsConnected(const INavLadder *ladder, Ladder_DirectionType dir) const
{
	FOR_EACH_VEC( m_ladder[ dir ], it )
	{
		if ( ladder == m_ladder[ dir ][ it ].ladder )
		{
			return true;
		}
	}

	return false;
}

void INavArea::ComputePortal(const INavArea *to, Nav_DirType dir, Vector *center, float *halfWidth) const
{
	if ( dir == NORTH || dir == SOUTH )
	{
		if ( dir == NORTH )
		{
			center->y = m_nwCorner.y;
		}
		else
		{
			center->y = m_seCorner.y;
		}

		float left = MAX( m_nwCorner.x, to->m_nwCorner.x );
		float right = MIN( m_seCorner.x, to->m_seCorner.x );

		// clamp to our extent in case areas are disjoint
		if ( left < m_nwCorner.x )
		{
			left = m_nwCorner.x;
		}
		else if ( left > m_seCorner.x )
		{
			left = m_seCorner.x;
		}

		if ( right < m_nwCorner.x )
		{
			right = m_nwCorner.x;
		}
		else if ( right > m_seCorner.x )
		{
			right = m_seCorner.x;
		}

		center->x = ( left + right )/2.0f;
		*halfWidth = ( right - left )/2.0f;
	}
	else	// EAST or WEST
	{
		if ( dir == WEST )
		{
			center->x = m_nwCorner.x;
		}
		else
		{
			center->x = m_seCorner.x;
		}

		float top = MAX( m_nwCorner.y, to->m_nwCorner.y );
		float bottom = MIN( m_seCorner.y, to->m_seCorner.y );

		// clamp to our extent in case areas are disjoint
		if ( top < m_nwCorner.y )
		{
			top = m_nwCorner.y;
		}
		else if ( top > m_seCorner.y )
		{
			top = m_seCorner.y;
		}

		if ( bottom < m_nwCorner.y )
		{
			bottom = m_nwCorner.y;
		}
		else if ( bottom > m_seCorner.y )
		{
			bottom = m_seCorner.y;
		}

		center->y = (top + bottom)/2.0f;
		*halfWidth = (bottom - top)/2.0f;
	}

	center->z = GetZ( center->x, center->y );
}

void INavArea::ComputeClosestPointInPortal(const INavArea *to, Nav_DirType dir, const Vector &fromPos, Vector *closePos) const
{
//	const float margin = 0.0f; //GenerationStepSize/2.0f;  // causes trouble with very small/narrow nav areas
	const float margin = GenerationStepSize;

	if ( dir == NORTH || dir == SOUTH )
	{
		if ( dir == NORTH )
		{
			closePos->y = m_nwCorner.y;
		}
		else
		{
			closePos->y = m_seCorner.y;
		}

		float left = MAX( m_nwCorner.x, to->m_nwCorner.x );
		float right = MIN( m_seCorner.x, to->m_seCorner.x );

		// clamp to our extent in case areas are disjoint
		// no good - need to push into to area for margins
		/*
		if (left < m_nwCorner.x)
			left = m_nwCorner.x;
		else if (left > m_seCorner.x)
			left = m_seCorner.x;

		if (right < m_nwCorner.x)
			right = m_nwCorner.x;
		else if (right > m_seCorner.x)
			right = m_seCorner.x;
			*/

		// keep margin if against edge
		/// @todo Need better check whether edge is outer edge or not - partial overlap is missed
		float leftMargin = ( to->IsEdge( WEST ) ) ? ( left + margin ) : left;
		float rightMargin = ( to->IsEdge( EAST ) ) ? ( right - margin ) : right;
		
		// if area is narrow, margins may have crossed
		if ( leftMargin > rightMargin )
		{
			// use midline
			float mid = ( left + right )/2.0f;
			leftMargin = mid;
			rightMargin = mid;
		}

		// limit x to within portal
		if ( fromPos.x < leftMargin )
		{
			closePos->x = leftMargin;
		}
		else if ( fromPos.x > rightMargin )
		{
			closePos->x = rightMargin;
		}
		else
		{
			closePos->x = fromPos.x;
		}
	}
	else	// EAST or WEST
	{
		if ( dir == WEST )
		{
			closePos->x = m_nwCorner.x;
		}
		else
		{
			closePos->x = m_seCorner.x;
		}

		float top = MAX( m_nwCorner.y, to->m_nwCorner.y );
		float bottom = MIN( m_seCorner.y, to->m_seCorner.y );

		// clamp to our extent in case areas are disjoint
		// no good - need to push into to area for margins
		/*
		if (top < m_nwCorner.y)
			top = m_nwCorner.y;
		else if (top > m_seCorner.y)
			top = m_seCorner.y;

		if (bottom < m_nwCorner.y)
			bottom = m_nwCorner.y;
		else if (bottom > m_seCorner.y)
			bottom = m_seCorner.y;
		*/
		
		// keep margin if against edge
		float topMargin = ( to->IsEdge( NORTH ) ) ? ( top + margin ) : top;
		float bottomMargin = ( to->IsEdge( SOUTH ) ) ? ( bottom - margin ) : bottom;

		// if area is narrow, margins may have crossed
		if ( topMargin > bottomMargin )
		{
			// use midline
			float mid = ( top + bottom )/2.0f;
			topMargin = mid;
			bottomMargin = mid;
		}

		// limit y to within portal
		if ( fromPos.y < topMargin )
		{
			closePos->y = topMargin;
		}
		else if ( fromPos.y > bottomMargin )
		{
			closePos->y = bottomMargin;
		}
		else
		{
			closePos->y = fromPos.y;
		}
	}

	closePos->z = GetZ( closePos->x, closePos->y );
}

float INavArea::GetDanger(int teamID)
{
	DecayDanger();

	int teamIdx = teamID % 2;
    return m_danger[teamIdx];
}

void INavArea::GetExtent(Extent *extent) const
{
	extent->lo = m_nwCorner;
	extent->hi = m_seCorner;

	extent->lo.z = MIN( extent->lo.z, m_nwCorner.z );
	extent->lo.z = MIN( extent->lo.z, m_seCorner.z );
	extent->lo.z = MIN( extent->lo.z, m_neZ );
	extent->lo.z = MIN( extent->lo.z, m_swZ );

	extent->hi.z = MAX( extent->hi.z, m_nwCorner.z );
	extent->hi.z = MAX( extent->hi.z, m_seCorner.z );
	extent->hi.z = MAX( extent->hi.z, m_neZ );
	extent->hi.z = MAX( extent->hi.z, m_swZ );
}

void INavArea::ComputeNormal(Vector *normal, bool alternate) const
{
	if ( !normal )
		return;

	Vector u, v;

	if ( !alternate )
	{
		u.x = m_seCorner.x - m_nwCorner.x;
		u.y = 0.0f;
		u.z = m_neZ - m_nwCorner.z;

		v.x = 0.0f;
		v.y = m_seCorner.y - m_nwCorner.y;
		v.z = m_swZ - m_nwCorner.z;
	}
	else
	{
		u.x = m_nwCorner.x - m_seCorner.x;
		u.y = 0.0f;
		u.z = m_swZ - m_seCorner.z;

		v.x = 0.0f;
		v.y = m_nwCorner.y - m_seCorner.y;
		v.z = m_neZ - m_seCorner.z;
	}

	*normal = CrossProduct( u, v );
	normal->NormalizeInPlace();
}

float INavArea::GetLightIntensity(void) const
{
	float light = m_lightIntensity[ NORTH_WEST ];
	light += m_lightIntensity[ NORTH_EAST ];
	light += m_lightIntensity[ SOUTH_WEST];
	light += m_lightIntensity[ SOUTH_EAST ];
	return light / 4.0f;
}

/**
 * Returns a 0..1 light intensity for the given point
 */
float INavArea::GetLightIntensity( const Vector &pos ) const
{
	Vector testPos;
	testPos.x = clamp( pos.x, m_nwCorner.x, m_seCorner.x );
	testPos.y = clamp( pos.y, m_nwCorner.y, m_seCorner.y );
	testPos.z = pos.z;

	float dX = (testPos.x - m_nwCorner.x) / (m_seCorner.x - m_nwCorner.x);
	float dY = (testPos.y - m_nwCorner.y) / (m_seCorner.y - m_nwCorner.y);

	float northLight = m_lightIntensity[ NORTH_WEST ] * ( 1 - dX ) + m_lightIntensity[ NORTH_EAST ] * dX;
	float southLight = m_lightIntensity[ SOUTH_WEST ] * ( 1 - dX ) + m_lightIntensity[ SOUTH_EAST ] * dX;
	float light = northLight * ( 1 - dY ) + southLight * dY;

	return light;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Returns a 0..1 light intensity for the given point
 */
float INavArea::GetLightIntensity( float x, float y ) const
{
	return GetLightIntensity( Vector( x, y, 0 ) );
}

Vector INavArea::FindRandomSpot()
{
    if((GetCorner(SOUTH_EAST).x - GetCorner(NORTH_WEST).x) >= 50.f 
    && (GetCorner(SOUTH_EAST).y - GetCorner(NORTH_WEST).y) >= 50.f)
    {
        Vector vecResult;

        float flNorthX = GetCorner(NORTH_WEST).x + 25.f;
        float flRiznX = (GetCorner(SOUTH_EAST).x - GetCorner(NORTH_WEST).x) - 50.f;
        float flFinaleX = RandomFloat(0, flRiznX) + flNorthX;
        vecResult.x = flFinaleX;

        float flNorthY = GetCorner(NORTH_WEST).y + 25.f;
        float flRiznY = (GetCorner(SOUTH_EAST).y - GetCorner(NORTH_WEST).y) - 50.f;
        float flFinaleY = RandomFloat(0, flRiznY) + flNorthY;
        vecResult.y = flFinaleY;
        vecResult.z = GetZ(flFinaleX, flFinaleY) + 15.f;

        return vecResult;
    }
    else
    {
        return (GetCenter() + Vector(0.f, 0.f, 15.f));
    }
}

void INavArea::DecayDanger(void)
{
	for( int i=0; i < 2; ++i )
	{
		float deltaT = g_pGlobals->curtime - m_dangerTimestamp[i];
		float decayAmount = GetDangerDecayRate() * deltaT;

		m_danger[i] -= decayAmount;
		if (m_danger[i] < 0.0f)
			m_danger[i] = 0.0f;

		// update timestamp
		m_dangerTimestamp[i] = g_pGlobals->curtime;
	}
}

void INavArea::ResetNodes(void)
{
	for ( int i=0; i<NUM_CORNERS; ++i )
	{
		m_node[i] = NULL;
	}
}

bool INavArea::IsRoughlySquare(void) const
{
	float aspect = GetSizeX() / GetSizeY();

	const float maxAspect = 3.01;
	const float minAspect = 1.0f / maxAspect;
	if (aspect < minAspect || aspect > maxAspect)
		return false;

	return true;
}

bool INavArea::HasNodes(void) const
{
	for ( int i=0; i<NUM_CORNERS; ++i )
	{
		if ( m_node[i] )
		{
			return true;
		}
	}

	return false;
}

void INavArea::Mark(void)
{
	unsigned int *m_masterMarker = g_HL2->GetNavArea_masterMarker();
	m_marker = *m_masterMarker;
}

BOOL INavArea::IsMarked(void) const
{
	unsigned int *m_masterMarker = g_HL2->GetNavArea_masterMarker();
	return (m_marker == *m_masterMarker) ? true : false;
}

bool INavArea::IsOpen(void) const
{
	unsigned int *m_masterMarker = g_HL2->GetNavArea_masterMarker();
	return (m_openMarker == *m_masterMarker) ? true : false;
}

void INavArea::AddToOpenList(void)
{
	unsigned int *m_masterMarker = g_HL2->GetNavArea_masterMarker();
	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());
	// INavArea **m_openListTail = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openListTail());

	Assert( (m_openList && (m_openList)->m_prevOpen == NULL) || m_openList == NULL );

	if ( IsOpen() )
	{
		// already on list
		return;
	}

	// mark as being on open list for quick check
	m_openMarker = *m_masterMarker;

	// if list is empty, add and return
	if ( m_openList == NULL )
	{
		m_openList = this;
		m_openListTail = this;
		this->m_prevOpen = NULL;
		this->m_nextOpen = NULL;
		return;
	}

	// insert self in ascending cost order
	INavArea *area, *last = NULL;
	int thisCostBits = *reinterpret_cast<const int *>(&m_totalCost);
	for( area = m_openList; area; area = (INavArea*)area->m_nextOpen )
	{
		int thoseCostBits = *reinterpret_cast<const int *>(&area->m_totalCost);
		if ( thisCostBits < thoseCostBits )
		{
			break;
		}
		last = area;
	}

	if ( area )
	{
		// insert before this area
		this->m_prevOpen = area->m_prevOpen;

		if ( this->m_prevOpen )
		{
			reinterpret_cast<INavArea*>(this->m_prevOpen)->m_nextOpen = (CNavArea*)this;
		}
		else
		{
			m_openList = this;
		}

		this->m_nextOpen = (CNavArea*)area;
		area->m_prevOpen = (CNavArea*)this;
	}
	else
	{
		// append to end of list
		last->m_nextOpen = (CNavArea*)this;
		this->m_prevOpen = (CNavArea*)last;
	
		this->m_nextOpen = NULL;

		m_openListTail = this;
	}

	Assert( (m_openList && (m_openList)->m_prevOpen == NULL) || m_openList == NULL );
}

void INavArea::AddToOpenListTail(void)
{
	unsigned int *m_masterMarker = g_HL2->GetNavArea_masterMarker();
	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());
	// INavArea **m_openListTail = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openListTail());

	Assert( (m_openList && (m_openList)->m_prevOpen == NULL) || m_openList == NULL );

	if ( IsOpen() )
	{
		return;
	}

	// mark as being on open list for quick check
	m_openMarker = *m_masterMarker;

	// if list is empty, add and return
	if ( m_openList == NULL )
	{
		m_openList = this;
		m_openListTail = this;
		this->m_prevOpen = NULL;
		this->m_nextOpen = NULL;

		Assert( (m_openList && (m_openList)->m_prevOpen == NULL) || m_openList == NULL );
		return;
	}

	// append to end of list
	(m_openListTail)->m_nextOpen = (CNavArea*)this;

	this->m_prevOpen = (CNavArea*)m_openListTail;
	this->m_nextOpen = NULL;

	m_openListTail = this;

	Assert( (*m_openList && *m_openList->m_prevOpen == NULL) || *m_openList == NULL );
}

void INavArea::UpdateOnOpenList(void)
{
	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());
	// INavArea **m_openListTail = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openListTail());

	// since value can only decrease, bubble this area up from current spot
	while( m_prevOpen && this->GetTotalCost() < reinterpret_cast<INavArea*>(m_prevOpen)->GetTotalCost() )
	{
		// swap position with predecessor
		INavArea *other = (INavArea*)m_prevOpen;
		INavArea *before = (INavArea*)other->m_prevOpen;
		INavArea *after  = (INavArea*)this->m_nextOpen;

		this->m_nextOpen = (CNavArea*)other;
		this->m_prevOpen = (CNavArea*)before;

		other->m_prevOpen = (CNavArea*)this;
		other->m_nextOpen = (CNavArea*)after;

		if ( before )
		{
			before->m_nextOpen = (CNavArea*)this;
		}
		else
		{
			m_openList = this;
		}

		if ( after )
		{
			after->m_prevOpen = (CNavArea*)other;
		}
		else
		{
			m_openListTail = this;
		}
	}
}

void INavArea::RemoveFromOpenList(void)
{
	if ( m_openMarker == 0 )
	{
		// not on the list
		return;
	}

	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());
	// INavArea **m_openListTail = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openListTail());

	if ( m_prevOpen )
	{
		reinterpret_cast<INavArea*>(m_prevOpen)->m_nextOpen = m_nextOpen;
	}
	else
	{
		m_openList = (INavArea*)m_nextOpen;
	}
	
	if ( m_nextOpen )
	{
		reinterpret_cast<INavArea*>(m_nextOpen)->m_prevOpen = m_prevOpen;
	}
	else
	{
		m_openListTail = (INavArea*)m_prevOpen;
	}
	
	// zero is an invalid marker
	m_openMarker = 0;
}


bool INavArea::IsClosed(void) const
{
	if (IsMarked() && !IsOpen())
		return true;

	return false;
}

void INavArea::AddToClosedList(void)
{
	Mark();
}

void INavArea::RemoveFromClosedList(void)
{
}

float INavArea::GetZDeltaAtEdgeToArea(const INavArea *area)
{
	if(HasAttributes(NAV_MESH_RUN) && area->HasAttributes(NAV_MESH_RUN))
		return (area->GetCorner(SOUTH_WEST).z - GetCorner(SOUTH_WEST).z);

	Vector vecClose;
	if(area->HasAttributes(NAV_MESH_RUN))
		vecClose = area->GetCenter();
	else
		area->GetClosestPointOnArea(GetCenter(), &vecClose);

	if(HasAttributes(NAV_MESH_RUN))
		return (vecClose.z - GetCorner(SOUTH_WEST).z);

	Vector vec_close;
	GetClosestPointOnArea(vecClose, &vec_close);

	return (vecClose.z - vec_close.z);
}

bool INavArea::IsBlocked(int teamID, bool ignoreNavBlockers) const
{
	if ( ignoreNavBlockers && ( m_attributeFlags & NAV_MESH_NAV_BLOCKER ) )
	{
		return false;
	}

	if ( teamID == -1 )
	{
		bool isBlocked = false;
		for ( int i = 0; i < 2; ++i )
		{
			isBlocked |= m_isBlocked[ i ];
		}

		return isBlocked;
	}

	int teamIdx = teamID % 2;
	return m_isBlocked[ teamIdx ];
}

void MakeNewMarker(void)
{
	unsigned int *m_masterMarker = g_HL2->GetNavArea_masterMarker();

	++(*m_masterMarker);
	if (*(m_masterMarker) == 0)
		*m_masterMarker = 1;
}

/**
 * Clears the open and closed lists for a new search
 */
void INavArea::ClearSearchLists( void )
{
	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());
	// INavArea **m_openListTail = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openListTail());

	// effectively clears all open list pointers and closed flags
	MakeNewMarker();

	m_openList = nullptr;
	m_openListTail = nullptr;
}

//--------------------------------------------------------------------------------------------------------------
bool INavArea::IsOpenListEmpty( void )
{
	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());
	return m_openList ? false : true;
}

//--------------------------------------------------------------------------------------------------------------
INavArea *INavArea::PopOpenList( void )
{
	// INavArea **m_openList = reinterpret_cast<INavArea**>(g_HL2->GetNavArea_openList());

	if ( m_openList )
	{
		INavArea *area = m_openList;
	
		// disconnect from list
		area->RemoveFromOpenList();
		area->Reset();

		return area;
	}

	return NULL;
}

bool INavArea::IsVisible(const Vector &eye, Vector *visSpot) const
{
	Vector corner;
	trace_t result;
	CTraceFilterNoNPC_OrPlayer traceFilter( NULL, COLLISION_GROUP_NONE );
	const float offset = 0.75f * HumanHeight;

	// check center first
	util_TraceLine( eye, GetCenter() + Vector( 0, 0, offset ), MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, &traceFilter, &result );
	if (result.fraction == 1.0f)
	{
		// we can see this area
		if (visSpot)
		{
			*visSpot = GetCenter();
		}
		return true;
	}

	for( int c=0; c<NUM_CORNERS; ++c )
	{
		corner = GetCorner( (NavCornerType)c );
		util_TraceLine( eye, corner + Vector( 0, 0, offset ), MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, &traceFilter, &result );
		if (result.fraction == 1.0f)
		{
			// we can see this area
			if (visSpot)
			{
				*visSpot = corner;
			}
			return true;
		}
	}

	return false;
}

Vector INavArea::GetRandomPoint( void ) const
{
	Extent extent;
	GetExtent( &extent );

	Vector spot;
	spot.x = RandomFloat( extent.lo.x, extent.hi.x ); 
	spot.y = RandomFloat( extent.lo.y, extent.hi.y );
	spot.z = GetZ( spot.x, spot.y );

	return spot;
}

void INavArea::SetCorner(NavCornerType corner, const Vector &newPosition)
{
	switch( corner )
	{
		case NORTH_WEST:
			m_nwCorner = newPosition;
			break;

		case NORTH_EAST:
			m_seCorner.x = newPosition.x;
			m_nwCorner.y = newPosition.y;
			m_neZ = newPosition.z;
			break;

		case SOUTH_WEST:
			m_nwCorner.x = newPosition.x;
			m_seCorner.y = newPosition.y;
			m_swZ = newPosition.z;
			break;

		case SOUTH_EAST:
			m_seCorner = newPosition;
			break;

		default:
		{
			Vector oldPosition = GetCenter();
			Vector delta = newPosition - oldPosition;
			m_nwCorner += delta;
			m_seCorner += delta;
			m_neZ += delta.z;
			m_swZ += delta.z;
		}
	}

	m_center.x = (m_nwCorner.x + m_seCorner.x)/2.0f;
	m_center.y = (m_nwCorner.y + m_seCorner.y)/2.0f;
	m_center.z = (m_nwCorner.z + m_seCorner.z)/2.0f;

	if ( ( m_seCorner.x - m_nwCorner.x ) > 0.0f && ( m_seCorner.y - m_nwCorner.y ) > 0.0f )
	{
		m_invDxCorners = 1.0f / ( m_seCorner.x - m_nwCorner.x );
		m_invDyCorners = 1.0f / ( m_seCorner.y - m_nwCorner.y );
	}
	else
	{
		m_invDxCorners = m_invDyCorners = 0;
	}

	CalcDebugID();
}

void INavArea::CalcDebugID()
{
	if ( m_debugid == 0 )
	{
		int coord[6] = { (int) m_nwCorner.x, (int) m_nwCorner.x, (int) m_nwCorner.z, (int) m_seCorner.x, (int) m_seCorner.y, (int) m_seCorner.z };
		m_debugid = CRC32_ProcessSingleBuffer( &coord, sizeof( coord ) );
	}
}

void HidingSpot::Save( CUtlBuffer &fileBuffer, unsigned int version ) const
{
	fileBuffer.PutUnsignedInt( m_id );
	fileBuffer.PutFloat( m_pos.x );
	fileBuffer.PutFloat( m_pos.y );
	fileBuffer.PutFloat( m_pos.z );
	fileBuffer.PutUnsignedChar( m_flags );
}


//--------------------------------------------------------------------------------------------------------------
void HidingSpot::Load( CUtlBuffer &fileBuffer, unsigned int version )
{
	m_id = fileBuffer.GetUnsignedInt();
	m_pos.x = fileBuffer.GetFloat();
	m_pos.y = fileBuffer.GetFloat();
	m_pos.z = fileBuffer.GetFloat();
	m_flags = fileBuffer.GetUnsignedChar();

	// update next ID to avoid ID collisions by later spots
	unsigned int *m_nextID = g_HL2->GetHidingSpot_nextID(); 
	if (m_id >= *m_nextID)
		*m_nextID = m_id+1;
}

void HidingSpot::Mark(void)
{
	unsigned int *m_masterMarker = g_HL2->GetHidingSpot_masterMarker();
	m_marker = *m_masterMarker;
}

bool HidingSpot::IsMarked(void) const
{
	unsigned int *m_masterMarker = g_HL2->GetHidingSpot_masterMarker();
	return (m_marker == *m_masterMarker) ? true : false;
}

void ChangeMasterMarker(void)
{
	unsigned int *m_masterMarker = g_HL2->GetHidingSpot_masterMarker();
	++(*m_masterMarker);
}

bool INavArea::IsPotentiallyVisible(const INavArea *area) const
{
	if ( area == NULL )
	{
		return false;
	}

	if ( area == this )
	{
		return true;
	}

	for ( int i=0; i<m_potentiallyVisibleAreas.Count(); ++i )
	{
		if ( m_potentiallyVisibleAreas[i].area == area )
		{
			return ( m_potentiallyVisibleAreas[i].attributes != NOT_VISIBLE );
		}
	}

	if ( m_inheritVisibilityFrom.area )
	{
		CAreaBindInfoArray &inherited = m_inheritVisibilityFrom.area->m_potentiallyVisibleAreas;

		for ( int i=0; i<inherited.Count(); ++i )
		{
			if ( inherited[i].area == area )
			{
				return ( inherited[i].attributes != NOT_VISIBLE );
			}
		}
	}
	
	return false;
}

#include "ITeam.h"
bool INavArea::IsPotentiallyVisibleToTeam(int team) const
{
    ITeam *teams = g_HL2->GetGlobalTeam(team);
	for(int i = 0; i < teams->GetNumPlayers(); ++i)
	{
		if(reinterpret_cast<IBasePlayer*>(teams->GetPlayer(i))->IsAlive())
		{
			INavArea* from = (INavArea*)reinterpret_cast<IBasePlayer*>(teams->GetPlayer(i))->GetLastKnownArea();
			if(from && from->IsPotentiallyVisible(this))
			{
				return true;
			}
		}
	}

	return false;
}
