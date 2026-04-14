#include "INextBotPathFollow.h"
#include "INextBot.h"
#include "CTrace.h"
#include "IBaseCombatCharacter.h"
#include "INavMesh.h"
#include "CTempEntity.h"

class NextBotTraversableTraceFilter : public CTraceFilterSimples
{
public:
	NextBotTraversableTraceFilter( INextBot *bot, int when = 1 ) : 
	CTraceFilterSimples( (IHandleEntity*)bot->GetEntity(), COLLISION_GROUP_NONE )
    {
        m_bot = bot;
        m_when = when;
    }
	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
    {
        IBaseEntity *entity = GetEntFromEntHandle( pServerEntity );

        if ( m_bot->IsSelf( (CBaseCombatCharacter*)entity ) )
        {
            return false;
        }

        if ( CTraceFilterSimples::ShouldHitEntity( pServerEntity, contentsMask ) )
        {
            return !m_bot->GetLocomotionInterface()->IsEntityTraversable( (CBaseEntity*)entity, static_cast<ILocomotion::TraverseWhenType>(m_when) );
        }

        return false;
    }

private:
	bool IsStaticProp(CBaseHandle handle)
	{
		return (handle.GetSerialNumber() == (0x40000000 >> NUM_ENT_ENTRY_BITS));
	}

	IBaseEntity * GetEntFromEntHandle(IHandleEntity *pHandleEntity)
	{
		IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
		IBaseEntity *pEnt = reinterpret_cast<IBaseEntity*>(pUnk->GetBaseEntity());
		if (IsStaticProp(pEnt->GetRefEHandle()))
			return nullptr;

		return pEnt;
	}

	INextBot *m_bot;
	int m_when;
};

IPathFollower::IPathFollower(void) : 
	NextBotAllowAvoiding( "nb_allow_avoiding"),
	NextBotAllowClimbing( "nb_allow_climbing"),
	NextBotAllowGapJumping( "nb_allow_gap_jumping"),
	NextBotLadderAlignRange("nb_ladder_align_range")
{
    m_goal = nullptr;
    m_didAvoidCheck = false;
    m_avoidTimer.Invalidate();
    m_waitTimer.Invalidate();
    m_hindrance = nullptr;
    m_minLookAheadRange = -1.f;
    m_goalTolerance = 10.f;
}

IPathFollower::~IPathFollower()
{
}

void IPathFollower::Invalidate(void)
{
    Path::Invalidate();
    m_goal = nullptr;

    m_avoidTimer.Invalidate();
    m_waitTimer.Invalidate();
    m_hindrance = nullptr;
}

void IPathFollower::Draw(const Path::Segment *start) const
{
	if ( m_goal == NULL )
		return;

	// extend
	Path::Draw();
}

void IPathFollower::OnPathChanged(INextBot *bot, Path::ResultType result)
{
    m_goal = FirstSegment();
}

void IPathFollower::Update(INextBot *bot)
{
	ILocomotion *mover = bot->GetLocomotionInterface();
	
	if ( !IsValid() || m_goal == NULL )
	{
		return;
	}

	if ( !m_waitTimer.IsElapsed() )
	{
		return;
	}


	if ( LadderUpdate( bot ) )
	{
		return;
	}

	AdjustSpeed( bot );

	if ( CheckProgress( bot ) == false )
	{
		return;
	}

	Vector forward = m_goal->pos - mover->GetFeet();

	if ( m_goal->type == CLIMB_UP )
	{
		const Segment *next = NextSegment( m_goal );
		if ( next )
		{
			forward = next->pos - mover->GetFeet();
		}
	}

	forward.z = 0.0f;

	float goalRange = forward.NormalizeInPlace();

	Vector left( -forward.y, forward.x, 0.0f );

	if ( left.IsZero() )
	{
		mover->GetBot()->OnMoveToFailure( this, FAIL_STUCK );

		if ( GetAge() > 0.0f )
		{
			Invalidate();
		}

		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg("PathFollower: OnMoveToFailure( FAIL_STUCK ) because forward and left are ZERO\n");
		}

		return;
	}

	const Vector &normal = mover->GetGroundNormal();

	forward = CrossProduct( left, normal );

	left = CrossProduct( normal, forward );

	if ( !Climbing( bot, m_goal, forward, left, goalRange ) )
	{
		if ( !IsValid() )
		{
			return;
		}
		JumpOverGaps( bot, m_goal, forward, left, goalRange );
	}

	if ( !IsValid() )
	{
		return;
	}
	
	INavArea *myArea = (INavArea *)reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetLastKnownArea();
	bool isOnStairs = ( myArea && myArea->HasAttributes( NAV_MESH_STAIRS ) );

	float tooHighDistance = mover->GetMaxJumpHeight();

	if ( !m_goal->ladder && !mover->IsClimbingOrJumping() && !isOnStairs && m_goal->pos.z > mover->GetFeet().z + tooHighDistance )
	{
		const float closeRange = 25.0f; // 75.0f;
		Vector2D to( mover->GetFeet().x - m_goal->pos.x, mover->GetFeet().y - m_goal->pos.y );
		if ( mover->IsStuck() || to.IsLengthLessThan( closeRange ) )
		{
			const Path::Segment *next = NextSegment( m_goal );
			if ( mover->IsStuck() || !next || ( next->pos.z - mover->GetFeet().z > mover->GetMaxJumpHeight() ) || !mover->IsPotentiallyTraversable( mover->GetFeet(), next->pos ) )
			{
				mover->GetBot()->OnMoveToFailure( this, FAIL_FELL_OFF );

				if ( GetAge() > 0.0f )
				{
					Invalidate();
				}

				if(ukr_next_bot_debug.GetBool())
				{
					DevMsg("PathFollower: OnMoveToFailure( FAIL_FELL_OFF )\n");
				}

				mover->ClearStuckStatus( "Fell off path" );
				return;
			}
		}
	}


	Vector goalPos = m_goal->pos;

	forward = goalPos - mover->GetFeet();
	forward.z = 0.0f;
	float rangeToGoal = forward.NormalizeInPlace();

	left.x = -forward.y;
	left.y = forward.x;
	left.z = 0.0f;

	if ( true || m_goal != LastSegment() )
	{
		const float nearLedgeRange = 50.0f;
		if ( rangeToGoal > nearLedgeRange || ( m_goal && m_goal->type != CLIMB_UP ) )
		{
			goalPos = Avoid( bot, goalPos, forward, left );
		}
	}

	if ( mover->IsOnGround() )
	{	
		mover->FaceTowards( goalPos );
	}

	mover->Approach( goalPos );

	if ( m_goal && ( m_goal->type == CLIMB_UP || m_goal->type == JUMP_OVER_GAP ) )
	{
		bot->GetBodyInterface()->SetDesiredPosture( IBody::STAND );
	}

	if(ukr_next_bot_debug.GetBool())
	{
		const Segment *start = GetCurrentGoal();
		if(start)
		{
			start = PriorSegment(start);
		}

		Draw(start);
	}
}

bool IPathFollower::IsDiscontinuityAhead(INextBot *bot, Path::SegmentType type, float range) const
{
	if ( m_goal )
	{
		const Path::Segment *current = PriorSegment( m_goal );
		if ( current && current->type == type )
		{
			return true;
		}

		float rangeSoFar = ( m_goal->pos - bot->GetLocomotionInterface()->GetFeet() ).Length();

		for( const Segment *s = m_goal; s; s = NextSegment( s ) )
		{
			if ( rangeSoFar >= range )
			{
				break;
			}

			if ( s->type == type )
			{
				return true;
			}

			rangeSoFar += s->length;
		}
	}

	return false;
}

void IPathFollower::AdjustSpeed(INextBot *bot)
{
    ILocomotion *mover = bot->GetLocomotionInterface();
    if((m_goal && m_goal->type == JUMP_OVER_GAP) || !mover->IsOnGround())
    {
        mover->SetDesiredSpeed(mover->GetRunSpeed());
        return;
    }

    MoveCursorToClosestPosition(bot->GetPosition());
    const Path::Data &data = GetCursorData();

    mover->SetDesiredSpeed(mover->GetRunSpeed() + fabs(data.curvature) * (mover->GetWalkSpeed() - mover->GetRunSpeed()));
}

Vector IPathFollower::Avoid(INextBot *bot, const Vector &goalPos, const Vector &forward, const Vector &left)
{
	if ( !NextBotAllowAvoiding.GetBool() )
	{
		return goalPos;
	}

	if ( !m_avoidTimer.IsElapsed() )
	{
		return goalPos;
	}
	
	const float avoidInterval = 0.5f; // 1.0f;
	m_avoidTimer.Start( avoidInterval );

	ILocomotion *mover = bot->GetLocomotionInterface();

	if ( mover->IsClimbingOrJumping() || !mover->IsOnGround() )
	{
		return goalPos;
	}

	m_hindrance = FindBlocker( bot );
	if ( m_hindrance != NULL )
	{
		// wait 
		m_waitTimer.Start( avoidInterval * RandomFloat( 1.0f, 2.0f ) );

		return mover->GetFeet();
	}

	INavArea *area = (INavArea *)reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetLastKnownArea();
	if ( area && ( area->GetAttributes() & NAV_MESH_PRECISE ) )
	{
		return goalPos;
	}

	m_didAvoidCheck = true;

	trace_t result;
	NextBotTraceFilterOnlyActors filter( (IHandleEntity *)bot->GetEntity(), COLLISION_GROUP_NONE );

	IBody *body = bot->GetBodyInterface();
	unsigned int mask = body->GetSolidMask();

	const float size = body->GetHullWidth() / 4.0f;
	const float offset = size + 2.0f;

	float range = mover->IsRunning() ? 50.0f : 30.0f;
	range *= reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetModelScale();

	m_hullMin = Vector( -size, -size, mover->GetStepHeight() + 0.1f );
	
	m_hullMax = Vector( size, size, body->GetCrouchHullHeight() );
	
	Vector nextStepHullMin( -size, -size, 2.0f * mover->GetStepHeight() + 0.1f );

	IBaseEntity *door = nullptr;

	// check left side
	m_leftFrom = mover->GetFeet() + offset * left;
	m_leftTo = m_leftFrom + range * forward;

	m_isLeftClear = true;
	float leftAvoid = 0.0f;

	NextBotTraversableTraceFilter traverseFilter( bot );
	util_TraceHull( m_leftFrom, m_leftTo, m_hullMin, m_hullMax, mask, &traverseFilter, &result );
	if ( result.fraction < 1.0f || result.startsolid )
	{
		if ( result.startsolid )
		{
			result.fraction = 0.0f;
		}

		leftAvoid = clamp( 1.0f - result.fraction, 0.0f, 1.0f );

		m_isLeftClear = false;

		if ( result.DidHitNonWorldEntity() )
		{
			door = (IBaseEntity *)result.m_pEnt;
		}

		// check for steps
/*		float firstHit = result.fraction;
 		util_TraceHull( m_leftFrom, m_leftTo, nextStepHullMin, m_hullMax, mask, &filter, &result );
 		if ( result.fraction <= firstHit ) //+ mover->GetStepHeight()/2.0f )
 		{
 			// it's not a step - we hit something
 			m_isLeftClear = false;
 		}*/
	}

	m_rightFrom = mover->GetFeet() - offset * left;
	m_rightTo = m_rightFrom + range * forward;

	m_isRightClear = true;
	float rightAvoid = 0.0f;

	util_TraceHull( m_rightFrom, m_rightTo, m_hullMin, m_hullMax, mask, &traverseFilter, &result );
	if ( result.fraction < 1.0f || result.startsolid )
	{
		if ( result.startsolid )
		{
			result.fraction = 0.0f;
		}

		rightAvoid = clamp( 1.0f - result.fraction, 0.0f, 1.0f );

		m_isRightClear = false;

		if ( !door && result.DidHitNonWorldEntity() )
		{
			door = (IBaseEntity*)result.m_pEnt;
		}

		// check for steps
/* 		float firstHit = result.fraction;
 		util_TraceHull( m_rightFrom, m_rightTo, nextStepHullMin, m_hullMax, mask, &filter, &result );
 		if ( result.fraction <= firstHit ) // + mover->GetStepHeight()/2.0f)
 		{
 			// it's not a step - we hit something
 			m_isRightClear = false;
 		}*/
	}

	Vector adjustedGoal = goalPos;

	if ( door && !m_isLeftClear && !m_isRightClear )
	{
		Vector forward, right, up;
		AngleVectors( door->GetAbsAngles(), &forward, &right, &up );

		const float doorWidth = 100.0f;
		Vector doorEdge = door->GetAbsOrigin() - doorWidth * right;

		adjustedGoal.x = doorEdge.x;
		adjustedGoal.y = doorEdge.y;

		m_avoidTimer.Invalidate();
	}
	else if ( !m_isLeftClear || !m_isRightClear )
	{
		float avoidResult = 0.0f;
		if ( m_isLeftClear )
		{
			avoidResult = -rightAvoid;
		}
		else if (m_isRightClear)
		{
			avoidResult = leftAvoid;
		}
		else
		{
			const float equalTolerance = 0.01f;
			if ( fabs( rightAvoid - leftAvoid ) < equalTolerance )
			{
				return adjustedGoal;
			} 
			else if ( rightAvoid > leftAvoid )
			{
				avoidResult = -rightAvoid;
			}
			else
			{
				avoidResult = leftAvoid;
			}
		}
		
		Vector avoidDir = 0.5f * forward - left * avoidResult;
		avoidDir.NormalizeInPlace();
		
		adjustedGoal = mover->GetFeet() + 100.0f * avoidDir;
		
		m_avoidTimer.Invalidate();
	}
	
	return adjustedGoal;
}

bool IPathFollower::Climbing(INextBot *bot, const Path::Segment *goal, const Vector &forward, const Vector &left, float goalRange)
{
	if ( !NextBotAllowClimbing.GetBool())
	{
		return false;
	}

	ILocomotion *mover = bot->GetLocomotionInterface();
	IBody *body = bot->GetBodyInterface();
	INavArea *myArea = (INavArea *)reinterpret_cast<IBaseCombatCharacter*>(bot->GetEntity())->GetLastKnownArea();

	Vector climbDirection = forward;
	climbDirection.z = 0.0f;
	climbDirection.NormalizeInPlace();

	const float ledgeLookAheadRange = body->GetHullWidth() - 1;

	if ( mover->IsClimbingOrJumping() || mover->IsAscendingOrDescendingLadder() || !mover->IsOnGround() )
	{
		return false;
	}

	if ( m_goal == NULL )
	{
		return false;
	}

	float climbUpLedgeHeightDelta = -1.0f;
	const float climbUpLedgeTolerance = body->GetCrouchHullHeight();

	if ( m_goal->type == CLIMB_UP )
	{
		const Segment *afterClimb = NextSegment( m_goal );
		if ( afterClimb && afterClimb->area )
		{
			// find closest point on climb-destination area
			Vector nearClimbGoal;
			afterClimb->area->GetClosestPointOnArea( mover->GetFeet(), &nearClimbGoal );

			climbDirection = nearClimbGoal - mover->GetFeet();
			climbUpLedgeHeightDelta = climbDirection.z;
			climbDirection.z = 0.0f;
			climbDirection.NormalizeInPlace();
		}
	}

	// don't try to climb up stairs
	if ( m_goal->area->HasAttributes( NAV_MESH_STAIRS ) || ( myArea && myArea->HasAttributes( NAV_MESH_STAIRS ) ) )
	{
		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg( "%3.2f: %s ON STAIRS\n", g_pGlobals->curtime, bot->GetDebugIdentifier() );
		}

		return false;
	}

	const Segment *current = PriorSegment( m_goal );
	if ( current == NULL )
	{
		return false;
	}

	Vector toGoal = m_goal->pos - mover->GetFeet();
	toGoal.NormalizeInPlace();

	if ( toGoal.z < mover->GetTraversableSlopeLimit() && !mover->IsStuck() && m_goal->type != CLIMB_UP && mover->IsPotentiallyTraversable( mover->GetFeet(), mover->GetFeet() + 2.0f * ledgeLookAheadRange * toGoal, ILocomotion::IMMEDIATELY ) )
	{
		return false;
	}

	// can't do this - we have to find the ledge to deal with breakable railings
#if 0
	// If our path requires a climb, do the climb.
	// This solves some issues where there are several possible climbable ledges at a given
	// location, and we need to know which ledge to climb - just use the preplanned path's choice.
	const Segment *ledge = NextSegment( m_goal );
	if ( m_goal->type == CLIMB_UP && ledge )
	{
		const float startClimbRange = body->GetHullWidth();
		if ( ( m_goal->pos - mover->GetFeet() ).IsLengthLessThan( startClimbRange ) )
		{
			mover->ClimbUpToLedge( ledge->pos, climbDirection );
			return true;
		}
	}
#endif 

	const float climbLookAheadRange = 150.0f;
	bool isPlannedClimbImminent = false;
	float plannedClimbZ = 0.0f;
	for( const Segment *s = current; s; s = NextSegment( s ) )
	{
		if ( s != current && ( s->pos - mover->GetFeet() ).AsVector2D().IsLengthGreaterThan( climbLookAheadRange ) )
		{
			break;
		}

		if ( s->type == CLIMB_UP )
		{
			isPlannedClimbImminent = true;

			const Segment *next = NextSegment( s );
			if ( next )
			{
				plannedClimbZ = next->pos.z;
			}
			break;
		}
	}

	unsigned int mask = body->GetSolidMask();
	trace_t result;
	NextBotTraversableTraceFilter filter( bot, ILocomotion::IMMEDIATELY );

	const float hullWidth = body->GetHullWidth();
	const float halfSize = hullWidth / 2.0f;
	const float minHullHeight = body->GetCrouchHullHeight();
	const float minLedgeHeight = mover->GetStepHeight() + 0.1f;

	Vector skipStepHeightHullMin( -halfSize, -halfSize, minLedgeHeight );

	Vector skipStepHeightHullMax( halfSize, halfSize, minHullHeight + 0.1f );


	float ceilingFraction;

	Vector feet( mover->GetFeet() );
	Vector ceiling( feet + Vector( 0, 0, mover->GetMaxJumpHeight() ) );
	util_TraceHull( feet, ceiling, skipStepHeightHullMin, skipStepHeightHullMax, mask, &filter, &result );
	ceilingFraction = result.fraction;
	bool isBackupTraceUsed = false;
	if ( ceilingFraction < 1.0f || result.startsolid )
	{
		trace_t backupTrace;
		const float backupDistance = hullWidth * 0.25f;	// The IsPotentiallyTraversable check this replaces uses a 1/4 hull width trace
		Vector backupFeet( feet - climbDirection * backupDistance );
		Vector backupCeiling( backupFeet + Vector( 0, 0, mover->GetMaxJumpHeight() ) );
		util_TraceHull( backupFeet, backupCeiling, skipStepHeightHullMin, skipStepHeightHullMax, mask, &filter, &backupTrace );
		if ( !backupTrace.startsolid && backupTrace.fraction > ceilingFraction )
		{
			bot->DebugConColorMsg( NEXTBOT_PATH, Color( 255, 255, 255, 255 ), "%s backing up when looking for max ledge height\n", bot->GetDebugIdentifier() );
			result = backupTrace;
			ceilingFraction = result.fraction;
			feet = backupFeet;
			ceiling = backupCeiling;
			isBackupTraceUsed = true;
		}
	}

	float maxLedgeHeight = ceilingFraction * mover->GetMaxJumpHeight();

	if ( maxLedgeHeight <= mover->GetStepHeight() )
	{
		return false;
	}

	Vector climbHullMax( halfSize, halfSize, maxLedgeHeight );

	Vector ledgePos = feet;

	util_TraceHull( feet, feet + climbDirection * ledgeLookAheadRange, skipStepHeightHullMin, climbHullMax, mask, &filter, &result );

	bool wasPotentialLedgeFound = result.DidHit() && !result.startsolid;
	if ( wasPotentialLedgeFound )
	{		
		CBaseEntity *obstacle = result.m_pEnt;
		if ( !result.DidHitNonWorldEntity() || bot->IsAbleToClimbOnto( obstacle ) )
		{
			if(ukr_next_bot_debug.GetBool())
			{
				DevMsg("%3.2f: %s at potential ledge climb\n", g_pGlobals->curtime, bot->GetDebugIdentifier() );
			}

			float ledgeFrontWallDepth = ledgeLookAheadRange * result.fraction;

			float minLedgeDepth = body->GetHullWidth() / 2.0f; // 5.0f;
			if ( m_goal->type == CLIMB_UP )
			{
				const Segment *afterClimb = NextSegment( m_goal );
				if ( afterClimb && afterClimb->area )
				{
					Vector depthVector = climbDirection * minLedgeDepth;
					depthVector.z = 0;
					if ( fabs( depthVector.x ) > afterClimb->area->GetSizeX() )
					{
						depthVector.x = (depthVector.x > 0) ? afterClimb->area->GetSizeX() : -afterClimb->area->GetSizeX();
					}
					if ( fabs( depthVector.y ) > afterClimb->area->GetSizeY() )
					{
						depthVector.y = (depthVector.y > 0) ? afterClimb->area->GetSizeY() : -afterClimb->area->GetSizeY();
					}

					float areaDepth = depthVector.NormalizeInPlace();
					minLedgeDepth = MIN( minLedgeDepth, areaDepth );
				}
			}

			float ledgeHeight = minLedgeHeight;
			const float ledgeHeightIncrement = 0.5f * mover->GetStepHeight();

			bool foundWall = false;
			bool foundLedge = false;
			
			float ledgeTopLookAheadRange = ledgeLookAheadRange;

			Vector climbHullMin( -halfSize, -halfSize, 0.0f );
			Vector climbHullMax( halfSize, halfSize, minHullHeight );

			Vector wallPos;
			float wallDepth = 0.0f;

			bool isLastIteration = false;
			while( true )
			{				
				util_TraceHull( feet + Vector( 0, 0, ledgeHeight ), feet + Vector( 0, 0, ledgeHeight ) + climbDirection * ledgeTopLookAheadRange, climbHullMin, climbHullMax, mask, &filter, &result );

				float traceDepth = ledgeTopLookAheadRange * result.fraction;

				if ( !result.startsolid )
				{
					if ( foundWall )
					{
						if ( ( traceDepth - ledgeFrontWallDepth ) > minLedgeDepth )
						{
							bool isUsable = true;

							ledgePos = result.endpos;

							util_TraceHull( ledgePos, ledgePos + Vector( 0, 0, -ledgeHeightIncrement ), climbHullMin, climbHullMax, mask, &filter, &result );

							ledgePos = result.endpos;

							const float MinGroundNormal = 0.7f;
							if ( result.allsolid || !result.DidHit() || result.plane.normal.z < MinGroundNormal )
							{
								isUsable = false;
							}
							else
							{
								if ( climbUpLedgeHeightDelta > 0.0f )
								{
									if ( result.DidHitWorld() )
									{
										float potentialLedgeHeight = result.endpos.z - feet.z;
										if ( fabs(potentialLedgeHeight - climbUpLedgeHeightDelta) > climbUpLedgeTolerance )
										{
											isUsable = false;
										}
									}
								}
							}

							if ( isUsable )
							{
								Vector validLedgePos = ledgePos;
								const float edgeTolerance = 4.0f;
								const float maxBackUp = hullWidth;
								float backUpSoFar = edgeTolerance;
								Vector testPos = ledgePos;

								while( backUpSoFar < maxBackUp )
								{
									testPos -= edgeTolerance * climbDirection;
									backUpSoFar += edgeTolerance;

									util_TraceHull( testPos, testPos + Vector( 0, 0, -ledgeHeightIncrement ), climbHullMin, climbHullMax, mask, &filter, &result );

									if ( result.DidHit() && result.plane.normal.z >= MinGroundNormal )
									{
										ledgePos = result.endpos;
									}
									else
									{
										break;
									}
								}

								ledgePos += climbDirection * halfSize;

								Vector climbHullMinStep( climbHullMin );
								util_TraceHull( validLedgePos, ledgePos, climbHullMinStep, climbHullMax, mask, &filter, &result );

								ledgePos = result.endpos;

								util_TraceHull( ledgePos + Vector( 0, 0, 18.f ), ledgePos, climbHullMin, climbHullMax, mask, &filter, &result );
								if ( !result.startsolid )
								{
									ledgePos = result.endpos;
								}
							}

							if ( isUsable )
							{
								foundLedge = true;
								break;
							}
						}
					}
					else if ( result.DidHit() )
					{
						foundWall = true;
						wallDepth = traceDepth;

						float minTraceDepth = traceDepth + minLedgeDepth + 0.1f;

						if ( ledgeTopLookAheadRange < minTraceDepth )
						{
							ledgeTopLookAheadRange = minTraceDepth;
						}

						if(ukr_next_bot_debug.GetBool())
						{
							DevMsg("%3.2f: Climbing - found wall.\n", g_pGlobals->curtime);
						}
					}
					else if ( ledgeHeight > body->GetCrouchHullHeight() && !isPlannedClimbImminent )
					{
						if(ukr_next_bot_debug.GetBool())
						{
							DevMsg("%3.2f: Climbing - skipping overhead climb we can walk/crawl under.\n", g_pGlobals->curtime);
						}
						break;
					}
				}

				ledgeHeight += ledgeHeightIncrement;

				if ( ledgeHeight >= maxLedgeHeight )
				{
					if ( isLastIteration )
					{
						break;
					}

					isLastIteration = true;
					ledgeHeight = maxLedgeHeight;
				}
			}
			
			if ( foundLedge )
			{
				if(ukr_next_bot_debug.GetBool())
				{
					DevMsg( "%3.2f: STARTING LEDGE CLIMB UP\n", g_pGlobals->curtime );
				}

				if ( !mover->ClimbUpToLedge( ledgePos, climbDirection, obstacle ) )
				{
					return false;
				}

				return true;
			}
			else if(ukr_next_bot_debug.GetBool())
			{
				DevMsg("%3.2f: CANT FIND LEDGE TO CLIMB\n", g_pGlobals->curtime);
			}
		}
	}

	return false;
}

bool IPathFollower::JumpOverGaps(INextBot *bot, const Path::Segment *goal, const Vector &forward, const Vector &left, float goalRange)
{
	ILocomotion *mover = bot->GetLocomotionInterface();
	IBody *body = bot->GetBodyInterface();

	if ( !NextBotAllowGapJumping.GetBool() )
	{
		return false;
	}

	if ( mover->IsClimbingOrJumping() || mover->IsAscendingOrDescendingLadder() || !mover->IsOnGround() )
	{
		return false;
	}
	
	if ( !body->IsActualPosture( IBody::STAND ) )
	{
		return false;
	}

	if ( m_goal == NULL )
	{
		return false;
	}

	trace_t result;
	NextBotTraversableTraceFilter filter( bot, ILocomotion::IMMEDIATELY );

	const float hullWidth = ( body ) ? body->GetHullWidth() : 1.0f;

	const Segment *current = PriorSegment( m_goal );
	if ( current == NULL )
	{
		return false;
	}

	const float minGapJumpRange = 2.0f * hullWidth;

	const Segment *gap = NULL;

	if ( current->type == JUMP_OVER_GAP )
	{
		gap = current;
	}
	else
	{
		float searchRange = goalRange;
		for( const Segment *s = m_goal; s; s = NextSegment( s ) )
		{
			if ( searchRange > minGapJumpRange )
			{
				break;
			}

			if ( s->type == JUMP_OVER_GAP )
			{
				gap = s;
				break;
			}

			searchRange += s->length;
		}
	}

	if ( gap )
	{
		float halfWidth = hullWidth/2.0f;

		if ( mover->IsGap( mover->GetFeet() + halfWidth * gap->forward, gap->forward ) )
		{
			const Segment *landing = NextSegment( gap );
			if ( landing )
			{
				mover->JumpAcrossGap( landing->pos, landing->forward );

				m_goal = landing;

				if(ukr_next_bot_debug.GetBool())
				{
					DevMsg( "%3.2f: GAP JUMP\n", g_pGlobals->curtime );
				}

				return true;
			}
		}
	}

	return false;
}

bool IPathFollower::LadderUpdate(INextBot *bot)
{
	ILocomotion *mover = bot->GetLocomotionInterface();
	IBody *body = bot->GetBodyInterface();
	
	if ( mover->IsUsingLadder() )
	{
		return true;
	}

	if ( m_goal->ladder == NULL )
	{
		if ( reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetMoveType() == MOVETYPE_LADDER )
		{
			// 'current' is the segment we are on/just passed over
			const Segment *current = PriorSegment( m_goal );
			if ( current == NULL )
			{
				return false;
			}

			const float ladderLookAheadRange = 50.0f;
			for( const Segment *s = current; s; s = NextSegment( s ) )
			{
				if ( s != current && ( s->pos - mover->GetFeet() ).AsVector2D().IsLengthGreaterThan( ladderLookAheadRange ) )
				{
					break;
				}

				if ( s->ladder != NULL && s->how == GO_LADDER_DOWN && s->ladder->m_length > mover->GetMaxJumpHeight() )
				{
					float destinationHeightDelta = s->pos.z - mover->GetFeet().z;
					if ( fabs(destinationHeightDelta) < mover->GetMaxJumpHeight() )
					{
						m_goal = s;
						break;
					}
				}
			}
		}

		if ( m_goal->ladder == NULL )
		{
			return false;
		}
	}
	
	const float mountRange = 25.0f;

	if ( m_goal->how == GO_LADDER_UP )
	{
		if ( !mover->IsUsingLadder() && mover->GetFeet().z > m_goal->ladder->m_top.z - mover->GetStepHeight() )
		{
			m_goal = NextSegment( m_goal );
			return false;
		}

		Vector2D to = ( m_goal->ladder->m_bottom - mover->GetFeet() ).AsVector2D();

		body->AimHeadTowards( m_goal->ladder->m_top - 50.0f * m_goal->ladder->GetNormal() + Vector( 0, 0, body->GetCrouchHullHeight() ), 
							  IBody::CRITICAL, 
							  2.0f, 
							  NULL,
							  "Mounting upward ladder" );

		float range = to.NormalizeInPlace();
		if ( range < NextBotLadderAlignRange.GetFloat() )
		{
			Vector2D ladderNormal2D = m_goal->ladder->GetNormal().AsVector2D();
			float dot = DotProduct2D( ladderNormal2D, to );

			const float cos5 = 0.9f;
			if ( dot < -cos5 )
			{
				mover->Approach( m_goal->ladder->m_bottom );

				if ( range < mountRange )
				{
					mover->ClimbLadder( (CNavLadder*)m_goal->ladder, (CNavArea *)m_goal->area );
				}				
			}
			else
			{
				Vector myPerp( -to.y, to.x, 0.0f );
				Vector2D ladderPerp2D( -ladderNormal2D.y, ladderNormal2D.x );

				Vector goal = m_goal->ladder->m_bottom;
				
				float alignRange = NextBotLadderAlignRange.GetFloat();
				
				if ( dot < 0.0f )
				{
					alignRange = mountRange + (1.0f + dot) * (alignRange - mountRange);
				}
				
				goal.x -= alignRange * to.x;
				goal.y -= alignRange * to.y;				
				
				if ( DotProduct2D( to, ladderPerp2D ) < 0.0f )
				{
					goal += 10.0f * myPerp;
				}
				else
				{
					goal -= 10.0f * myPerp;
				}
				
				mover->Approach( goal );
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		if ( mover->GetFeet().z < m_goal->ladder->m_bottom.z + mover->GetStepHeight() )
		{
			m_goal = NextSegment( m_goal );
		}
		else
		{
			Vector mountPoint = m_goal->ladder->m_top + 0.5f * body->GetHullWidth() * m_goal->ladder->GetNormal();
			Vector2D to = ( mountPoint - mover->GetFeet() ).AsVector2D();

			body->AimHeadTowards( m_goal->ladder->m_bottom + 50.0f * m_goal->ladder->GetNormal() + Vector( 0, 0, body->GetCrouchHullHeight() ), 
								  IBody::CRITICAL, 
								  1.0f, 
								  NULL,
								  "Mounting downward ladder" );

			float range = to.NormalizeInPlace();

			if ( range < mountRange || reinterpret_cast<IBaseCombatCharacter *>(bot->GetEntity())->GetMoveType() == MOVETYPE_LADDER )
			{
				mover->DescendLadder( (CNavLadder *)m_goal->ladder, (CNavArea *)m_goal->area );

				m_goal = NextSegment( m_goal );
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

CBaseEntity *IPathFollower::FindBlocker(INextBot *bot)
{
	IIntention *think = bot->GetIntentionInterface();

	if ( think->IsHindrance( bot, IS_ANY_HINDRANCE_POSSIBLE ) != ANSWER_YES )
		return NULL;

	ILocomotion *mover = bot->GetLocomotionInterface();
	IBody *body = bot->GetBodyInterface();

	trace_t result;
	NextBotTraceFilterOnlyActors filter( (IHandleEntity*)bot->GetEntity(), COLLISION_GROUP_NONE );

	const float size = body->GetHullWidth() / 4.0f;
	Vector blockerMins( -size, -size, mover->GetStepHeight() );
	Vector blockerMaxs( size, size, body->GetCrouchHullHeight() );

	Vector from = mover->GetFeet();
	float range = 0.0f;

	const float maxHindranceRangeAlong = 750.0f;

	MoveCursorToClosestPosition( mover->GetFeet() );

	for( const Segment *s = GetCursorData().segmentPrior; s && range < maxHindranceRangeAlong; s = NextSegment( s ) )
	{
		Vector traceForward = s->pos - from;
		float traceRange = traceForward.NormalizeInPlace();

		const float minTraceRange = 2.0f * body->GetHullWidth();
		if ( traceRange < minTraceRange )
		{
			traceRange = minTraceRange;
		}

		util_TraceHull( from, from + traceRange * traceForward, blockerMins, blockerMaxs, body->GetSolidMask(), &filter, &result );

		if ( result.DidHitNonWorldEntity() )
		{
			Vector toBlocker = reinterpret_cast<IBaseEntity*>(result.m_pEnt)->GetAbsOrigin() - bot->GetLocomotionInterface()->GetFeet();

			Vector alongPath = s->pos - from;
			alongPath.z = 0.0f;

			if ( DotProduct( toBlocker, alongPath ) > 0.0f )
			{
				if ( think->IsHindrance( bot, result.m_pEnt ) == ANSWER_YES )
				{
					return result.m_pEnt;
				}
			}
		}

		from = s->pos;
		range += s->length;
	}

	return NULL;
}

bool IPathFollower::CheckProgress(INextBot *bot)
{
	ILocomotion *mover = bot->GetLocomotionInterface();

	const Path::Segment *pSkipToGoal = NULL;
	if ( m_minLookAheadRange > 0.0f )
	{
		pSkipToGoal = m_goal;
		const Vector &myFeet = mover->GetFeet();
		while( pSkipToGoal && pSkipToGoal->type == ON_GROUND && mover->IsOnGround() )
		{
			if ( ( pSkipToGoal->pos - myFeet ).IsLengthLessThan( m_minLookAheadRange ) )
			{
				const Path::Segment *nextSegment = NextSegment( pSkipToGoal );

				if ( !nextSegment || nextSegment->type != ON_GROUND )
				{
					break;
				}

				if ( nextSegment->pos.z > myFeet.z + mover->GetStepHeight() )
				{
					break;
				}

				if ( mover->IsPotentiallyTraversable( myFeet, nextSegment->pos ) && !mover->HasPotentialGap( myFeet, nextSegment->pos ) )
				{
					pSkipToGoal = nextSegment;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}

		if ( pSkipToGoal == m_goal )
		{
			pSkipToGoal = NULL;
		}
	}

	if ( IsAtGoal( bot ) )
	{
		const Path::Segment *nextSegment = pSkipToGoal ? pSkipToGoal : NextSegment( m_goal );

		if ( nextSegment == NULL )
		{
			if ( mover->IsOnGround() )
			{
				mover->GetBot()->OnMoveToSuccess( this );
				if(ukr_next_bot_debug.GetBool())
				{
					DevMsg("IPathFollower: OnMoveToSuccess\n");
				}

				if ( GetAge() > 0.0f )
				{
					Invalidate();
				}

				return false;
			}
		}
		else
		{
			m_goal = nextSegment;
			if(ukr_next_bot_debug.GetBool() && !mover->IsPotentiallyTraversable(mover->GetFeet(), nextSegment->pos))
			{
				DevMsg("PathFollower: path to my goal is blocked by something\n");
			}
		}
	}

	return true;
}

bool IPathFollower::IsAtGoal(INextBot *bot) const
{
	ILocomotion *mover = bot->GetLocomotionInterface();
	IBody *body = bot->GetBodyInterface();

	const Segment *current = PriorSegment( m_goal );
	Vector toGoal = m_goal->pos - mover->GetFeet();

	if ( current == NULL )
	{
		return true;
	}
	else if ( m_goal->type == DROP_DOWN )
	{
		const Segment *landing = NextSegment( m_goal );

		if ( landing == NULL )
		{
			return true;
		}		
		else
		{
			if ( mover->GetFeet().z - landing->pos.z < mover->GetStepHeight() )
			{
				return true;
			}
		}		
	}
	else if ( m_goal->type == CLIMB_UP )
	{
		const Segment *landing = NextSegment( m_goal );

		if ( landing == NULL )
		{
			return true;
		}		
		else if ( /*!mover->IsOnGround() &&*/ mover->GetFeet().z > m_goal->pos.z + mover->GetStepHeight() )
		{
			return true;
		}
		/*else if ( mover->IsOnGround() )
		{
			const float rangeTolerance = 10.0f;
			if ( toGoal.AsVector2D().IsLengthLessThan( rangeTolerance ) )
			{
				return true;
			}
		}*/
	}
	else
	{
		const Segment *next = NextSegment( m_goal );

		if ( next )
		{
			Vector2D dividingPlane;

			if ( current->ladder )
			{
				dividingPlane = m_goal->forward.AsVector2D();
			}
			else
			{
				dividingPlane = current->forward.AsVector2D() + m_goal->forward.AsVector2D();
			}

			if ( DotProduct2D( toGoal.AsVector2D(), dividingPlane ) < 0.0001f && abs( toGoal.z ) < body->GetStandHullHeight() )
			{	
				if ( toGoal.z < mover->GetStepHeight() && ( mover->IsPotentiallyTraversable( mover->GetFeet(), next->pos ) && !mover->HasPotentialGap( mover->GetFeet(), next->pos ) ) )
				{
					return true;
				}
			}
		}

		if ( toGoal.AsVector2D().IsLengthLessThan( m_goalTolerance ) )
		{
			return true;
		}
	}

	return false;	
}
