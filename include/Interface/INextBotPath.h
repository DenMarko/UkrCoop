#ifndef _HEADER_NEXT_BOT_PATH_INCLUDE_
#define _HEADER_NEXT_BOT_PATH_INCLUDE_

#include "INavArea.h"
#include "INavMesh.h"
#include "INextBot.h"

#define PATH_NO_LENGTH_LIMIT 0.0f				// non-default argument value for Path::Compute()
#define PATH_TRUNCATE_INCOMPLETE_PATH false		// non-default argument value for Path::Compute()

class CNavArea;
class CNavLadder;
class CFuncElevator;


//--------------------------------------------------------------------------------------------------------------
/**
 * Find path from startArea to goalArea via an A* search, using supplied cost heuristic.
 * If cost functor returns -1 for an area, that area is considered a dead end.
 * This doesn't actually build a path, but the path is defined by following parent
 * pointers back from goalArea to startArea.
 * If 'closestArea' is non-NULL, the closest area to the goal is returned (useful if the path fails).
 * If 'goalArea' is NULL, will compute a path as close as possible to 'goalPos'.
 * If 'goalPos' is NULL, will use the center of 'goalArea' as the goal position.
 * If 'maxPathLength' is nonzero, path building will stop when this length is reached.
 * Returns true if a path exists.
 */

template< class T >
T Max( T const &val1, T const &val2 )
{
	return val1 > val2 ? val1 : val2;
}

#define IGNORE_NAV_BLOCKERS true
template< typename CostFunctor >
bool NavAreaBuildPath( INavArea *startArea, INavArea *goalArea, const Vector *goalPos, CostFunctor &costFunc, INavArea **closestArea = NULL, float maxPathLength = 0.0f, int teamID = -1, bool ignoreNavBlockers = false )
{
	if ( closestArea )
	{
		*closestArea = startArea;
	}

	if (startArea == NULL)
	{
		return false;
	}

	startArea->SetParent( NULL );

	if (goalArea != NULL && goalArea->IsBlocked( teamID, ignoreNavBlockers ))
		goalArea = NULL;

	if (goalArea == NULL && goalPos == NULL)
	{
		return false;
	}

	if (startArea == goalArea)
	{
		return true;
	}

	Vector actualGoalPos = (goalPos) ? *goalPos : goalArea->GetCenter();

	INavArea::ClearSearchLists();

	startArea->SetTotalCost( (startArea->GetCenter() - actualGoalPos).Length() );

	float initCost = costFunc( startArea, NULL, NULL, NULL, -1.0f );	
	if (initCost < 0.0f)
	{
		return false;
	}
	startArea->SetCostSoFar( initCost );
	startArea->SetPathLengthSoFar( 0.0 );

	startArea->AddToOpenList();

	// keep track of the area we visit that is closest to the goal
	float closestAreaDist = startArea->GetTotalCost();

	// do A* search
	while( !INavArea::IsOpenListEmpty() )
	{
		// get next area to check
		INavArea *area = INavArea::PopOpenList();

		// don't consider blocked areas
		if ( area->IsBlocked( teamID, ignoreNavBlockers ) )
		{
			continue;
		}

		// check if we have found the goal area or position
		if (area == goalArea || (goalArea == NULL && goalPos && area->Contains( *goalPos )))
		{
			if (closestArea)
			{
				*closestArea = area;
			}

			return true;
		}

		// search adjacent areas
		enum SearchType
		{
			SEARCH_FLOOR, SEARCH_LADDERS, SEARCH_ELEVATORS
		};
		SearchType searchWhere = SEARCH_FLOOR;
		int searchIndex = 0;

		int dir = NORTH;
		const NavConnectVector *floorList = area->GetAdjacentAreas( NORTH );

		bool ladderUp = true;
		const NavLadderConnectVector *ladderList = NULL;
		enum { AHEAD = 0, LEFT, RIGHT, BEHIND, NUM_TOP_DIRECTIONS };
		int ladderTopDir = AHEAD;
		bool bHaveMaxPathLength = ( maxPathLength > 0.0f );
		float length = -1;
		
		while( true )
		{
			INavArea *newArea = NULL;
			NavTraverseType how;
			const INavLadder *ladder = NULL;
			const CFuncElevator *elevator = NULL;

			//
			// Get next adjacent area - either on floor or via ladder
			//
			if ( searchWhere == SEARCH_FLOOR )
			{
				// if exhausted adjacent connections in current direction, begin checking next direction
				if ( searchIndex >= floorList->Count() )
				{
					++dir;

					if ( dir == NUM_DIRECTIONS )
					{
						// checked all directions on floor - check ladders next
						searchWhere = SEARCH_LADDERS;

						ladderList = area->GetLadders( Ladder_DirectionType::LADDER_UP );
						searchIndex = 0;
						ladderTopDir = AHEAD;
					}
					else
					{
						// start next direction
						floorList = area->GetAdjacentAreas( (Nav_DirType)dir );
						searchIndex = 0;
					}

					continue;
				}

				const NavConnect &floorConnect = floorList->Element( searchIndex );
				newArea = floorConnect.area;
				length = floorConnect.length;
				how = (NavTraverseType)dir;
				++searchIndex;

			}
			else if ( searchWhere == SEARCH_LADDERS )
			{
				if ( searchIndex >= ladderList->Count() )
				{
					if ( !ladderUp )
					{
						// checked both ladder directions - check elevators next
						searchWhere = SEARCH_ELEVATORS;
						searchIndex = 0;
						ladder = NULL;
					}
					else
					{
						// check down ladders
						ladderUp = false;
						ladderList = area->GetLadders( Ladder_DirectionType::LADDER_DOWN );
						searchIndex = 0;
					}
					continue;
				}

				if ( ladderUp )
				{
					ladder = ladderList->Element( searchIndex ).ladder;

					// do not use BEHIND connection, as its very hard to get to when going up a ladder
					if ( ladderTopDir == AHEAD )
					{
						newArea = ladder->m_topForwardArea;
					}
					else if ( ladderTopDir == LEFT )
					{
						newArea = ladder->m_topLeftArea;
					}
					else if ( ladderTopDir == RIGHT )
					{
						newArea = ladder->m_topRightArea;
					}
					else
					{
						++searchIndex;
						ladderTopDir = AHEAD;
						continue;
					}

					how = GO_LADDER_UP;
					++ladderTopDir;
				}
				else
				{
					newArea = ladderList->Element( searchIndex ).ladder->m_bottomArea;
					how = GO_LADDER_DOWN;
					ladder = ladderList->Element(searchIndex).ladder;
					++searchIndex;
				}

				if ( newArea == NULL )
					continue;

				length = -1.0f;
			}
			else // if ( searchWhere == SEARCH_ELEVATORS )
			{
				const NavConnectVector &elevatorAreas = area->GetElevatorAreas();

				elevator = area->GetElevator();

				if ( elevator == NULL || searchIndex >= elevatorAreas.Count() )
				{
					// done searching connected areas
					elevator = NULL;
					break;
				}

				newArea = elevatorAreas[ searchIndex++ ].area;
				if ( newArea->GetCenter().z > area->GetCenter().z )
				{
					how = GO_ELEVATOR_UP;
				}
				else
				{
					how = GO_ELEVATOR_DOWN;
				}

				length = -1.0f;
			}


			// don't backtrack
			Assert( newArea );
			if ( newArea == area->GetParent() )
				continue;
			if ( newArea == area ) // self neighbor?
				continue;

			// don't consider blocked areas
			if ( newArea->IsBlocked( teamID, ignoreNavBlockers ) )
				continue;

			float newCostSoFar = costFunc( newArea, area, ladder, elevator, length );
			// NaNs really mess this function up causing tough to track down hangs. If
			//  we get inf back, clamp it down to a really high number.
			if ( (((*(int *)&newCostSoFar) & nanmask) == nanmask) )
			{
				newCostSoFar = 1e30f;
			}
			// check if cost functor says this area is a dead-end
			if ( newCostSoFar < 0.0f )
			{
				continue;
			}
			// Safety check against a bogus functor.  The cost of the path
			// A...B, C should always be at least as big as the path A...B.
			Assert( newCostSoFar >= area->GetCostSoFar() );

			// And now that we've asserted, let's be a bit more defensive.
			// Make sure that any jump to a new area incurs some pathfinsing
			// cost, to avoid us spinning our wheels over insignificant cost
			// benefit, floating point precision bug, or busted cost functor.
			float minNewCostSoFar = area->GetCostSoFar() * 1.00001f + 0.00001f;
			newCostSoFar = Max( newCostSoFar, minNewCostSoFar );
				
			// stop if path length limit reached
			if ( bHaveMaxPathLength )
			{
				// keep track of path length so far
				float deltaLength = ( newArea->GetCenter() - area->GetCenter() ).Length();
				float newLengthSoFar = area->GetPathLengthSoFar() + deltaLength;
				if ( newLengthSoFar > maxPathLength )
					continue;
				
				newArea->SetPathLengthSoFar( newLengthSoFar );
			}

			if ( ( newArea->IsOpen() || newArea->IsClosed() ) && newArea->GetCostSoFar() <= newCostSoFar )
			{
				// this is a worse path - skip it
				continue;
			}
			else
			{
				// compute estimate of distance left to go
				float distSq = ( newArea->GetCenter() - actualGoalPos ).LengthSqr();
				float newCostRemaining = ( distSq > 0.0 ) ? ::sqrtf( distSq ) : 0.0 ;

				// track closest area to goal in case path fails
				if ( closestArea && newCostRemaining < closestAreaDist )
				{
					*closestArea = newArea;
					closestAreaDist = newCostRemaining;
				}
				
				newArea->SetCostSoFar( newCostSoFar );
				newArea->SetTotalCost( newCostSoFar + newCostRemaining );

				if ( newArea->IsClosed() )
				{
					newArea->RemoveFromClosedList();
				}

				if ( newArea->IsOpen() )
				{
					// area already on open list, update the list order to keep costs sorted
					newArea->UpdateOnOpenList();
				}
				else
				{
					newArea->AddToOpenList();
				}

				newArea->SetParent( (CNavArea*)area, how );
			}
		}

		// we have searched this area
		area->AddToClosedList();
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------------
/**
 * The interface for pathfinding costs.
 * TODO: Replace all template cost functors with this interface, so we can virtualize and derive from them.
 */
class IPathCost
{
public:
	virtual float operator()( INavArea *area, INavArea *fromArea, const INavLadder *ladder, const CFuncElevator *elevator, float length ) const = 0;
};


//---------------------------------------------------------------------------------------------------------------
/**
 * The interface for selecting a goal area during "open goal" pathfinding
 */
class IPathOpenGoalSelector
{
public:
	// compare "newArea" to "currentGoal" and return the area that is the better goal area
	virtual INavArea *operator() ( INavArea *currentGoal, INavArea *newArea ) const = 0;
};


//---------------------------------------------------------------------------------------------------------------
/**
 * A Path through the world.
 * Not only does this encapsulate a path to get from point A to point B,
 * but also the selecting the decision algorithm for how to build that path.
 */
class Path
{
public:
	Path( void );
	virtual ~Path() { }
	
	enum SegmentType
	{
		ON_GROUND,
		DROP_DOWN,
		CLIMB_UP,
		JUMP_OVER_GAP,
		LADDER_UP,
		LADDER_DOWN,
		
		NUM_SEGMENT_TYPES
	};

	// @todo Allow custom Segment classes for different kinds of paths	
	struct Segment
	{
		INavArea *area;									// the area along the path
		NavTraverseType how;							// how to enter this area from the previous one
		Vector pos;										// our movement goal position at this point in the path
		const INavLadder *ladder;						// if "how" refers to a ladder, this is it
		
		SegmentType type;								// how to traverse this segment of the path
		Vector forward;									// unit vector along segment
		float length;									// length of this segment
		float distanceFromStart;						// distance of this node from the start of the path
		float curvature;								// how much the path 'curves' at this point in the XY plane (0 = none, 1 = 180 degree doubleback)

		Vector m_portalCenter;							// position of center of 'portal' between previous area and this area
		float m_portalHalfWidth;						// half width of 'portal'
	};

	virtual float GetLength( void ) const;						// return length of path from start to finish
	virtual const Vector &GetPosition( float distanceFromStart, const Segment *start = NULL ) const;	// return a position on the path at the given distance from the path start
	virtual const Vector &GetClosestPosition( const Vector &pos, const Segment *start = NULL, float alongLimit = 0.0f ) const;		// return the closest point on the path to the given position

	virtual const Vector &GetStartPosition( void ) const;	// return the position where this path starts
	virtual const Vector &GetEndPosition( void ) const;		// return the position where this path ends
	virtual CBaseCombatCharacter *GetSubject( void ) const;	// return the actor this path leads to, or NULL if there is no subject

	virtual const Path::Segment *GetCurrentGoal( void ) const;	// return current goal along the path we are trying to reach

	virtual float GetAge( void ) const;					// return "age" of this path (time since it was built)

	enum SeekType
	{
		SEEK_ENTIRE_PATH,			// search the entire path length
		SEEK_AHEAD,					// search from current cursor position forward toward end of path
		SEEK_BEHIND					// search from current cursor position backward toward path start
	};
	virtual void MoveCursorToClosestPosition( const Vector &pos, SeekType type = SEEK_ENTIRE_PATH, float alongLimit = 0.0f ) const;		// Set cursor position to closest point on path to given position
	
	enum MoveCursorType
	{
		PATH_ABSOLUTE_DISTANCE,
		PATH_RELATIVE_DISTANCE
	};
	virtual void MoveCursorToStart( void );				// set seek cursor to start of path
	virtual void MoveCursorToEnd( void );				// set seek cursor to end of path
	virtual void MoveCursor( float value, MoveCursorType type = PATH_ABSOLUTE_DISTANCE );	// change seek cursor position
	virtual float GetCursorPosition( void ) const;		// return position of seek cursor (distance along path)

	struct Data
	{
		Vector pos;										// the position along the path
		Vector forward;									// unit vector along path direction
		float curvature;								// how much the path 'curves' at this point in the XY plane (0 = none, 1 = 180 degree doubleback)
		const Segment *segmentPrior;					// the segment just before this position
	};
	virtual const Data &GetCursorData( void ) const;	// return path state at the current cursor position

	virtual bool IsValid( void ) const;
	virtual void Invalidate( void );					// make path invalid (clear it)

	virtual void Draw( const Path::Segment *start = nullptr ) const;	// draw the path for debugging
	virtual void DrawInterpolated( float from, float to );	// draw the path for debugging - MODIFIES cursor position

	virtual const Segment *FirstSegment( void ) const;	// return first segment of path
	virtual const Segment *NextSegment( const Segment *currentSegment ) const;	// return next segment of path, given current one
	virtual const Segment *PriorSegment( const Segment *currentSegment ) const;	// return previous segment of path, given current one
	virtual const Segment *LastSegment( void ) const;	// return last segment of path

	enum ResultType
	{
		COMPLETE_PATH,
		PARTIAL_PATH,
		NO_PATH
	};
	virtual void OnPathChanged( INextBot *bot, ResultType result ) { }		// invoked when the path is (re)computed (path is valid at the time of this call)

	virtual void Copy( INextBot *bot, const Path &path );	// Replace this path with the given path's data


	//-----------------------------------------------------------------------------------------------------------------
	/**
	 * Compute shortest path from bot to given actor via A* algorithm.
	 * If returns true, path was found to the subject.
	 * If returns false, path may either be invalid (use IsValid() to check), or valid but 
	 * doesn't reach all the way to the subject.
	 */
	template< typename CostFunctor >
	bool Compute( INextBot *bot, CBaseCombatCharacter *subject, CostFunctor &costFunc, float maxPathLength = 0.0f, bool includeGoalIfPathFails = true )
	{
		Invalidate();

		m_subject = subject;
		
		const Vector &start = bot->GetPosition();
		
		INavArea *startArea = (INavArea *)reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetLastKnownArea();
		if ( !startArea )
		{
			OnPathChanged( bot, NO_PATH );
			return false;
		}

		INavArea *subjectArea = (INavArea *)reinterpret_cast<IBaseCombatCharacter *>(subject)->GetLastKnownArea();
		if ( !subjectArea )
		{
			OnPathChanged( bot, NO_PATH );
			return false;
		}

		Vector subjectPos = reinterpret_cast<IBaseCombatCharacter *>(subject)->GetAbsOrigin();
		
		if ( startArea == subjectArea )
		{
			BuildTrivialPath( bot, subjectPos );
			return true;
		}

		INavArea *closestArea = NULL;
		bool pathResult = NavAreaBuildPath( startArea, subjectArea, &subjectPos, costFunc, &closestArea, maxPathLength, reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetTeamNumber() );

		if ( closestArea == NULL )
			return false;

		int count = 0;
		INavArea *area;
		for( area = closestArea; area; area = area->GetParent() )
		{
			++count;

			if ( area == startArea )
			{
				break;
			}
			if ( count >= MAX_PATH_SEGMENTS-1 )
				break;
		}
		
		if ( count == 1 )
		{
			BuildTrivialPath( bot, subjectPos );
			return pathResult;
		}

		m_segmentCount = count;
		for( area = closestArea; count && area; area = area->GetParent() )
		{
			--count;
			m_path[ count ].area = area;
			m_path[ count ].how = area->GetParentHow();
			m_path[ count ].type = ON_GROUND;
		}

		if ( pathResult || includeGoalIfPathFails )
		{
			m_path[ m_segmentCount ].area = closestArea;
			m_path[ m_segmentCount ].pos = subjectPos;
			m_path[ m_segmentCount ].ladder = NULL;
			m_path[ m_segmentCount ].how = NUM_TRAVERSE_TYPES;
			m_path[ m_segmentCount ].type = ON_GROUND;
			++m_segmentCount;
		}
				
		if ( ComputePathDetails( bot, start ) == false )
		{
			Invalidate();
			OnPathChanged( bot, NO_PATH );
			return false;
		}

		Optimize( bot );
		PostProcess();
		OnPathChanged( bot, pathResult ? COMPLETE_PATH : PARTIAL_PATH );

		return pathResult;
	}

	template< typename CostFunctor >
	bool Compute( INextBot *bot, const Vector &goal, CostFunctor &costFunc, float maxPathLength = 0.0f, bool includeGoalIfPathFails = true )
	{
		Invalidate();

		const Vector &start = bot->GetPosition();
		
		INavArea *startArea = (INavArea *)reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetLastKnownArea();
		if ( !startArea )
		{
			OnPathChanged( bot, NO_PATH );
			return false;
		}

		INavMesh* pMesh = g_HL2->GetTheNavMesh();
		if(!pMesh)
		{
			return false;
		}

		const float maxDistanceToArea = 200.0f;
		INavArea *goalArea = pMesh->GetNearestNavArea( goal, true, maxDistanceToArea, true );

		if ( startArea == goalArea )
		{
			BuildTrivialPath( bot, goal );
			return true;
		}

		Vector pathEndPosition = goal;
		if ( goalArea )
		{
			pathEndPosition.z = goalArea->GetZ( pathEndPosition );
		}
		else
		{
			pMesh->GetGroundHeight( pathEndPosition, &pathEndPosition.z );
		}

		INavArea *closestArea = NULL;
		bool pathResult = NavAreaBuildPath( startArea, goalArea, &goal, costFunc, &closestArea, maxPathLength, reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetTeamNumber() );

		if ( closestArea == NULL )
			return false;

		int count = 0;
		INavArea *area;
		for( area = closestArea; area; area = area->GetParent() )
		{
			++count;

			if ( area == startArea )
			{
				break;
			}
			if ( count >= MAX_PATH_SEGMENTS-1 )
				break;
		}
		
		if ( count == 1 )
		{
			BuildTrivialPath( bot, goal );
			return pathResult;
		}

		m_segmentCount = count;
		for( area = closestArea; count && area; area = area->GetParent() )
		{
			--count;
			m_path[ count ].area = area;
			m_path[ count ].how = area->GetParentHow();
			m_path[ count ].type = ON_GROUND;
		}

		if ( pathResult || includeGoalIfPathFails )
		{
			m_path[ m_segmentCount ].area = closestArea;
			m_path[ m_segmentCount ].pos = pathEndPosition;
			m_path[ m_segmentCount ].ladder = NULL;
			m_path[ m_segmentCount ].how = NUM_TRAVERSE_TYPES;
			m_path[ m_segmentCount ].type = ON_GROUND;
			++m_segmentCount;
		}
				
		if ( ComputePathDetails( bot, start ) == false )
		{
			Invalidate();
			OnPathChanged( bot, NO_PATH );
			return false;
		}

		Optimize( bot );
		PostProcess();
		OnPathChanged( bot, pathResult ? COMPLETE_PATH : PARTIAL_PATH );

		return pathResult;
	}


	virtual bool ComputeWithOpenGoal( INextBot *bot, const IPathCost &costFunc, const IPathOpenGoalSelector &goalSelector, float maxSearchRadius = 0.0f )
	{
		int teamID = reinterpret_cast<IBaseEntity *>(bot->GetEntity())->GetTeamNumber();

		INavArea *startArea = (INavArea *)reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetLastKnownArea();

		if ( startArea == NULL )
			return NULL;

		startArea->SetParent( NULL );

		INavArea::ClearSearchLists();

		float initCost = costFunc( startArea, NULL, NULL, NULL, -1.0f );
		if ( initCost < 0.0f )
			return NULL;

		startArea->SetTotalCost( initCost );
		startArea->AddToOpenList();

		INavArea *goalArea = NULL;

		while( !INavArea::IsOpenListEmpty() )
		{
			INavArea *area = INavArea::PopOpenList();

			area->AddToClosedList();

			if ( area->IsBlocked( teamID ) )
				continue;

			CollectAdjacentAreas( area );

			for( int i=0; i<m_adjAreaIndex; ++i )
			{
				INavArea *newArea = (INavArea *)m_adjAreaVector[ i ].area;

				if ( newArea->IsClosed() )
					continue;

				if ( newArea->IsBlocked( teamID ) )
					continue;

				if ( maxSearchRadius > 0.0f && ( newArea->GetCenter() - reinterpret_cast<IBaseEntity*>(bot->GetEntity())->GetAbsOrigin() ).IsLengthGreaterThan( maxSearchRadius ) )
					continue;

				float newCost = costFunc( newArea, area, m_adjAreaVector[ i ].ladder, NULL, -1.0f );

				if ( newCost < 0.0f )
					continue;

				if ( newArea->IsOpen() && newArea->GetTotalCost() <= newCost )
				{
					continue;
				}
				else
				{
					newArea->SetParent( (CNavArea*)area, m_adjAreaVector[ i ].how );
					newArea->SetTotalCost( newCost );

					newArea->SetCostSoFar( newCost );

					if ( newArea->IsOpen() )
					{
						newArea->UpdateOnOpenList();
					}
					else
					{
						newArea->AddToOpenList();
					}

					goalArea = goalSelector( goalArea, newArea );
				}
			}
		}

		if ( goalArea )
		{
			AssemblePrecomputedPath( bot, goalArea->GetCenter(), goalArea );
			return true;
		}	

		return false;
	}


	void AssemblePrecomputedPath( INextBot *bot, const Vector &goal, INavArea *endArea )
	{
		const Vector &start = bot->GetPosition();

		int count = 0;
		INavArea *area;
		for( area = endArea; area; area = (INavArea *)area->GetParent() )
		{
			++count;
		}

		if ( count > MAX_PATH_SEGMENTS-1 )
		{
			count = MAX_PATH_SEGMENTS-1;
		}
		else if ( count == 0 )
		{
			return;
		}

		if ( count == 1 )
		{
			BuildTrivialPath( bot, goal );
			return;
		}

		m_segmentCount = count;
		for( area = endArea; count && area; area = (INavArea *)area->GetParent() )
		{
			--count;
			m_path[ count ].area = area;
			m_path[ count ].how = area->GetParentHow();
			m_path[ count ].type = ON_GROUND;
		}

		m_path[ m_segmentCount ].area = endArea;
		m_path[ m_segmentCount ].pos = goal;
		m_path[ m_segmentCount ].ladder = NULL;
		m_path[ m_segmentCount ].how = NUM_TRAVERSE_TYPES;
		m_path[ m_segmentCount ].type = ON_GROUND;
		++m_segmentCount;

		if ( ComputePathDetails( bot, start ) == false )
		{
			Invalidate();
			OnPathChanged( bot, NO_PATH );
			return;
		}

		Optimize( bot );
		PostProcess();
		OnPathChanged( bot, COMPLETE_PATH );
	}

	bool BuildTrivialPath( INextBot *bot, const Vector &goal );	
	virtual void ComputeAreaCrossing( INextBot *bot, const CNavArea *from, const Vector &fromPos, const CNavArea *to, Nav_DirType dir, Vector *crossPos ) const;


private:
	enum { MAX_PATH_SEGMENTS = 256 };
	Segment m_path[ MAX_PATH_SEGMENTS ];
	int m_segmentCount;

	bool ComputePathDetails( INextBot *bot, const Vector &start );		// determine actual path positions 

	void Optimize( INextBot *bot );
	void PostProcess( void );
	int FindNextOccludedNode( INextBot *bot, int anchor );	// used by Optimize()

	void InsertSegment( Segment newSegment, int i );		// insert new segment at index i
	
	mutable Vector m_pathPos;								// used by GetPosition()
	mutable Vector m_closePos;								// used by GetClosestPosition()

	mutable float m_cursorPos;					// current cursor position (distance along path)
	mutable Data m_cursorData;					// used by GetCursorData()
	mutable bool m_isCursorDataDirty;

	IntervalTimers m_ageTimer;					// how old is this path?
	CHandle< CBaseCombatCharacter > m_subject;	// the subject this path leads to

	void CollectAdjacentAreas( INavArea *area )
	{
		m_adjAreaIndex = 0;			

		const NavConnectVector &adjNorth = *area->GetAdjacentAreas( NORTH );		
		for ( int it = 0; (adjNorth).IsUtlVector && it < (adjNorth).Count(); it++ )
		{
			if ( m_adjAreaIndex >= MAX_ADJ_AREAS )
				break;

			m_adjAreaVector[ m_adjAreaIndex ].area = adjNorth[ it ].area;
			m_adjAreaVector[ m_adjAreaIndex ].how = GO_NORTH;
			m_adjAreaVector[ m_adjAreaIndex ].ladder = nullptr;
			++m_adjAreaIndex;
		}

		const NavConnectVector &adjSouth = *area->GetAdjacentAreas( SOUTH );		
		for ( int it = 0; (adjSouth).IsUtlVector && it < (adjSouth).Count(); it++ )
		{
			if ( m_adjAreaIndex >= MAX_ADJ_AREAS )
				break;

			m_adjAreaVector[ m_adjAreaIndex ].area = adjSouth[ it ].area;
			m_adjAreaVector[ m_adjAreaIndex ].how = GO_SOUTH;
			m_adjAreaVector[ m_adjAreaIndex ].ladder = NULL;
			++m_adjAreaIndex;
		}

		const NavConnectVector &adjWest = *area->GetAdjacentAreas( WEST );		
		for ( int it = 0; (adjWest).IsUtlVector && it < (adjWest).Count(); it++ )
		{
			if ( m_adjAreaIndex >= MAX_ADJ_AREAS )
				break;

			m_adjAreaVector[ m_adjAreaIndex ].area = adjWest[ it ].area;
			m_adjAreaVector[ m_adjAreaIndex ].how = GO_WEST;
			m_adjAreaVector[ m_adjAreaIndex ].ladder = NULL;
			++m_adjAreaIndex;
		}

		const NavConnectVector &adjEast = *area->GetAdjacentAreas( EAST );	
		for ( int it = 0; (adjEast).IsUtlVector && it < (adjEast).Count(); it++ )
		{
			if ( m_adjAreaIndex >= MAX_ADJ_AREAS )
				break;

			m_adjAreaVector[ m_adjAreaIndex ].area = adjEast[ it ].area;
			m_adjAreaVector[ m_adjAreaIndex ].how = GO_EAST;
			m_adjAreaVector[ m_adjAreaIndex ].ladder = NULL;
			++m_adjAreaIndex;
		}

		const NavLadderConnectVector &adjUpLadder = *area->GetLadders( Ladder_DirectionType::LADDER_UP );
		for ( int it = 0; (adjUpLadder).IsUtlVector && it < (adjUpLadder).Count(); it++ )
		{
			INavLadder *ladder = (INavLadder*)adjUpLadder[ it ].ladder;

			if ( ladder->m_topForwardArea && m_adjAreaIndex < MAX_ADJ_AREAS )
			{
				m_adjAreaVector[ m_adjAreaIndex ].area = ladder->m_topForwardArea;
				m_adjAreaVector[ m_adjAreaIndex ].how = GO_LADDER_UP;
				m_adjAreaVector[ m_adjAreaIndex ].ladder = ladder;
				++m_adjAreaIndex;
			}

			if ( ladder->m_topLeftArea && m_adjAreaIndex < MAX_ADJ_AREAS )
			{
				m_adjAreaVector[ m_adjAreaIndex ].area = ladder->m_topLeftArea;
				m_adjAreaVector[ m_adjAreaIndex ].how = GO_LADDER_UP;
				m_adjAreaVector[ m_adjAreaIndex ].ladder = ladder;
				++m_adjAreaIndex;
			}

			if ( ladder->m_topRightArea && m_adjAreaIndex < MAX_ADJ_AREAS )
			{
				m_adjAreaVector[ m_adjAreaIndex ].area = ladder->m_topRightArea;
				m_adjAreaVector[ m_adjAreaIndex ].how = GO_LADDER_UP;
				m_adjAreaVector[ m_adjAreaIndex ].ladder = ladder;
				++m_adjAreaIndex;
			}
		}

		const NavLadderConnectVector &adjDownLadder = *area->GetLadders( Ladder_DirectionType::LADDER_DOWN );
		for ( int it = 0; (adjDownLadder).IsUtlVector && it < (adjDownLadder).Count(); it++ )
		{
			INavLadder *ladder = (INavLadder*)adjDownLadder[ it ].ladder;

			if ( m_adjAreaIndex >= MAX_ADJ_AREAS )
				break;

			if ( ladder->m_bottomArea )
			{
				m_adjAreaVector[ m_adjAreaIndex ].area = ladder->m_bottomArea;
				m_adjAreaVector[ m_adjAreaIndex ].how = GO_LADDER_DOWN;
				m_adjAreaVector[ m_adjAreaIndex ].ladder = ladder;
				++m_adjAreaIndex;
			}
		}
	}

	enum { MAX_ADJ_AREAS = 64 };

	struct AdjInfo
	{
		INavArea *area;
		INavLadder *ladder;
		NavTraverseType how;		
	};

	AdjInfo m_adjAreaVector[ MAX_ADJ_AREAS ];
	int m_adjAreaIndex;
};

inline float Path::GetLength( void ) const
{
	if (m_segmentCount <= 0)
	{
		return 0.0f;
	}
	
	return m_path[ m_segmentCount-1 ].distanceFromStart;
}

inline bool Path::IsValid( void ) const
{
	return (m_segmentCount > 0);
}

inline void Path::Invalidate( void )
{
	m_segmentCount = 0;

	m_cursorPos = 0.0f;
	
	m_cursorData.pos = vec3_origin;
	m_cursorData.forward = Vector( 1.0f, 0, 0 );
	m_cursorData.curvature = 0.0f;
	m_cursorData.segmentPrior = NULL;
	
	m_isCursorDataDirty = true;

	m_subject = NULL;
}

inline const Path::Segment *Path::FirstSegment( void ) const
{
	return (IsValid()) ? &m_path[0] : NULL;
}

inline const Path::Segment *Path::NextSegment( const Segment *currentSegment ) const
{
	if (currentSegment == NULL || !IsValid())
		return NULL;
		
	int i = currentSegment - m_path;

	if (i < 0 || i >= m_segmentCount-1)
	{
		return NULL;
	}
	
	return &m_path[ i+1 ];
}

inline const Path::Segment *Path::PriorSegment( const Segment *currentSegment ) const
{
	if (currentSegment == NULL || !IsValid())
		return NULL;
		
	int i = currentSegment - m_path;

	if (i < 1 || i >= m_segmentCount)
	{
		return NULL;
	}
	
	return &m_path[ i-1 ];
}

inline const Path::Segment *Path::LastSegment( void ) const
{
	return ( IsValid() ) ? &m_path[ m_segmentCount-1 ] : NULL;
}

inline const Vector &Path::GetStartPosition( void ) const
{
	return ( IsValid() ) ? m_path[ 0 ].pos : vec3_origin;
}

inline const Vector &Path::GetEndPosition( void ) const
{
	return ( IsValid() ) ? m_path[ m_segmentCount-1 ].pos : vec3_origin;
}

inline CBaseCombatCharacter *Path::GetSubject( void ) const
{
	return m_subject;
}

inline void Path::MoveCursorToStart( void )
{
	m_cursorPos = 0.0f;
	m_isCursorDataDirty = true;
}

inline void Path::MoveCursorToEnd( void )
{
	m_cursorPos = GetLength();
	m_isCursorDataDirty = true;
}

inline void Path::MoveCursor( float value, MoveCursorType type )
{
	if ( type == PATH_ABSOLUTE_DISTANCE )
	{
		m_cursorPos = value;
	}
	else
	{
		m_cursorPos += value;
	}
	
	if ( m_cursorPos < 0.0f )
	{
		m_cursorPos = 0.0f;
	}
	else if ( m_cursorPos > GetLength() )
	{
		m_cursorPos = GetLength();
	}
	
	m_isCursorDataDirty = true;
}

inline float Path::GetCursorPosition( void ) const
{
	return m_cursorPos;
}

inline const Path::Segment *Path::GetCurrentGoal( void ) const
{
	return NULL;
}

inline float Path::GetAge( void ) const
{
	return m_ageTimer.GetElapsedTime();
}


#endif