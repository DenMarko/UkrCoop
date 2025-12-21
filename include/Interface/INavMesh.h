#ifndef _HEADER_NAV_MESH_INCLUDE_
#define _HEADER_NAV_MESH_INCLUDE_

#include "CGameEventListener.h"
#include "INavLadder.h"
#include "INavArea.h"
#include "vprof.h"

class CNavArea;
class HidingSpot;
class CNavLadder;
class CNavNode;
class CCommand;

enum GetNavAreaFlags_t
{
	GETNAVAREA_CHECK_LOS			= 0x1,
	GETNAVAREA_ALLOW_BLOCKED_AREAS	= 0x2,
	GETNAVAREA_CHECK_GROUND			= 0x4,
};

class INavMesh : public CGameEventListeners
{
public:
    virtual ~INavMesh() {}
	virtual void PreLoadAreas( int nAreas ) = 0;
	virtual CNavArea *CreateArea( void ) const = 0;							// CNavArea factory
	virtual void DestroyArea( CNavArea * ) const = 0;
	virtual HidingSpot *CreateHidingSpot( void ) const = 0;				// Hiding Spot factory

	virtual void Reset( void ) = 0;											// destroy Navigation Mesh data and revert to initial state
	virtual void Update( void ) = 0;										// invoked on each game frame

	virtual void FireGameEvent( IGameEvent *event ) = 0;					// incoming event processing

	virtual NavErrorType Load( void ) = 0;									// load navigation data from a file
	virtual NavErrorType PostLoad( unsigned int version ) = 0;				// (EXTEND) invoked after all areas have been loaded - for pointer binding, etc
	virtual bool Save( void ) const = 0;									// store Navigation Mesh to a file

	virtual unsigned int GetSubVersionNumber( void ) const = 0;										// returns sub-version number of data format used by derived classes
	virtual void SaveCustomData( CUtlBuffer &fileBuffer ) const = 0;								// store custom mesh data for derived classes
	virtual void LoadCustomData( CUtlBuffer &fileBuffer, unsigned int subVersion ) = 0;			// load custom mesh data for derived classes
	virtual void SaveCustomDataPreArea( CUtlBuffer &fileBuffer ) const = 0;						// store custom mesh data for derived classes that needs to be loaded before areas are read in
	virtual void LoadCustomDataPreArea( CUtlBuffer &fileBuffer, unsigned int subVersion ) = 0;	// load custom mesh data for derived classes that needs to be loaded before areas are read in

	virtual void OnServerActivate( void ) = 0;								// (EXTEND) invoked when server loads a new map
	virtual void OnRoundRestart( void ) = 0;								// invoked when a game round restarts
	virtual void OnRoundRestartPreEntity( void ) = 0;						// invoked when a game round restarts, but before entities are deleted and recreated
	virtual void OnBreakableCreated( CBaseEntity *breakable ) = 0;		// invoked when a breakable is created
	virtual void OnBreakableBroken( CBaseEntity *broken ) = 0;			// invoked when a breakable is broken
	virtual void OnAreaBlocked( CNavArea *area ) = 0;						// invoked when the area becomes blocked
	virtual void OnAreaUnblocked( CNavArea *area ) = 0;						// invoked when the area becomes un-blocked
	virtual void OnAvoidanceObstacleEnteredArea( CNavArea *area ) = 0;					// invoked when the area becomes obstructed
	virtual void OnAvoidanceObstacleLeftArea( CNavArea *area ) = 0;					// invoked when the area becomes un-obstructed

	virtual void OnEditCreateNotify( CNavArea *newArea ) = 0;				// invoked when given area has just been added to the mesh in edit mode
	virtual void OnEditDestroyNotify( CNavArea *deadArea ) = 0;				// invoked when given area has just been deleted from the mesh in edit mode
	virtual void OnEditDestroyNotify( CNavLadder *deadLadder ) = 0;			// invoked when given ladder has just been deleted from the mesh in edit mode
	virtual void OnNodeAdded( CNavNode *node ) = 0;						
	virtual void AddWalkableSeeds( void ) = 0;								// adds walkable positions for any/all positions a mod specifies
	virtual void CommandNavFloodSelect( const CCommand &args ) = 0;			// select current area and all connected areas, recursively

	virtual void PostCustomAnalysis(void) = 0;
	virtual void RemoveNavArea(CNavArea*) = 0;

	unsigned int GetNavAreaCount( void ) const	{ return m_areaCount; }

	INavArea *GetNavArea( const Vector &pos, float beneathLimt = 120.0f ) const;	// given a position, return the nav area that IsOverlapping and is *immediately* beneath it
	INavArea *GetNavArea( IBaseEntity *pEntity, int nGetNavAreaFlags, float flBeneathLimit = 120.0f ) const;
	INavArea *GetNavAreaByID( unsigned int id ) const;
	INavArea *GetNearestNavArea( const Vector &pos, bool anyZ = false, float maxDist = 10000.0f, bool checkLOS = false, bool checkGround = true, int team = -1 ) const;
	INavArea *GetNearestNavArea( IBaseEntity *pEntity, int nGetNavAreaFlags = GETNAVAREA_CHECK_GROUND, float maxDist = 10000.0f ) const;

	int ComputeHashKey( unsigned int id ) const;

	int WorldToGridX( float wx ) const;							// given X component, return grid index
	int WorldToGridY( float wy ) const;							// given Y component, return grid index

	bool GetGroundHeight( const Vector &pos, float *height, Vector *normal = NULL ) const;		// get the Z coordinate of the topmost ground level below the given point

	template < typename Functor >
	bool ForAllAreasOverlappingExtent( Functor &func, const Extent &extent )
	{
		if ( !m_grid.Count() )
		{
			return true;
		}

		static unsigned int searchMarker = RandomInt(0, 1024*1024 );
		if ( ++searchMarker == 0 )
		{
			++searchMarker;
		}

		Extent areaExtent;

		// get list in cell that contains position
		int startX = WorldToGridX( extent.lo.x );
		int endX = WorldToGridX( extent.hi.x );
		int startY = WorldToGridY( extent.lo.y );
		int endY = WorldToGridY( extent.hi.y );

		for( int x = startX; x <= endX; ++x )
		{
			for( int y = startY; y <= endY; ++y )
			{
				int iGrid = x + y * m_gridSizeX;
				if ( iGrid >= m_grid.Count() )
				{
					ExecuteNTimes( 10, Warning( "** Walked off of the INavMesh::m_grid in ForAllAreasOverlappingExtent()\n" ) );
					return true;
				}

				CUtlVector< INavArea * > *areaVector = &m_grid[ iGrid ];

				// find closest area in this cell
				for ( int it = 0; it < (*areaVector).Count(); it++ )
				{
					INavArea *area = (*areaVector)[ it ];

					// skip if we've already visited this area
					if ( area->GetNavSearchMarker() == searchMarker )
						continue;

					// mark as visited
					area->SetNavSearchMarker(searchMarker);
					area->GetExtent( &areaExtent );

					if ( extent.IsOverlapping( areaExtent ) )
					{
						if ( func( area ) == false )
							return false;
					}
				}
			}
		}
		return true;
	}

	enum EditModeType
	{
		NORMAL,				// normal mesh editing
		PLACE_PAINTING,		// in place painting mode
		CREATING_AREA,		// creating a new nav area
		CREATING_LADDER,	// creating a nav ladder
		DRAG_SELECTING,		// drag selecting a set of areas
		SHIFTING_XY,		// shifting selected set in XY plane
		SHIFTING_Z,			// shifting selected set in Z plane
	};


private:
	friend class INavArea;


	mutable CUtlVector<CUtlVector< INavArea * >> m_grid;
	float m_gridCellSize;										// the width/height of a grid cell for spatially partitioning nav areas for fast access
	int m_gridSizeX;
	int m_gridSizeY;
	float m_minX;
	float m_minY;
	unsigned int m_areaCount;									// offset 13 total number of nav areas

	bool m_isLoaded;											// true if a Navigation Mesh has been loaded
	bool m_isOutOfDate;											// true if the Navigation Mesh is older than the actual BSP
	bool m_isAnalyzed;											// true if the Navigation Mesh needs analysis
	
	enum { HASH_TABLE_SIZE = 256 };
	INavArea *m_hashTable[ HASH_TABLE_SIZE ];					// hash table to optimize lookup by ID
	char **m_placeName;											// master directory of place names (ie: "places")
	unsigned int m_placeCount;									// number of "places" defined in placeName[]
	EditModeType m_editMode;									// the current edit mode
	bool m_isEditing;											// true if in edit mode

	unsigned int m_navPlace;									// current navigation place for editing
	Vector m_editCursorPos;										// current position of the cursor
	CNavArea *m_markedArea;										// offset 279 currently marked area for edit operations
	CNavArea *m_selectedArea;									// area that is selected this frame
	CNavArea *m_lastSelectedArea;								// area that was selected last frame
	NavCornerType m_markedCorner;								// currently marked corner for edit operations
	Vector m_anchor;											// first corner of an area being created
	bool m_isPlacePainting;										// if true, we set an area's place by pointing at it
	bool m_splitAlongX;											// direction the selected nav area would be split
	float m_splitEdge;											// location of the possible split

	bool m_climbableSurface;									// if true, the cursor is pointing at a climable surface
	Vector m_surfaceNormal;										// Normal of the surface the cursor is pointing at
	Vector m_ladderAnchor;										// first corner of a ladder being created
	Vector m_ladderNormal;										// Normal of the surface of the ladder being created
	CNavLadder *m_selectedLadder;								// ladder that is selected this frame
	CNavLadder *m_lastSelectedLadder;							// ladder that was selected last frame
	CNavLadder *m_markedLadder;									// currently marked ladder for edit operations

};

inline int INavMesh::ComputeHashKey( unsigned int id ) const
{
	return id & 0xFF;
}

#endif