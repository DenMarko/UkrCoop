#include "INextBotPath.h"
#include "INextBot.h"
#include "INavMesh.h"
#include "CTrace.h"
#include "CTempEntity.h"

Path::Path(void)
{
	m_segmentCount = 0;
	
	m_cursorPos = 0.0f;
	m_isCursorDataDirty = true;
	m_cursorData.segmentPrior = NULL;
	m_ageTimer.Invalidate();
	m_subject = NULL;
}

bool Path::ComputePathDetails( INextBot *bot, const Vector &start )
{
	if (m_segmentCount == 0)
		return false;
		
	IBody *body = bot->GetBodyInterface();
	ILocomotion *mover = bot->GetLocomotionInterface();
	
	const float stepHeight = ( mover ) ? mover->GetStepHeight() : 18.0f;

	// inflate hull width slightly as a safety margin
	const float hullWidth = ( body ) ? body->GetHullWidth() + 5.0f : 1.0f;

	// set first path position
	if ( m_path[0].area->Contains( start ) )
	{
		m_path[0].pos = start;
	}
	else
	{
		// start in first area's center
		m_path[0].pos = m_path[0].area->GetCenter();
	}	
	m_path[0].ladder = NULL;
	m_path[0].how = NUM_TRAVERSE_TYPES;
	m_path[0].type = ON_GROUND;

	// set positions along the path
	for( int i=1; i<m_segmentCount; ++i )
	{
		Segment *from = &m_path[ i-1 ];
		Segment *to = &m_path[ i ];
		
		if ( to->how <= GO_WEST )		// walk along the floor to the next area
		{
			to->ladder = NULL;

			from->area->ComputePortal( to->area, (Nav_DirType)to->how, &to->m_portalCenter, &to->m_portalHalfWidth );

			// compute next point
			ComputeAreaCrossing( bot, (CNavArea *)from->area, from->pos, (CNavArea *)to->area, (Nav_DirType)to->how, &to->pos );

			// we need to walk out of "from" area, so keep Z where we can reach it
			to->pos.z = from->area->GetZ( to->pos );

			// if this is a "jump down" connection, we must insert an additional point on the path
			// float expectedHeightDrop = from->area->GetZ( from->pos ) - to->area->GetZ( to->pos );

			// measure the drop distance relative to the actual slope of the ground
			Vector fromPos = from->pos;
			fromPos.z = from->area->GetZ( fromPos );

			Vector toPos = to->pos;
			toPos.z = to->area->GetZ( toPos );

			Vector groundNormal;
			from->area->ComputeNormal( &groundNormal );

			Vector alongPath = toPos - fromPos;

			float expectedHeightDrop = -DotProduct( alongPath, groundNormal );

			if ( expectedHeightDrop > mover->GetStepHeight() )
			{
				// NOTE: We can't know this is a drop-down yet, because of subtle interactions
				// between nav area links and "portals" and "area crossings"

				// compute direction of path just prior to "jump down"
				Vector2D dir = vec2_origin;
				DirectionToVector2D( (Nav_DirType)to->how, &dir );

				// shift top of "jump down" out a bit to "get over the ledge"
				const float inc = 0.25f * hullWidth; // 10.0f;
				const float maxPushDist = 75.0f; // 2.0f * hullWidth;
				float halfWidth = hullWidth / 2.0f;
				float hullHeight = ( body ) ? body->GetCrouchHullHeight() : 1.0f;
				
				float pushDist;
				for( pushDist = 0.0f; pushDist <= maxPushDist; pushDist += inc )
				{
					Vector pos = to->pos + Vector( pushDist * dir.x, pushDist * dir.y, 0.0f );
					Vector lowerPos = Vector( pos.x, pos.y, toPos.z );
					
					trace_t result;
					NextBotTraceFilterIgnoreActors filter( (IHandleEntity*)bot->GetEntity(), COLLISION_GROUP_NONE );
					util_TraceHull( pos, lowerPos,
									Vector( -halfWidth, -halfWidth, stepHeight ), Vector( halfWidth, halfWidth, hullHeight ), 
									bot->GetBodyInterface()->GetSolidMask(), &filter, &result );
					
					if ( result.fraction >= 1.0f )
					{
						break;
					}
				}
				
				Vector startDrop( to->pos.x + pushDist * dir.x, to->pos.y + pushDist * dir.y, to->pos.z );
				Vector endDrop( startDrop.x, startDrop.y, to->area->GetZ( to->pos ) );

				INavMesh* pMesh = g_HL2->GetTheNavMesh();
				if(!pMesh)
				{
					Msg("[Path::ComputePathDetails] INavMesh is nullptr\n");
				}
				else
				{
					float ground;
					if ( pMesh->GetGroundHeight( endDrop, &ground ) )
					{
						if ( startDrop.z > ground + stepHeight )
						{
							// if "ground" is lower than the next segment along the path
							// there is a chasm between - this is not a drop down
							// NOTE next->pos is not yet valid - this loop is computing it!
							const Segment *next = NextSegment( to );
							if ( !next || next->area->GetCenter().z < ground + stepHeight )
							{
								to->pos = startDrop;
								to->type = DROP_DOWN;

								if ( m_segmentCount < MAX_PATH_SEGMENTS-1 )
								{
									for( int j=m_segmentCount; j>i; --j )
										m_path[j] = m_path[j-1];

									++m_segmentCount;

									++i;

									m_path[i].pos.x = endDrop.x;
									m_path[i].pos.y = endDrop.y;
									m_path[i].pos.z = ground;
									
									m_path[i].type = ON_GROUND;
								}						
							}
						}
					}
				}
			}
		}
		else if ( to->how == GO_LADDER_UP )
		{
			const NavLadderConnectVector *ladders = from->area->GetLadders( Ladder_DirectionType::LADDER_UP );
			int it;
			for( it=0; it<ladders->Count(); ++it )
			{
				INavLadder *ladder = (INavLadder*)(*ladders)[ it ].ladder;
				
				if (ladder->m_topForwardArea == to->area ||
					ladder->m_topLeftArea == to->area ||
					ladder->m_topRightArea == to->area)
				{
					to->ladder = ladder;
					to->pos = ladder->m_bottom + ladder->GetNormal() * 2.0f * HalfHumanWidth;
					to->type = LADDER_UP;
					break;
				}
			}
			
			if (it == ladders->Count())
			{
				return false;
			}
		}
		else if ( to->how == GO_LADDER_DOWN )
		{
			const NavLadderConnectVector *ladders = from->area->GetLadders( Ladder_DirectionType::LADDER_DOWN );
			int it;
			for( it=0; it<ladders->Count(); ++it )
			{
				INavLadder *ladder = (INavLadder*)(*ladders)[ it ].ladder;

				if (ladder->m_bottomArea == to->area)
				{
					to->ladder = ladder;
					to->pos = ladder->m_top;
					to->pos = ladder->m_top - ladder->GetNormal() * 2.0f * HalfHumanWidth;
					to->type = LADDER_DOWN;
					break;
				}
			}

			if (it == ladders->Count())
			{
				return false;
			}
		}
		else if ( to->how == GO_ELEVATOR_UP || to->how == GO_ELEVATOR_DOWN )
		{
			to->pos = to->area->GetCenter();
			to->ladder = NULL;
		}
	}

	for( int i=0; i<m_segmentCount-1; ++i )
	{
		Segment *from = &m_path[ i ];
		Segment *to = &m_path[ i+1 ];
		
		if ( from->how != NUM_TRAVERSE_TYPES && from->how > GO_WEST )
			continue;
		
		if ( to->how > GO_WEST || !to->type == ON_GROUND )
			continue;

		Vector closeFrom, closeTo;
		to->area->GetClosestPointOnArea( from->pos, &closeTo );
		from->area->GetClosestPointOnArea( closeTo, &closeFrom );

		const float separationTolerance = 1.9f * 25.f;
		if ( (closeFrom - closeTo).AsVector2D().IsLengthGreaterThan( separationTolerance ) && ( closeTo - closeFrom ).AsVector2D().IsLengthGreaterThan( 0.5f * fabs( closeTo.z - closeFrom.z ) ) )
		{
			Vector landingPos;
			to->area->GetClosestPointOnArea( to->pos, &landingPos );

			Vector launchPos;
			from->area->GetClosestPointOnArea( landingPos, &launchPos );

			Vector forward = landingPos - launchPos;
			forward.NormalizeInPlace();
			
			const float halfWidth = hullWidth/2.0f;

			to->pos = landingPos + forward * halfWidth;
			
			Segment newSegment = *from;

			newSegment.pos = launchPos - forward * halfWidth;
			newSegment.type = JUMP_OVER_GAP;

			InsertSegment( newSegment, i+1 );
			
			++i;
		}
		else if ( (closeTo.z - closeFrom.z) > stepHeight )
		{
			to->pos = to->area->GetCenter();
			
			Segment newSegment = *from;
			
			Vector launchPos;
			from->area->GetClosestPointOnArea( to->pos, &launchPos );
			
			newSegment.pos = launchPos;
			newSegment.type = CLIMB_UP;

			InsertSegment( newSegment, i+1 );

			++i;			
		}
	}

	return true;
}

void Path::InsertSegment( Segment newSegment, int i )
{
	if (m_segmentCount < MAX_PATH_SEGMENTS-1)
	{
		// shift segments to make room for new one
		for( int j=m_segmentCount; j>i; --j )
			m_path[j] = m_path[j-1];

		// path is one node longer
		++m_segmentCount;

		m_path[i] = newSegment;
	}										
}

bool Path::BuildTrivialPath( INextBot *bot, const Vector &goal )
{
	const Vector &start = bot->GetPosition();
	
	m_segmentCount = 0;

	INavMesh* pMesh = g_HL2->GetTheNavMesh();
	if(!pMesh)
	{
		Msg("[Path::BuildTrivialPath] INavMesh is null\n");
		return false;
	}

	/// @todo Dangerous to use "nearset" nav area - could be far away
	INavArea *startArea = pMesh->GetNearestNavArea( start );
	if (startArea == NULL)
		return false;

	INavArea *goalArea = pMesh->GetNearestNavArea( goal );
	if (goalArea == NULL)
		return false;

	m_segmentCount = 2;

	m_path[0].area = startArea;
	m_path[0].pos.x = start.x;
	m_path[0].pos.y = start.y;
	m_path[0].pos.z = startArea->GetZ( start );
	m_path[0].ladder = NULL;
	m_path[0].how = NUM_TRAVERSE_TYPES;
	m_path[0].type = ON_GROUND;

	m_path[1].area = goalArea;
	m_path[1].pos.x = goal.x;
	m_path[1].pos.y = goal.y;
	m_path[1].pos.z = goalArea->GetZ( goal );
	m_path[1].ladder = NULL;
	m_path[1].how = NUM_TRAVERSE_TYPES;
	m_path[1].type = ON_GROUND;

	m_path[0].forward = m_path[1].pos - m_path[0].pos;
	m_path[0].length = m_path[0].forward.NormalizeInPlace();
	m_path[0].distanceFromStart = 0.0f;
	m_path[0].curvature = 0.0f;
	
	m_path[1].forward = m_path[0].forward;
	m_path[1].length = 0.0f;
	m_path[1].distanceFromStart = m_path[0].length;
	m_path[1].curvature = 0.0f;

	OnPathChanged( bot, COMPLETE_PATH );

	return true;
}

void Path::Draw( const Path::Segment *start ) const
{
	if ( !IsValid() )
		return;
}

void Path::DrawInterpolated( float from, float to )
{
	if ( !IsValid() )
	{
		return;
	}
	
	float t = from;
	MoveCursor( t );
}

int Path::FindNextOccludedNode( INextBot *bot, int anchorIndex )
{
	ILocomotion *mover = bot->GetLocomotionInterface();
	if ( mover == NULL)
	{
		return m_segmentCount;
	}
	
	Segment *anchor = &m_path[ anchorIndex ];
	
	for( int i=anchorIndex+1; i<m_segmentCount; ++i )
	{
		Segment *to = &m_path[i];
		
		if ( !to->type == ON_GROUND || (to->area->GetAttributes() & NAV_MESH_PRECISE) )
		{
			return i;
		}

		if ( !mover->IsPotentiallyTraversable( anchor->pos, to->pos, ILocomotion::IMMEDIATELY ) )
		{
			return i;
		}

		if ( mover->HasPotentialGap( anchor->pos, to->pos ) )
		{
			return i;
		}
	}

	return m_segmentCount;
}

void Path::Optimize( INextBot *bot )
{
	return;
}

void Path::PostProcess( void )
{
	m_ageTimer.Start();

	if (m_segmentCount == 0)
		return;
		
	if (m_segmentCount == 1)
	{
		m_path[0].forward = vec3_origin;
		m_path[0].length = 0.0f;
		m_path[0].distanceFromStart = 0.0f;
		m_path[0].curvature = 0.0f;
		return;
	}
	
	float distanceSoFar = 0.0f;
	int i;
	for( i=0; i < m_segmentCount-1; ++i )
	{
		Segment *from = &m_path[ i ];
		Segment *to = &m_path[ i+1 ];
		
		from->forward = to->pos - from->pos;
		from->length = from->forward.NormalizeInPlace();
		
		from->distanceFromStart = distanceSoFar;

		distanceSoFar += from->length;
	}
	
		
	Vector2D from, to;
	for( i=1; i < m_segmentCount-1; ++i )
	{
		if (m_path[ i ].type != ON_GROUND)
		{
			m_path[ i ].curvature = 0.0f;
		}
		else
		{
			from = m_path[ i-1 ].forward.AsVector2D();
			from.NormalizeInPlace();
			
			to = m_path[ i ].forward.AsVector2D();
			to.NormalizeInPlace();	
		
			m_path[ i ].curvature = 0.5f * ( 1.0f - from.Dot( to ) );
			
			Vector2D right( -from.y, from.x );
			if ( to.Dot( right ) < 0.0f )
			{
				m_path[ i ].curvature = -m_path[ i ].curvature;
			}
		}
	}

	m_path[ 0 ].curvature = 0.0f;
	
	m_path[ i ].forward = m_path[ i-1 ].forward;
	m_path[ i ].length = 0.0f;
	m_path[ i ].distanceFromStart = distanceSoFar;
	m_path[ i ].curvature = 0.0f;
}

const Vector &Path::GetPosition( float distanceFromStart, const Segment *start ) const
{
	if (!IsValid())
	{
		return vec3_origin;
	}

	float lengthSoFar;
	const Segment *segment;
	
	if (start)
	{
		segment = start;
		lengthSoFar = start->distanceFromStart;
	}
	else
	{
		segment = &m_path[0];
		lengthSoFar = 0.0f;
	}

	if (segment->distanceFromStart > distanceFromStart)
	{
		return segment->pos;
	}

	
	const Segment *next = NextSegment( segment );

	Vector delta;
	float length;

	while( next )
	{
		delta = next->pos - segment->pos;
		length = segment->length;

		if (lengthSoFar + length >= distanceFromStart)
		{
			float overlap = distanceFromStart - lengthSoFar;
			float t = overlap / length;

			m_pathPos = segment->pos + t * delta;
			
			return m_pathPos;
		}

		lengthSoFar += length;
		
		segment = next;
		next = NextSegment( next );
	}

	return segment->pos;
}

const Vector &Path::GetClosestPosition( const Vector &pos, const Segment *start, float alongLimit ) const
{
	const Segment *segment = (start) ? start : &m_path[0];
	
	if (segment == NULL)
	{
		return pos;
	}
	
	m_closePos = pos;
	float closeRangeSq = 99999999999.9f;

	float distanceSoFar = 0.0f;	
	while( alongLimit == 0.0f || distanceSoFar <= alongLimit )
	{
		const Segment *nextSegment = NextSegment( segment );
		
		if (nextSegment)
		{
			Vector close;
			CalcClosestPointOnLineSegment( pos, segment->pos, nextSegment->pos, close );
			float rangeSq = (close - pos).LengthSqr();
			if (rangeSq < closeRangeSq)
			{	
				m_closePos = close;
				closeRangeSq = rangeSq;
			}
		}
		else
		{
			break;
		}
		
		distanceSoFar += segment->length;
		segment = nextSegment;
	}
	
	return m_closePos;
}

void Path::Copy( INextBot *bot, const Path &path )
{
	Invalidate();
	
	for( int i = 0; i < path.m_segmentCount; ++i )
	{
		m_path[i] = path.m_path[i];
	}
	m_segmentCount = path.m_segmentCount;

	OnPathChanged( bot, COMPLETE_PATH );
}

void Path::MoveCursorToClosestPosition( const Vector &pos, SeekType type, float alongLimit ) const
{
	if ( !IsValid() )
	{
		return;
	}
	
	if ( type == SEEK_ENTIRE_PATH || type == SEEK_AHEAD )
	{
		const Segment *segment;
		
		if ( type == SEEK_AHEAD )
		{
			if ( m_cursorData.segmentPrior )
			{
				segment = m_cursorData.segmentPrior;
			}
			else
			{
				segment = &m_path[ 0 ];
			}
		}
		else
		{
			segment = &m_path[ 0 ];
		}

		m_cursorData.pos = pos;
		m_cursorData.segmentPrior = segment;
		float closeRangeSq = 99999999999.9f;

		float distanceSoFar = 0.0f;	
		while( alongLimit == 0.0f || distanceSoFar <= alongLimit )
		{
			const Segment *nextSegment = NextSegment( segment );

			if ( nextSegment )
			{
				Vector close;
				CalcClosestPointOnLineSegment( pos, segment->pos, nextSegment->pos, close );
				
				float rangeSq = ( close - pos ).LengthSqr();
				if ( rangeSq < closeRangeSq )
				{	
					m_cursorData.pos = close;
					m_cursorData.segmentPrior = segment;
					
					closeRangeSq = rangeSq;
				}
			}
			else
			{
				break;
			}

			distanceSoFar += segment->length;
			segment = nextSegment;
		}

		segment = m_cursorData.segmentPrior;
				
		float t = ( m_cursorData.pos - segment->pos ).Length() / segment->length;

		m_cursorPos = segment->distanceFromStart + t * segment->length;	
		m_isCursorDataDirty = true;
	}
}

const Path::Data &Path::GetCursorData( void ) const
{
	ConVarRef NextBotPathSegmentInfluenceRadius( "nb_path_segment_influence_radius");
	if ( IsValid() )
	{
		if ( m_isCursorDataDirty )
		{
			const float epsilon = 0.0001f;
			if ( m_cursorPos < epsilon || m_segmentCount < 2 )
			{
				m_cursorData.pos = m_path[0].pos;
				m_cursorData.forward = m_path[0].forward;
				m_cursorData.curvature = m_path[0].curvature;
				m_cursorData.segmentPrior = &m_path[0];
			}
			else if ( m_cursorPos > GetLength() - epsilon )
			{
				m_cursorData.pos = m_path[ m_segmentCount-1 ].pos;
				m_cursorData.forward = m_path[ m_segmentCount-1 ].forward;
				m_cursorData.curvature = m_path[ m_segmentCount-1 ].curvature;
				m_cursorData.segmentPrior = &m_path[ m_segmentCount-1 ];
			}
			else
			{
				float lengthSoFar = 0.0f;
				const Segment *segment = &m_path[0];

				const Segment *next = NextSegment( segment );

				while( next )
				{
					float length = segment->length;

					if ( lengthSoFar + length >= m_cursorPos )
					{
						float overlap = m_cursorPos - lengthSoFar;
						float t = 1.0f;
						if ( length > 0.0f )
						{
							t = overlap / length;
						}
						
						m_cursorData.pos = segment->pos + t * ( next->pos - segment->pos );
						m_cursorData.forward = segment->forward + t * ( next->forward - segment->forward );
						m_cursorData.segmentPrior = segment;

						if ( overlap < NextBotPathSegmentInfluenceRadius.GetFloat() )
						{
							if ( length - overlap < NextBotPathSegmentInfluenceRadius.GetFloat() )
							{
								float startCurvature = segment->curvature * ( 1.0f - ( overlap / NextBotPathSegmentInfluenceRadius.GetFloat() ) );
								float endCurvature = next->curvature * ( 1.0f - ( ( length - overlap ) / NextBotPathSegmentInfluenceRadius.GetFloat() ) );

								m_cursorData.curvature = ( startCurvature + endCurvature ) / 2.0f;
							}
							else
							{
								m_cursorData.curvature = segment->curvature * ( 1.0f - ( overlap / NextBotPathSegmentInfluenceRadius.GetFloat() ) );
							}
						}
						else if ( length - overlap < NextBotPathSegmentInfluenceRadius.GetFloat() )
						{
							m_cursorData.curvature = next->curvature * ( 1.0f - ( ( length - overlap ) / NextBotPathSegmentInfluenceRadius.GetFloat() ) );
						}

						
						break;
					}

					lengthSoFar += length;

					segment = next;
					next = NextSegment( next );
				}
			}
			
			m_isCursorDataDirty = false;
		}
	}
	else
	{
		m_cursorData.pos = vec3_origin;
		m_cursorData.forward = Vector( 1.0f, 0, 0 );
		m_cursorData.curvature = 0.0f;
		m_cursorData.segmentPrior = NULL;
	}
	
	return m_cursorData;
}

void Path::ComputeAreaCrossing( INextBot *bot, const CNavArea *from, const Vector &fromPos, const CNavArea *to, Nav_DirType dir, Vector *crossPos ) const
{
	reinterpret_cast<INavArea*>(const_cast<CNavArea*>(from))->ComputeClosestPointInPortal( (INavArea*)to, dir, fromPos, crossPos );
	// move goal position into the goal area a bit to avoid running directly along the edge of an area against a wall, etc
	// don't do this unless area is against a wall - and what if our hull is wider than the area?
	AddDirectionVector( crossPos, dir, bot->GetBodyInterface()->GetHullWidth() / 2.0f );
}
