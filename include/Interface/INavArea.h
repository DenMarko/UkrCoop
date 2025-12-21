#ifndef _HEADER_NAV_AREA_INCLUDE_
#define _HEADER_NAV_AREA_INCLUDE_

#include "IBasePlayer.h"
#include "INavLadder.h"

#define _FOR_EACH_VEC_( vecName, iteratorName ) \
	for ( int iteratorName = 0; (vecName).IsUtlVector && iteratorName < (vecName).Count(); iteratorName++ )


class INavArea;
class INavLadder;

/**
 * Below are several constants used by the navigation system.
 * @todo Move these into TheNavMesh singleton.
 */
const float GenerationStepSize = 25.0f;			///< (30) was 20, but bots can't fit always fit
const float StepHeight = 18.0f;					///< if delta Z is greater than this, we have to jump to get up
const float JumpHeight = 41.8f;					///< if delta Z is less than this, we can jump up on it
const float JumpCrouchHeight = 58.0f;			///< (48) if delta Z is less than or equal to this, we can jumpcrouch up on it

const float BotRadius = 10.0f;					///< circular extent that contains bot
const float DeathDrop = 200.0f;					///< (300) distance at which we will die if we fall - should be about 600, and pay attention to fall damage during pathfind

const float HalfHumanWidth = 16.0f;
const float HalfHumanHeight = 35.5f;
const float HumanHeight = 71.0f;


#define NAV_MAGIC_NUMBER 0xFEEDFACE				///< to help identify nav files

/**
 * A place is a named group of navigation areas
 */
typedef unsigned int Place;
#define UNDEFINED_PLACE 0				// ie: "no place"
#define ANY_PLACE 0xFFFF

enum NavErrorType
{
	NAV_OK,
	NAV_CANT_ACCESS_FILE,
	NAV_INVALID_FILE,
	NAV_BAD_FILE_VERSION,
	NAV_FILE_OUT_OF_DATE,
	NAV_CORRUPT_DATA,
	NAV_OUT_OF_MEMORY,
};

enum NavAttributeType
{
	NAV_MESH_INVALID		= 0,
	NAV_MESH_CROUCH			= 0x00000001,				// must crouch to use this node/area
	NAV_MESH_JUMP			= 0x00000002,				// must jump to traverse this area (only used during generation)
	NAV_MESH_PRECISE		= 0x00000004,				// do not adjust for obstacles, just move along area
	NAV_MESH_NO_JUMP		= 0x00000008,				// inhibit discontinuity jumping
	NAV_MESH_STOP			= 0x00000010,				// must stop when entering this area
	NAV_MESH_RUN			= 0x00000020,				// must run to traverse this area
	NAV_MESH_WALK			= 0x00000040,				// must walk to traverse this area
	NAV_MESH_AVOID			= 0x00000080,				// avoid this area unless alternatives are too dangerous
	NAV_MESH_TRANSIENT		= 0x00000100,				// area may become blocked, and should be periodically checked
	NAV_MESH_DONT_HIDE		= 0x00000200,				// area should not be considered for hiding spot generation
	NAV_MESH_STAND			= 0x00000400,				// bots hiding in this area should stand
	NAV_MESH_NO_HOSTAGES	= 0x00000800,				// hostages shouldn't use this area
	NAV_MESH_STAIRS			= 0x00001000,				// this area represents stairs, do not attempt to climb or jump them - just walk up
	NAV_MESH_NO_MERGE		= 0x00002000,				// don't merge this area with adjacent areas
	NAV_MESH_OBSTACLE_TOP	= 0x00004000,				// this nav area is the climb point on the tip of an obstacle
	NAV_MESH_CLIFF			= 0x00008000,				// this nav area is adjacent to a drop of at least CliffHeight

	NAV_MESH_FIRST_CUSTOM	= 0x00010000,				// apps may define custom app-specific bits starting with this value
	NAV_MESH_LAST_CUSTOM	= 0x04000000,				// apps must not define custom app-specific bits higher than with this value

	NAV_MESH_FUNC_COST		= 0x20000000,				// area has designer specified cost controlled by func_nav_cost entities
	NAV_MESH_HAS_ELEVATOR	= 0x40000000,				// area is in an elevator's path
	NAV_MESH_NAV_BLOCKER	= 0x80000000				// area is blocked by nav blocker ( Alas, needed to hijack a bit in the attributes to get within a cache line [7/24/2008 tom])
};

/**
 * Defines possible ways to move from one area to another
 */
enum NavTraverseType
{
	// NOTE: First 4 directions MUST match Nav_DirType
	GO_NORTH = 0,
	GO_EAST,
	GO_SOUTH,
	GO_WEST,

	GO_LADDER_UP,
	GO_LADDER_DOWN,
	GO_JUMP,
	GO_ELEVATOR_UP,
	GO_ELEVATOR_DOWN,

	NUM_TRAVERSE_TYPES
};

enum NavCornerType
{
	NORTH_WEST = 0,
	NORTH_EAST = 1,
	SOUTH_EAST = 2,
	SOUTH_WEST = 3,

	NUM_CORNERS
};

enum NavRelativeDirType
{
	FORWARD = 0,
	RIGHT,
	BACKWARD,
	LEFT,
	UP,
	DOWN,

	NUM_RELATIVE_DIRECTIONS
};

struct Extent
{
	Vector lo, hi;

	void Init( void )
	{
		lo.Init();
		hi.Init();
	}

	void Init( IBaseEntity *entity )
	{
		entity->CollisionProp()->WorldSpaceSurroundingBounds( &lo, &hi );
	}

	float SizeX( void ) const	{ return hi.x - lo.x; }
	float SizeY( void ) const	{ return hi.y - lo.y; }
	float SizeZ( void ) const	{ return hi.z - lo.z; }
	float Area( void ) const	{ return SizeX() * SizeY(); }

	// Increase bounds to contain the given point
	void Encompass( const Vector &pos )
	{
		for ( int i=0; i<3; ++i )
		{
			if ( pos[i] < lo[i] )
			{
				lo[i] = pos[i];
			}
			else if ( pos[i] > hi[i] )
			{
				hi[i] = pos[i];
			}
		}
	}

	// Increase bounds to contain the given extent
	void Encompass( const Extent &extent )
	{
		Encompass( extent.lo );
		Encompass( extent.hi );
	}

	// return true if 'pos' is inside of this extent
	bool Contains( const Vector &pos ) const
	{
		return (pos.x >= lo.x && pos.x <= hi.x &&
				pos.y >= lo.y && pos.y <= hi.y &&
				pos.z >= lo.z && pos.z <= hi.z);
	}
	
	// return true if this extent overlaps the given one
	bool IsOverlapping( const Extent &other ) const
	{
		return (lo.x <= other.hi.x && hi.x >= other.lo.x &&
				lo.y <= other.hi.y && hi.y >= other.lo.y &&
				lo.z <= other.hi.z && hi.z >= other.lo.z);
	}

	// return true if this extent completely contains the given one
	bool IsEncompassing( const Extent &other, float tolerance = 0.0f ) const
	{
		return (lo.x <= other.lo.x + tolerance && hi.x >= other.hi.x - tolerance &&
				lo.y <= other.lo.y + tolerance && hi.y >= other.hi.y - tolerance &&
				lo.z <= other.lo.z + tolerance && hi.z >= other.hi.z - tolerance);
	}
};

struct Ray
{
	Vector from, to;
};

class CNavArea;
class CNavNode;


//--------------------------------------------------------------------------------------------------------------
inline Nav_DirType OppositeDirection( Nav_DirType dir )
{
	switch( dir )
	{
		case NORTH: return SOUTH;
		case SOUTH: return NORTH;
		case EAST:	return WEST;
		case WEST:	return EAST;
		default: break;
	}

	return NORTH;
}

//--------------------------------------------------------------------------------------------------------------
inline Nav_DirType DirectionLeft( Nav_DirType dir )
{
	switch( dir )
	{
		case NORTH: return WEST;
		case SOUTH: return EAST;
		case EAST:	return NORTH;
		case WEST:	return SOUTH;
		default: break;
	}

	return NORTH;
}

//--------------------------------------------------------------------------------------------------------------
inline Nav_DirType DirectionRight( Nav_DirType dir )
{
	switch( dir )
	{
		case NORTH: return EAST;
		case SOUTH: return WEST;
		case EAST:	return SOUTH;
		case WEST:	return NORTH;
		default: break;
	}

	return NORTH;
}

//--------------------------------------------------------------------------------------------------------------
inline void AddDirectionVector( Vector *v, Nav_DirType dir, float amount )
{
	switch( dir )
	{
		case NORTH: v->y -= amount; return;
		case SOUTH: v->y += amount; return;
		case EAST:  v->x += amount; return;
		case WEST:  v->x -= amount; return;
		default: break;
	}
}

//--------------------------------------------------------------------------------------------------------------
inline float DirectionToAngle( Nav_DirType dir )
{
	switch( dir )
	{
		case NORTH:	return 270.0f;
		case SOUTH:	return 90.0f;
		case EAST:	return 0.0f;
		case WEST:	return 180.0f;
		default: break;
	}

	return 0.0f;
}

//--------------------------------------------------------------------------------------------------------------
inline void DirectionToVector2D( Nav_DirType dir, Vector2D *v )
{
	switch( dir )
	{
		case NORTH: v->x =  0.0f; v->y = -1.0f; break;
		case SOUTH: v->x =  0.0f; v->y =  1.0f; break;
		case EAST:  v->x =  1.0f; v->y =  0.0f; break;
		case WEST:  v->x = -1.0f; v->y =  0.0f; break;
		default: break;
	}
}


//--------------------------------------------------------------------------------------------------------------
inline void CornerToVector2D( NavCornerType dir, Vector2D *v )
{
	switch( dir )
	{
		case NORTH_WEST: v->x = -1.0f; v->y = -1.0f; break;
		case NORTH_EAST: v->x =  1.0f; v->y = -1.0f; break;
		case SOUTH_EAST: v->x =  1.0f; v->y =  1.0f; break;
		case SOUTH_WEST: v->x = -1.0f; v->y =  1.0f; break;
		default: break;
	}

	v->NormalizeInPlace();
}

class CUtlVectorUltraConservativeAllocator
{
public:
	static void *Alloc( size_t nSize )
	{
		return malloc( nSize );
	}

	static void *Realloc( void *pMem, size_t nSize )
	{
		return realloc( pMem, nSize );
	}

	static void Free( void *pMem )
	{
		free( pMem );
	}

	static size_t GetSize( void *pMem )
	{
		return malloc_usable_size( pMem );
	}

};
typedef CUtlVectorUltraConservativeAllocator CNavVectorAllocator;

template <typename T, typename A = CUtlVectorUltraConservativeAllocator >
class CUtlVectorUltraConservative : private A
{
public:
	// Don't inherit from base_vector_t because multiple-inheritance increases
	// class size!
	enum { IsUtlVector = true }; // Used to match this at compiletime 		

	CUtlVectorUltraConservative()
	{
		m_pData = StaticData();
	}

	~CUtlVectorUltraConservative()
	{
		RemoveAll();
	}

	int Count() const
	{
		return m_pData->m_Size;
	}

	static int InvalidIndex()
	{
		return -1;
	}

	inline bool IsValidIndex( int i ) const
	{
		return (i >= 0) && (i < Count());
	}

	T& operator[]( int i )
	{
		Assert( IsValidIndex( i ) );
		return m_pData->m_Elements[i];
	}

	const T& operator[]( int i ) const
	{
		Assert( IsValidIndex( i ) );
		return m_pData->m_Elements[i];
	}

	T& Element( int i )
	{
		Assert( IsValidIndex( i ) );
		return m_pData->m_Elements[i];
	}

	const T& Element( int i ) const
	{
		Assert( IsValidIndex( i ) );
		return m_pData->m_Elements[i];
	}

	void EnsureCapacity( int num )
	{
		int nCurCount = Count();
		if ( num <= nCurCount )
		{
			return;
		}
		if ( m_pData == StaticData() )
		{
			m_pData = (Data_t *)A::Alloc( sizeof(Data_t) + ( num * sizeof(T) ) );
			m_pData->m_Size = 0;
		}
		else
		{
			int nNeeded = sizeof(Data_t) + ( num * sizeof(T) );
			int nHave = A::GetSize( m_pData );
			if ( nNeeded > nHave )
			{
				m_pData = (Data_t *)A::Realloc( m_pData, nNeeded );
			}
		}
	}

	int AddToTail( const T& src )
	{
		int iNew = Count();
		EnsureCapacity( Count() + 1 );
		m_pData->m_Elements[iNew] = src;
		m_pData->m_Size++;
		return iNew;
	}

	void RemoveAll()
	{
		if ( Count() )
		{
			for (int i = m_pData->m_Size; --i >= 0; )
			{
				::Destruct(&m_pData->m_Elements[i]);
			}
		}
		if ( m_pData != StaticData() )
		{
			A::Free( m_pData );
			m_pData = StaticData();

		}
	}

	void PurgeAndDeleteElements()
	{
		if ( m_pData != StaticData() )
		{
			for( int i=0; i < m_pData->m_Size; i++ )
			{
				delete Element(i);
			}
			RemoveAll();
		}
	}

	void FastRemove( int elem )
	{
		Assert( IsValidIndex(elem) );

		::Destruct( &Element(elem) );
		if (Count() > 0)
		{
			if ( elem != m_pData->m_Size -1 )
				memcpy( &Element(elem), &Element(m_pData->m_Size-1), sizeof(T) );
			--m_pData->m_Size;
		}
		if ( !m_pData->m_Size )
		{
			A::Free( m_pData );
			m_pData = StaticData();
		}
	}

	void Remove( int elem )
	{
		::Destruct( &Element(elem) );
		ShiftElementsLeft(elem);
		--m_pData->m_Size;
		if ( !m_pData->m_Size )
		{
			A::Free( m_pData );
			m_pData = StaticData();
		}
	}

	int Find( const T& src ) const
	{
		int nCount = Count();
		for ( int i = 0; i < nCount; ++i )
		{
			if (Element(i) == src)
				return i;
		}
		return -1;
	}

	bool FindAndRemove( const T& src )
	{
		int elem = Find( src );
		if ( elem != -1 )
		{
			Remove( elem );
			return true;
		}
		return false;
	}


	bool FindAndFastRemove( const T& src )
	{
		int elem = Find( src );
		if ( elem != -1 )
		{
			FastRemove( elem );
			return true;
		}
		return false;
	}

	bool DebugCompileError_ANonVectorIsUsedInThe_FOR_EACH_VEC_Macro( void ) const { return true; }

	struct Data_t
	{
		int m_Size;
		T m_Elements[0];
	};

	Data_t *m_pData;
private:
	void ShiftElementsLeft( int elem, int num = 1 )
	{
		int Size = Count();
		Assert( IsValidIndex(elem) || ( Size == 0 ) || ( num == 0 ));
		int numToMove = Size - elem - num;
		if ((numToMove > 0) && (num > 0))
		{
			Q_memmove( &Element(elem), &Element(elem+num), numToMove * sizeof(T) );
		}
	}

	static Data_t *StaticData()
	{
		static Data_t staticData;
		Assert( staticData.m_Size == 0 );
		return &staticData;
	}
};


union NavLadderConnect
{
	unsigned int id;
	INavLadder *ladder;

	bool operator==( const NavLadderConnect &other ) const
	{
		return (ladder == other.ladder) ? true : false;
	}
};
typedef CUtlVectorUltraConservative<NavLadderConnect, CNavVectorAllocator> NavLadderConnectVector;

struct NavConnect
{
	NavConnect()
	{
		id = 0;
		length = -1;
	}

	union
	{
		unsigned int id;
		INavArea *area;
	};

	mutable float length;

	bool operator==( const NavConnect &other ) const
	{
		return (area == other.area) ? true : false;
	}
};

typedef CUtlVectorUltraConservative<NavConnect, CNavVectorAllocator> NavConnectVector;

void ChangeMasterMarker( void );

class HidingSpot
{
public:
	virtual ~HidingSpot()	{ }

	enum 
	{ 
		IN_COVER			= 0x01,							// in a corner with good hard cover nearby
		GOOD_SNIPER_SPOT	= 0x02,							// had at least one decent sniping corridor
		IDEAL_SNIPER_SPOT	= 0x04,							// can see either very far, or a large area, or both
		EXPOSED				= 0x08							// spot in the open, usually on a ledge or cliff
	};

	bool HasGoodCover( void ) const			{ return (m_flags & IN_COVER) ? true : false; }	// return true if hiding spot in in cover
	bool IsGoodSniperSpot( void ) const		{ return (m_flags & GOOD_SNIPER_SPOT) ? true : false; }
	bool IsIdealSniperSpot( void ) const	{ return (m_flags & IDEAL_SNIPER_SPOT) ? true : false; }
	bool IsExposed( void ) const			{ return (m_flags & EXPOSED) ? true : false; }	

	int GetFlags( void ) const		{ return m_flags; }

	void Save( CUtlBuffer &fileBuffer, unsigned int version ) const;
	void Load( CUtlBuffer &fileBuffer, unsigned int version );
	NavErrorType PostLoad( void );

	const Vector &GetPosition( void ) const		{ return m_pos; }	// get the position of the hiding spot
	unsigned int GetID( void ) const			{ return m_id; }
	const CNavArea *GetArea( void ) const		{ return m_area; }	// return nav area this hiding spot is within

	void Mark( void );
	bool IsMarked( void ) const;


public:
	void SetFlags( int flags )				{ m_flags |= flags; }	// FOR INTERNAL USE ONLY
	void SetPosition( const Vector &pos )	{ m_pos = pos; }		// FOR INTERNAL USE ONLY

private:
	friend class CNavMesh;
	friend void ClassifySniperSpot( HidingSpot *spot );

	HidingSpot( void );										// must use factory to create

	Vector m_pos;											// world coordinates of the spot
	unsigned int m_id;										// this spot's unique ID
	unsigned int m_marker;									// this spot's unique marker
	CNavArea *m_area;										// the nav area containing this hiding spot

	unsigned char m_flags;									// bit flags

	//static unsigned int m_nextID;							// used when allocating spot ID's
	//static unsigned int m_masterMarker;						// used to mark spots
};
typedef CUtlVectorUltraConservative< HidingSpot * > HidingSpotVector;

struct SpotOrder
{
	float t;						// parametric distance along ray where this spot first has LOS to our path
	union
	{
		HidingSpot *spot;			// the spot to look at
		unsigned int id;			// spot ID for save/load
	};
};
typedef CUtlVector< SpotOrder > SpotOrderVector;


struct SpotEncounter
{
	NavConnect from;
	Nav_DirType fromDir;
	NavConnect to;
	Nav_DirType toDir;
	Ray path;									// the path segment
	SpotOrderVector spots;						// list of spots to look at, in order of occurrence
};
typedef CUtlVectorUltraConservative< SpotEncounter * > SpotEncounterVector;


class CNavLadder;
class CFuncElevator;
class CFuncNavPrerequisite;
class CNavNode;

class CNavAreaCriticalData
{
protected:
	// --- Begin critical data, which is heavily hit during pathing operations and carefully arranged for cache performance [7/24/2008 tom] ---

	/* 4  */	Vector m_nwCorner;											// north-west corner position (2D mins)
	/* 16 */	Vector m_seCorner;											// south-east corner position (2D maxs)
	/* 28 */	float m_invDxCorners;
	/* 32 */	float m_invDyCorners;
	/* 36 */	float m_neZ;												// height of the implicit corner defined by (m_seCorner.x, m_nwCorner.y, m_neZ)
	/* 40 */	float m_swZ;												// height of the implicit corner defined by (m_nwCorner.x, m_seCorner.y, m_neZ)
	/* 44 */	Vector m_center;											// centroid of area

	/* 54 */	unsigned char m_playerCount[ 2 ];				            // the number of players currently in this area

	/* 56 */	bool m_isBlocked[ 2 ];							            // if true, some part of the world is preventing movement through this nav area

	/* 58 */	unsigned int m_marker;										// used to flag the area as visited
	/* 62 */	float m_totalCost;											// the distance so far plus an estimate of the distance left
	/* 66 */	float m_costSoFar;											// distance travelled so far

	/* 70 */	CNavArea *m_nextOpen, *m_prevOpen;							// only valid if m_openMarker == m_masterMarker
	/* 78 */	unsigned int m_openMarker;									// if this equals the current marker value, we are on the open list

	/* 82 */	int	m_attributeFlags;										// set of attribute bit flags (see NavAttributeType)

	//- connections to adjacent areas -------------------------------------------------------------------
	/* 86 */	NavConnectVector m_connect[ NUM_DIRECTIONS ];				// a list of adjacent areas for each direction
	/* 104*/	NavLadderConnectVector m_ladder[ /*CNavLadder::NUM_LADDER_DIRECTIONS*/ 2 ];	// list of ladders leading up and down from this area
	/* 112*/	NavConnectVector m_elevatorAreas;							// a list of areas reachable via elevator from this area

	/* 116*/	unsigned int m_nearNavSearchMarker;							// used in GetNearestNavArea()

	/* 120*/	CNavArea *m_parent;											// the area just prior to this on in the search path
	/* 124*/	NavTraverseType m_parentHow;								// how we get from parent to us

	/* 128*/	float m_pathLengthSoFar;									// length of path so far, needed for limiting pathfind max path length

	/* *************** 360 cache line *************** */

	/* 132*/	CFuncElevator *m_elevator;									// if non-NULL, this area is in an elevator's path. The elevator can transport us vertically to another area.

	// --- End critical data --- 
};

void MakeNewMarker( void );
// void ClearSearchLists( void );
// bool IsOpenListEmpty( void );

class INavArea : protected CNavAreaCriticalData
{
public:
	virtual ~INavArea() {}
	
	virtual void OnServerActivate( void ) = 0;						// (EXTEND) invoked when map is initially loaded
	virtual void OnRoundRestart( void ) = 0;						// (EXTEND) invoked for each area when the round restarts
	virtual void OnRoundRestartPreEntity( void )  = 0;			// invoked for each area when the round restarts, but before entities are deleted and recreated
	virtual void OnEnter( CBaseCombatCharacter *who, CNavArea *areaJustLeft ) = 0;	// invoked when player enters this area 
	virtual void OnExit( CBaseCombatCharacter *who, CNavArea *areaJustEntered ) = 0;	// invoked when player exits this area 

	virtual void OnDestroyNotify( CNavArea *dead ) = 0;				// invoked when given area is going away
	virtual void OnDestroyNotify( CNavLadder *dead ) = 0;			// invoked when given ladder is going away

	virtual void OnEditCreateNotify( CNavArea *newArea ) = 0;		// invoked when given area has just been added to the mesh in edit mode
	virtual void OnEditDestroyNotify( CNavArea *deadArea ) = 0;		// invoked when given area has just been deleted from the mesh in edit mode
	virtual void OnEditDestroyNotify( CNavLadder *deadLadder ) = 0;	// invoked when given ladder has just been deleted from the mesh in edit mode

	virtual void Save( CUtlBuffer &fileBuffer, unsigned int version ) const  = 0;	// (EXTEND)
	virtual NavErrorType Load( CUtlBuffer &fileBuffer, unsigned int version, unsigned int subVersion )  = 0;		// (EXTEND)
	virtual NavErrorType PostLoad( void ) = 0;							// (EXTEND) invoked after all areas have been loaded - for pointer binding, etc

	virtual void SaveToSelectedSet( KeyValues *areaKey ) const = 0;		// (EXTEND) saves attributes for the area to a KeyValues
	virtual void RestoreFromSelectedSet( KeyValues *areaKey ) = 0;		// (EXTEND) restores attributes from a KeyValues
/*
	// for interactively building or generating nav areas
	void Build( CNavNode *nwNode, CNavNode *neNode, CNavNode *seNode, CNavNode *swNode );
	void Build( const Vector &corner, const Vector &otherCorner );
	void Build( const Vector &nwCorner, const Vector &neCorner, const Vector &seCorner, const Vector &swCorner );

	void ConnectTo( CNavArea *area, Nav_DirType dir );			// connect this area to given area in given direction
	void Disconnect( CNavArea *area );							// disconnect this area from given area

	void ConnectTo( CNavLadder *ladder );						// connect this area to given ladder
	void Disconnect( CNavLadder *ladder );						// disconnect this area from given ladder
*/
	unsigned int GetID( void ) const	{ return m_id; }		// return this area's unique ID
	//static void CompressIDs( void );							// re-orders area ID's so they are continuous
	unsigned int GetDebugID( void ) const { return m_debugid; }


	void SetNavSearchMarker(unsigned int value)	{ m_nearNavSearchMarker = value;}
	unsigned int GetNavSearchMarker(void) { return m_nearNavSearchMarker; }
	void SetAttributes( int bits )			{ m_attributeFlags = bits; }
	int GetAttributes( void ) const			{ return m_attributeFlags; }
	bool HasAttributes( int bits ) const	{ return ( m_attributeFlags & bits ) ? true : false; }
	bool HasSpawnAttributes( int flags );
	void RemoveAttributes( int bits )		{ m_attributeFlags &= ( ~bits ); }

	void Reset() 							{ m_prevOpen = nullptr; m_nextOpen = nullptr; }

	void SetPlace( Place place )		{ m_place = place; }	// set place descriptor
	Place GetPlace( void ) const		{ return m_place; }		// get place descriptor

	//void MarkAsBlocked( int teamID, CBaseEntity *blocker, bool bGenerateEvent = true );	// An entity can force a nav area to be blocked
	virtual void UpdateBlocked( bool force = false, int teamID = -1 )  = 0;		// Updates the (un)blocked status of the nav area (throttled)
	bool IsBlocked( int teamID, bool ignoreNavBlockers = false ) const;
/*	void UnblockArea( int teamID = -1 );					// clear blocked status for the given team(s)

	void CheckFloor( CBaseEntity *ignore );						// Checks if there is a floor under the nav area, in case a breakable floor is gone

	void MarkObstacleToAvoid( float obstructionHeight );
	void UpdateAvoidanceObstacles( void );
	bool HasAvoidanceObstacle( float maxObstructionHeight = StepHeight ) const; // is there a large, immobile object obstructing this area
	float GetAvoidanceObstacleHeight( void ) const; // returns the maximum height of the obstruction above the ground

	bool HasPrerequisite( CBaseCombatCharacter *actor = NULL ) const;							// return true if this area has a prerequisite that applies to the given actor
	const CUtlVector< CHandle< CFuncNavPrerequisite > > &GetPrerequisiteVector( void ) const;	// return vector of prerequisites that must be met before this area can be traversed
	void RemoveAllPrerequisites( void );
	void AddPrerequisite( CFuncNavPrerequisite *prereq );

	void ClearAllNavCostEntities( void );							// clear set of func_nav_cost entities that affect this area
	void AddFuncNavCostEntity( CFuncNavCost *cost );				// add the given func_nav_cost entity to the cost of this area
	float ComputeFuncNavCost( CBaseCombatCharacter *who ) const;	// return the cost multiplier of this area's func_nav_cost entities for the given actor
	bool HasFuncNavAvoid( void ) const;
	bool HasFuncNavPrefer( void ) const;
*/
	void CheckWaterLevel( void );
	bool IsUnderwater( void ) const		{ return m_isUnderwater; }

	bool IsOverlapping( const Vector &pos, float tolerance = 0.0f ) const;	// return true if 'pos' is within 2D extents of area.
	bool IsOverlapping( const INavArea *area ) const;			// return true if 'area' overlaps our 2D extents
	bool IsOverlapping( const Extent &extent ) const;			// return true if 'extent' overlaps our 2D extents
	bool IsOverlappingX( const INavArea *area ) const;			// return true if 'area' overlaps our X extent
	bool IsOverlappingY( const INavArea *area ) const;			// return true if 'area' overlaps our Y extent
	inline float GetZ( const Vector * pPos ) const ;			// return Z of area at (x,y) of 'pos'
	inline float GetZ( const Vector &pos ) const;						// return Z of area at (x,y) of 'pos'
	float GetZ( float x, float y ) const;				// return Z of area at (x,y) of 'pos'
	bool Contains( const Vector &pos ) const;					// return true if given point is on or above this area, but no others
	bool Contains( const INavArea *area ) const;	
	//bool IsCoplanar( const CNavArea *area ) const;				// return true if this area and given area are approximately co-planar
	void GetClosestPointOnArea( const Vector *pPos, Vector *close ) const;	// return closest point to 'pos' on this area - returned point in 'close'
	void GetClosestPointOnArea( const Vector &pos, Vector *close ) const { return GetClosestPointOnArea( &pos, close ); }
	float GetDistanceSquaredToPoint( const Vector &pos ) const;	// return shortest distance between point and this area
	bool IsDegenerate( void ) const;							// return true if this area is badly formed
	bool IsRoughlySquare( void ) const;							// return true if this area is approximately square
	//bool IsFlat( void ) const;									// return true if this area is approximately flat
	bool HasNodes( void ) const;
	//void GetNodes( Nav_DirType dir, CUtlVector< CNavNode * > *nodes ) const;	// build a vector of nodes along the given direction
	//CNavNode *FindClosestNode( const Vector &pos, Nav_DirType dir ) const;	// returns the closest node along the given edge to the given point

	bool IsContiguous( const INavArea *other ) const;			// return true if the given area and 'other' share a colinear edge (ie: no drop-down or step/jump/climb)
	float ComputeAdjacentConnectionHeightChange( const INavArea *destinationArea ) const;			// return height change between edges of adjacent nav areas (not actual underlying ground)

	bool IsEdge( Nav_DirType dir ) const;						// return true if there are no bi-directional links on the given side

	bool IsDamaging( void ) const;								// Return true if continuous damage (ie: fire) is in this area
	void MarkAsDamaging( float duration );						// Mark this area is damaging for the next 'duration' seconds

	bool IsVisible( const Vector &eye, Vector *visSpot = NULL ) const;	// return true if area is visible from the given eyepoint, return visible spot

	int GetAdjacentCount( Nav_DirType dir ) const	{ return m_connect[ dir ].Count(); }	// return number of connected areas in given direction
	INavArea *GetAdjacentArea( Nav_DirType dir, int i ) const;	// return the i'th adjacent area in the given direction
	INavArea *GetRandomAdjacentArea( Nav_DirType dir ) const;
	void CollectAdjacentAreas( CUtlVector< INavArea * > *adjVector ) const;	// build a vector of all adjacent areas

	const NavConnectVector *GetAdjacentAreas( Nav_DirType dir ) const	{ return &m_connect[dir]; }
	bool IsConnected( const INavArea *area, Nav_DirType dir ) const;	// return true if given area is connected in given direction
	bool IsConnected( const INavLadder *ladder, Ladder_DirectionType dir ) const;	// return true if given ladder is connected in given direction
/*	float ComputeGroundHeightChange( const CNavArea *area );			// compute change in actual ground height from this area to given area
*/
	const NavConnectVector *GetIncomingConnections( Nav_DirType dir ) const	{ return &m_incomingConnect[dir]; }	// get areas connected TO this area by a ONE-WAY link (ie: we have no connection back to them)
	//void AddIncomingConnection( CNavArea *source, Nav_DirType incomingEdgeDir );

	const NavLadderConnectVector *GetLadders( Ladder_DirectionType dir ) const	{ return &m_ladder[dir]; }
	CFuncElevator *GetElevator( void ) const												{ Assert( !( m_attributeFlags & NAV_MESH_HAS_ELEVATOR ) == (m_elevator == NULL) ); return ( m_attributeFlags & NAV_MESH_HAS_ELEVATOR ) ? m_elevator : NULL; }
	const NavConnectVector &GetElevatorAreas( void ) const									{ return m_elevatorAreas; }	// return collection of areas reachable via elevator from this area

	void ComputePortal( const INavArea *to, Nav_DirType dir, Vector *center, float *halfWidth ) const;		// compute portal to adjacent area
	//Nav_DirType ComputeLargestPortal( const CNavArea *to, Vector *center, float *halfWidth ) const;		// compute largest portal to adjacent area, returning direction
	void ComputeClosestPointInPortal( const INavArea *to, Nav_DirType dir, const Vector &fromPos, Vector *closePos ) const; // compute closest point within the "portal" between to adjacent areas
/*	Nav_DirType ComputeDirection( Vector *point ) const;			// return direction from this area to the given point

	//- for hunting algorithm ---------------------------------------------------------------------------
	void SetClearedTimestamp( int teamID );						// set this area's "clear" timestamp to now
	float GetClearedTimestamp( int teamID ) const;*/				// get time this area was marked "clear"

	//- hiding spots ------------------------------------------------------------------------------------
	const HidingSpotVector *GetHidingSpots( void ) const	{ return &m_hidingSpots; }

	/*SpotEncounter *GetSpotEncounter( const CNavArea *from, const CNavArea *to );	// given the areas we are moving between, return the spots we will encounter
	int GetSpotEncounterCount( void ) const				{ return m_spotEncounters.Count(); }

	//- "danger" ----------------------------------------------------------------------------------------
	void IncreaseDanger( int teamID, float amount );			// increase the danger of this area for the given team
*/	float GetDanger( int teamID );								// return the danger of this area (decays over time)
	virtual float GetDangerDecayRate( void ) const = 0;				// return danger decay rate per second

	//- extents -----------------------------------------------------------------------------------------
	float GetSizeX( void ) const			{ return m_seCorner.x - m_nwCorner.x; }
	float GetSizeY( void ) const			{ return m_seCorner.y - m_nwCorner.y; }
	void GetExtent( Extent *extent ) const;						// return a computed extent (XY is in m_nwCorner and m_seCorner, Z is computed)
	const Vector &GetCenter( void ) const	{ return m_center; }
	Vector GetRandomPoint( void ) const;
	Vector GetCorner( NavCornerType corner ) const;
	void SetCorner( NavCornerType corner, const Vector& newPosition );
	void ComputeNormal( Vector *normal, bool alternate = false ) const;	// Computes the area's normal based on m_nwCorner.  If 'alternate' is specified, m_seCorner is used instead.
/*	void RemoveOrthogonalConnections( Nav_DirType dir );

	//- occupy time ------------------------------------------------------------------------------------
	float GetEarliestOccupyTime( int teamID ) const;			// returns the minimum time for someone of the given team to reach this spot from their spawn
	bool IsBattlefront( void ) const	{ return m_isBattlefront; }	// true if this area is a "battlefront" - where rushing teams initially meet

	//- player counting --------------------------------------------------------------------------------
	void IncrementPlayerCount( int teamID, int entIndex );		// add one player to this area's count
	void DecrementPlayerCount( int teamID, int entIndex );		// subtract one player from this area's count*/
	unsigned char GetPlayerCount( int teamID = 0 ) const;		// return number of players of given team currently within this area (team of zero means any/all)

	//- lighting ----------------------------------------------------------------------------------------
	float GetLightIntensity( const Vector &pos ) const;			// returns a 0..1 light intensity for the given point
	float GetLightIntensity( float x, float y ) const;			// returns a 0..1 light intensity for the given point
	float GetLightIntensity( void ) const;						// returns a 0..1 light intensity averaged over the whole area

	//- A* pathfinding algorithm ------------------------------------------------------------------------
	// static void MakeNewMarker( void )	{ ++m_masterMarker; if (m_masterMarker == 0) m_masterMarker = 1; }
	void Mark( void );
	BOOL IsMarked( void ) const;
	
	void SetParent( CNavArea *parent, NavTraverseType how = NUM_TRAVERSE_TYPES )	{ m_parent = parent; m_parentHow = how; }
	INavArea *GetParent( void ) const	{ return (INavArea*)m_parent; }
	NavTraverseType GetParentHow( void ) const	{ return m_parentHow; }

	bool IsOpen( void ) const;									// true if on "open list"
	void AddToOpenList( void );									// add to open list in decreasing value order
	void AddToOpenListTail( void );								// add to tail of the open list
	void UpdateOnOpenList( void );								// a smaller value has been found, update this area on the open list
	void RemoveFromOpenList( void );
	static bool IsOpenListEmpty( void );
	static INavArea *PopOpenList( void );						// remove and return the first element of the open list													

	bool IsClosed( void ) const;								// true if on "closed list"
	void AddToClosedList( void );								// add to the closed list
	void RemoveFromClosedList( void );

	static void ClearSearchLists( void );						// clears the open and closed lists for a new search

	void SetTotalCost( float value )	{ m_totalCost = value; }
	float GetTotalCost( void ) const	{ return m_totalCost; }

	void SetCostSoFar( float value )	{ m_costSoFar = value; }
	float GetCostSoFar( void ) const	{ return m_costSoFar; }

	void SetPathLengthSoFar( float value )	{ m_pathLengthSoFar = value; }
	float GetPathLengthSoFar( void ) const	{ return m_pathLengthSoFar; }

	float GetZDeltaAtEdgeToArea(const INavArea* area);

	//- editing -----------------------------------------------------------------------------------------
	virtual void Draw( void ) const = 0;							// draw area for debugging & editing
	virtual void DrawFilled( int r, int g, int b, int a, float deltaT = 0.1f, bool noDepthTest = true, float margin = 5.0f ) const = 0;	// draw area as a filled rect of the given color
	virtual void DrawSelectedSet( const Vector &shift ) const = 0;	// draw this area as part of a selected set
/*	void DrawDragSelectionSet( Color &dragSelectionSetColor ) const;
	void DrawConnectedAreas( void ) const;
	void DrawHidingSpots( void ) const;
	bool SplitEdit( bool splitAlongX, float splitEdge, CNavArea **outAlpha = NULL, CNavArea **outBeta = NULL );	// split this area into two areas at the given edge
	bool MergeEdit( CNavArea *adj );							// merge this area and given adjacent area 
	bool SpliceEdit( CNavArea *other );							// create a new area between this area and given area 
	void RaiseCorner( NavCornerType corner, int amount, bool raiseAdjacentCorners = true );	// raise/lower a corner (or all corners if corner == NUM_CORNERS)
	void PlaceOnGround( NavCornerType corner, float inset = 0.0f );	// places a corner (or all corners if corner == NUM_CORNERS) on the ground
	NavCornerType GetCornerUnderCursor( void ) const;
	bool GetCornerHotspot( NavCornerType corner, Vector hotspot[NUM_CORNERS] ) const;	// returns true if the corner is under the cursor
	void Shift( const Vector &shift );							// shift the nav area

	//- ladders -----------------------------------------------------------------------------------------
	void AddLadderUp( CNavLadder *ladder );
	void AddLadderDown( CNavLadder *ladder );
*/
	//- generation and analysis -------------------------------------------------------------------------
	virtual void ComputeHidingSpots( void ) = 0;					// analyze local area neighborhood to find "hiding spots" in this area - for map learning
	virtual void ComputeSniperSpots( void ) = 0;					// analyze local area neighborhood to find "sniper spots" in this area - for map learning
	virtual void ComputeSpotEncounters( void ) = 0;				// compute spot encounter data - for map learning
	virtual void ComputeEarliestOccupyTimes( void ) = 0;
	virtual void CustomAnalysis( bool isIncremental = false )  = 0;	// for game-specific analysis
	virtual bool ComputeLighting( void ) = 0;						// compute 0..1 light intensity at corners and center (requires client via listenserver)
	//bool TestStairs( void );									// Test an area for being on stairs
	virtual bool IsAbleToMergeWith( CNavArea *other ) const = 0;

	virtual void InheritAttributes( CNavArea *first, CNavArea *second = NULL ) = 0;

	//- visibility -------------------------------------------------------------------------------------
	enum VisibilityType
	{
		NOT_VISIBLE				= 0x00,
		POTENTIALLY_VISIBLE		= 0x01,
		COMPLETELY_VISIBLE		= 0x02,
	};
/*
	VisibilityType ComputeVisibility( const CNavArea *area, bool isPVSValid, bool bCheckPVS = true, bool *pOutsidePVS = NULL ) const;	// do actual line-of-sight traces to determine if any part of given area is visible from this area
	void SetupPVS( void ) const;
	bool IsInPVS( void ) const;					// return true if this area is within the current PVS
*/
	struct AreaBindInfo							// for pointer loading and binding
	{
		union
		{
			INavArea *area;
			unsigned int id;
		};

		unsigned char attributes;				// VisibilityType

		bool operator==( const AreaBindInfo &other ) const
		{
			return ( area == other.area );
		}
	};
/*
	virtual bool IsEntirelyVisible( const Vector &eye, const CBaseEntity *ignore = NULL ) const;				// return true if entire area is visible from given eyepoint (CPU intensive)
	virtual bool IsPartiallyVisible( const Vector &eye, const CBaseEntity *ignore = NULL ) const;				// return true if any portion of the area is visible from given eyepoint (CPU intensive)
*/
	virtual bool IsPotentiallyVisible( const INavArea *area ) const;		// return true if given area is potentially visible from somewhere in this area (very fast)
	virtual bool IsPotentiallyVisibleToTeam( int team ) const;				// return true if any portion of this area is visible to anyone on the given team (very fast)
/*
	virtual bool IsCompletelyVisible( const CNavArea *area ) const;			// return true if given area is completely visible from somewhere in this area (very fast)
	virtual bool IsCompletelyVisibleToTeam( int team ) const;				// return true if given area is completely visible from somewhere in this area by someone on the team (very fast)
*/
	//-------------------------------------------------------------------------------------
	/**
	 * Apply the functor to all navigation areas that are potentially
	 * visible from this area.
	 */
/*	template < typename Functor >
	bool ForAllPotentiallyVisibleAreas( Functor &func )
	{
		int i;

		++s_nCurrVisTestCounter;

		for ( i=0; i<m_potentiallyVisibleAreas.Count(); ++i )
		{
			CNavArea *area = m_potentiallyVisibleAreas[i].area;
			if ( !area )
				continue;

			// If this assertion triggers, an area is in here twice!
			Assert( area->m_nVisTestCounter != s_nCurrVisTestCounter );
			area->m_nVisTestCounter = s_nCurrVisTestCounter;

			if ( m_potentiallyVisibleAreas[i].attributes == NOT_VISIBLE )
				continue;
			
			if ( func( area ) == false )
				return false;
		}

		// for each inherited area
		if ( !m_inheritVisibilityFrom.area )
			return true;

		CAreaBindInfoArray &inherited = m_inheritVisibilityFrom.area->m_potentiallyVisibleAreas;

		for ( i=0; i<inherited.Count(); ++i )
		{
			if ( !inherited[i].area )
				continue;

			// We may have visited this from m_potentiallyVisibleAreas
			if ( inherited[i].area->m_nVisTestCounter == s_nCurrVisTestCounter )
				continue;

			// Theoretically, this shouldn't matter. But, just in case!
			inherited[i].area->m_nVisTestCounter = s_nCurrVisTestCounter;

			if ( inherited[i].attributes == NOT_VISIBLE )
				continue;

			if ( func( inherited[i].area ) == false )
				return false;
		}

		return true;
	}
*/
	//-------------------------------------------------------------------------------------
	/**
	 * Apply the functor to all navigation areas that are
	 * completely visible from somewhere in this area.
	 */
/*	template < typename Functor >
	bool ForAllCompletelyVisibleAreas( Functor &func )
	{
		int i;

		++s_nCurrVisTestCounter;

		for ( i=0; i<m_potentiallyVisibleAreas.Count(); ++i )
		{
			CNavArea *area = m_potentiallyVisibleAreas[i].area;
			if ( !area )
				continue;

			// If this assertion triggers, an area is in here twice!
			Assert( area->m_nVisTestCounter != s_nCurrVisTestCounter );
			area->m_nVisTestCounter = s_nCurrVisTestCounter;

			if ( ( m_potentiallyVisibleAreas[i].attributes & COMPLETELY_VISIBLE ) == 0 )
				continue;

			if ( func( area ) == false )
				return false;
		}

		if ( !m_inheritVisibilityFrom.area )
			return true;

		// for each inherited area
		CAreaBindInfoArray &inherited = m_inheritVisibilityFrom.area->m_potentiallyVisibleAreas;

		for ( i=0; i<inherited.Count(); ++i )
		{
			if ( !inherited[i].area )
				continue;
			
			// We may have visited this from m_potentiallyVisibleAreas
			if ( inherited[i].area->m_nVisTestCounter == s_nCurrVisTestCounter )
				continue;

			// Theoretically, this shouldn't matter. But, just in case!
			inherited[i].area->m_nVisTestCounter = s_nCurrVisTestCounter;

			if ( ( inherited[i].attributes & COMPLETELY_VISIBLE ) == 0 )
				continue;

			if ( func( inherited[i].area ) == false )
				return false;
		}

		return true;
	}

*/

	Vector FindRandomSpot();
	inline INavArea* GetNextHash() { return m_nextHash; }


private:
/*	friend class CNavMesh;
	friend class CNavLadder;
	friend class CCSNavArea;									// allow CS load code to complete replace our default load behavior

	static bool m_isReset;										// if true, don't bother cleaning up in destructor since everything is going away
*/
	/*
	m_nwCorner
		nw           ne
		 +-----------+
		 | +-->x     |
		 | |         |
		 | v         |
		 | y         |
		 |           |
		 +-----------+
		sw           se
					m_seCorner
	*/

	//static unsigned int m_nextID;								// used to allocate unique IDs
	unsigned int m_id;											// 34 unique area ID
	unsigned int m_debugid;										// 35

	Place m_place;												// place descriptor

	CountdownTimers m_blockedTimer;								// Throttle checks on our blocked state while blocked
	//void UpdateBlockedFromNavBlockers( void );					// checks if nav blockers are still blocking the area

	bool m_isUnderwater;										// true if the center of the area is underwater

	bool m_isBattlefront;

	float m_avoidanceObstacleHeight;							// if nonzero, a prop is obstructing movement through this nav area
	CountdownTimers m_avoidanceObstacleTimer;					// Throttle checks on our obstructed state while obstructed

	//- for hunting -------------------------------------------------------------------------------------
	float m_clearedTimestamp[ 2 ];								// time this area was last "cleared" of enemies

	//- "danger" ----------------------------------------------------------------------------------------
	float m_danger[ 2 ];										// danger of this area, allowing bots to avoid areas where they died in the past - zero is no danger
	float m_dangerTimestamp[ 2 ];								// time when danger value was set - used for decaying
	void DecayDanger( void );

	//- hiding spots ------------------------------------------------------------------------------------
	HidingSpotVector m_hidingSpots;
	//bool IsHidingSpotCollision( const Vector &pos ) const;	// returns true if an existing hiding spot is too close to given position

	//- encounter spots ---------------------------------------------------------------------------------
	SpotEncounterVector m_spotEncounters;						// list of possible ways to move thru this area, and the spots to look at as we do
	//void AddSpotEncounters( const CNavArea *from, Nav_DirType fromDir, const CNavArea *to, Nav_DirType toDir );	// add spot encounter data when moving from area to area

	float m_earliestOccupyTime[ 2 ];				// min time to reach this spot from spawn

	//- lighting ----------------------------------------------------------------------------------------
	float m_lightIntensity[ NUM_CORNERS ];						// 55 56 57 58 0..1 light intensity at corners

	//- A* pathfinding algorithm ------------------------------------------------------------------------
	// static unsigned int m_masterMarker;

	static INavArea *m_openList;
	static INavArea *m_openListTail;

	//- connections to adjacent areas -------------------------------------------------------------------
	NavConnectVector m_incomingConnect[ NUM_DIRECTIONS ];		// a list of adjacent areas for each direction that connect TO us, but we have no connection back to them

	//---------------------------------------------------------------------------------------------------
	CNavNode *m_node[ NUM_CORNERS ];							// 63 64 65 66 nav nodes at each corner of the area

	void ResetNodes( void );									// nodes are going away as part of an incremental nav generation
/*	void Strip( void );											// remove "analyzed" data from nav area

	void FinishMerge( CNavArea *adjArea );						// recompute internal data once nodes have been adjusted during merge
	void MergeAdjacentConnections( CNavArea *adjArea );			// for merging with "adjArea" - pick up all of "adjArea"s connections
	void AssignNodes( CNavArea *area );							// assign internal nodes to the given area

	void FinishSplitEdit( CNavArea *newArea, Nav_DirType ignoreEdge );	// given the portion of the original area, update its internal data
*/
	void CalcDebugID();

	//CUtlVector< CHandle< CFuncNavPrerequisite > > m_prerequisiteVector;		// list of prerequisites that must be met before this area can be traversed
	int iUnknown[4];

	INavArea *m_prevHash, *m_nextHash;							// 71 72 for hash table in CNavMesh

	//void ConnectElevators( void );								// find elevator connections between areas

	int m_damagingTickCount;									// 73 this area is damaging through this tick count


	//- visibility --------------------------------------------------------------------------------------
/*	void ComputeVisibilityToMesh( void );						// compute visibility to surrounding mesh
	void ResetPotentiallyVisibleAreas();
	static void ComputeVisToArea( CNavArea *&pOtherArea );
*/
#ifndef _X360
	typedef CUtlVectorConservative<AreaBindInfo> CAreaBindInfoArray; // shaves 8 bytes off structure caused by need to support editing
#else
	typedef CUtlVector<AreaBindInfo> CAreaBindInfoArray; // Need to use this on 360 to support external allocation pattern
#endif

	AreaBindInfo m_inheritVisibilityFrom;						// if non-NULL, m_potentiallyVisibleAreas becomes a list of additions and deletions (NOT_VISIBLE) to the list of this area
	CAreaBindInfoArray m_potentiallyVisibleAreas;				// list of areas potentially visible from inside this area (after PostLoad(), use area portion of union)
	bool m_isInheritedFrom;										// latch used during visibility inheritance computation
/*
	const CAreaBindInfoArray &ComputeVisibilityDelta( const CNavArea *other ) const;	// return a list of the delta between our visibility list and the given adjacent area

	uint32 m_nVisTestCounter;
	static uint32 s_nCurrVisTestCounter;

	CUtlVector< CHandle< CFuncNavCost > > m_funcNavCostVector;	// active, overlapping cost entities
	*/
};

// INavArea *PopOpenList( void );

inline float INavArea::GetZ(const Vector *pos) const
{
	return GetZ( pos->x, pos->y );
}

inline float INavArea::GetZ( const Vector & pos ) const
{
	return GetZ( pos.x, pos.y );
}

inline bool INavArea::IsDegenerate(void) const
{
    return (m_nwCorner.x >= m_seCorner.x || m_nwCorner.y >= m_seCorner.y);
}

inline INavArea *INavArea::GetAdjacentArea(Nav_DirType dir, int i) const
{
	if ( ( i < 0 ) || ( i >= m_connect[dir].Count() ) )
		return NULL;
	return m_connect[dir][i].area;
}

inline unsigned char INavArea::GetPlayerCount(int teamID) const
{
	if (teamID)
	{
		return m_playerCount[ teamID % 2 ];
	}

	// sum all players
	unsigned char total = 0;
	for( int i = 0; i < 2; ++i )
	{
		total += m_playerCount[i];
	}

	return total;
}

inline Vector INavArea::GetCorner(NavCornerType corner) const
{
	Vector pos;

	switch( corner )
	{
	default:
		Assert( false && "GetCorner: Invalid type" );
	case NORTH_WEST:
		return m_nwCorner;

	case NORTH_EAST:
		pos.x = m_seCorner.x;
		pos.y = m_nwCorner.y;
		pos.z = m_neZ;
		return pos;

	case SOUTH_WEST:
		pos.x = m_nwCorner.x;
		pos.y = m_seCorner.y;
		pos.z = m_swZ;
		return pos;

	case SOUTH_EAST:
		return m_seCorner;
	}
}

inline bool INavArea::IsDamaging( void ) const
{
	return ( g_pGlobals->tickcount <= m_damagingTickCount );
}

inline void INavArea::MarkAsDamaging( float duration )
{
	m_damagingTickCount = g_pGlobals->tickcount + ( (int)( 0.5f + (float)(duration) / (g_pGlobals->interval_per_tick) ) );
}

/** Функція з ITerrorNavArea */
inline bool INavArea::HasSpawnAttributes( int flags )
{
    int m_spawnAttributes = access_member<int>(this, 304);
    return (m_spawnAttributes & flags) ? true : false;
}

// inline bool INavArea::HasPrerequisite( CBaseCombatCharacter *actor ) const
// {
// 	return m_prerequisiteVector.Count() > 0;
// }

// //--------------------------------------------------------------------------------------------------------------
// inline const CUtlVector< CHandle< CFuncNavPrerequisite > > &INavArea::GetPrerequisiteVector( void ) const
// {
// 	return m_prerequisiteVector;
// }

// //--------------------------------------------------------------------------------------------------------------
// inline void INavArea::RemoveAllPrerequisites( void )
// {
// 	m_prerequisiteVector.RemoveAll();
// }

// //--------------------------------------------------------------------------------------------------------------
// inline void INavArea::AddPrerequisite( CFuncNavPrerequisite *prereq )
// {
// 	if ( m_prerequisiteVector.Find( prereq ) == m_prerequisiteVector.InvalidIndex() )
// 	{
// 		m_prerequisiteVector.AddToTail( prereq );
// 	}
// }


inline void AddAreaToOpenList( INavArea *area, INavArea *parent, const Vector &startPos, float maxRange )
{
	if (area == NULL)
		return;

	if (!area->IsMarked())
	{
		area->Mark();
		area->SetTotalCost( 0.0f );
		area->SetParent( (CNavArea *)parent );

		if (maxRange > 0.0f)
		{
			// make sure this area overlaps range
			Vector closePos;
			area->GetClosestPointOnArea( startPos, &closePos );
			if ((closePos - startPos).AsVector2D().IsLengthLessThan( maxRange ))
			{
				// compute approximate distance along path to limit travel range, too
				float distAlong = parent->GetCostSoFar();
				distAlong += (area->GetCenter() - parent->GetCenter()).Length();
				area->SetCostSoFar( distAlong );

				// allow for some fudge due to large size areas
				if (distAlong <= 1.5f * maxRange)
					area->AddToOpenList();
			}
		}
		else
		{
			// infinite range
			area->AddToOpenList();
		}
	}
}

#define INCLUDE_INCOMING_CONNECTIONS	0x1
#define INCLUDE_BLOCKED_AREAS			0x2
#define EXCLUDE_OUTGOING_CONNECTIONS	0x4
#define EXCLUDE_ELEVATORS				0x8
template < typename Functor >
void SearchSurroundingAreas( INavArea *startArea, const Vector &startPos, Functor &func, float maxRange = -1.0f, unsigned int options = 0, int teamID = -1 )
{
	if (startArea == NULL)
	{
		return;
	}

	MakeNewMarker();
	INavArea::ClearSearchLists();

	startArea->AddToOpenList();
	startArea->SetTotalCost( 0.0f );
	startArea->SetCostSoFar( 0.0f );
	startArea->SetParent( NULL );
	startArea->Mark();

	while( !INavArea::IsOpenListEmpty())
	{
		// get next area to check
		INavArea *area = INavArea::PopOpenList();

		// don't use blocked areas
		if ( area->IsBlocked( teamID ) && !(options & INCLUDE_BLOCKED_AREAS) )
			continue;

		// invoke functor on area
		if (func( area ))
		{
			// explore adjacent floor areas
			for( int dir=0; dir<NUM_DIRECTIONS; ++dir )
			{
				int count = area->GetAdjacentCount( (Nav_DirType)dir );
				for( int i=0; i<count; ++i )
				{
					INavArea *adjArea = area->GetAdjacentArea( (Nav_DirType)dir, i );
					if ( options & EXCLUDE_OUTGOING_CONNECTIONS )
					{
						if ( !adjArea->IsConnected( area, NUM_DIRECTIONS ) )
						{
							continue;	// skip this outgoing connection
						}
					}
					
					AddAreaToOpenList( adjArea, area, startPos, maxRange );
				}
			}
			
			// potentially include areas that connect TO this area via a one-way link
			if (options & INCLUDE_INCOMING_CONNECTIONS)
			{
				for( int dir=0; dir<NUM_DIRECTIONS; ++dir )
				{
					const NavConnectVector *list = area->GetIncomingConnections( (Nav_DirType)dir );

					FOR_EACH_VEC( (*list), it )
					{
						NavConnect connect = (*list)[ it ];				
						
						AddAreaToOpenList( connect.area, area, startPos, maxRange );
					}
				}
			}


			// explore adjacent areas connected by ladders

			// check up ladders
			const NavLadderConnectVector *ladderList = area->GetLadders( Ladder_DirectionType::LADDER_UP );
			if (ladderList)
			{
				_FOR_EACH_VEC_( (*ladderList), it )
				{
					const INavLadder *ladder = (*ladderList)[ it ].ladder;

					// do not use BEHIND connection, as its very hard to get to when going up a ladder
					AddAreaToOpenList( ladder->m_topForwardArea, area, startPos, maxRange );
					AddAreaToOpenList( ladder->m_topLeftArea, area, startPos, maxRange );
					AddAreaToOpenList( ladder->m_topRightArea, area, startPos, maxRange );
				}
			}

			// check down ladders
			ladderList = area->GetLadders( Ladder_DirectionType::LADDER_DOWN );
			if (ladderList)
			{
				_FOR_EACH_VEC_( (*ladderList), it )
				{
					const INavLadder *ladder = (*ladderList)[ it ].ladder;

					AddAreaToOpenList( ladder->m_bottomArea, area, startPos, maxRange );
				}
			}

			if((options & EXCLUDE_ELEVATORS) == 0)
			{
				const NavConnectVector &elevatorList = area->GetElevatorAreas();
				_FOR_EACH_VEC_(elevatorList, it)
				{
					INavArea* elevatorArea = elevatorList[it].area;
					AddAreaToOpenList(elevatorArea, area, startPos, maxRange);
				}
			}
		}
	}
}


#endif