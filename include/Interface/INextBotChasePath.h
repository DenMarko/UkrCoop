#ifndef _HEADER_NEXT_BOT_CHASEPATH_INCLUDE_
#define _HEADER_NEXT_BOT_CHASEPATH_INCLUDE_

#include "INextBotPathFollow.h"

/*Шлях погоні*/
class IChasePath : public IPathFollower
{
public:
	enum SubjectChaseType
	{
		LEAD_SUBJECT,
		DONT_LEAD_SUBJECT
	};
	IChasePath( SubjectChaseType chaseHow = DONT_LEAD_SUBJECT );

	virtual ~IChasePath() { }

	virtual void Update( INextBot *bot, IBaseEntity *subject, const IPathCost &cost, Vector *pPredictedSubjectPos = NULL );	// update path to chase target and move bot along path

	virtual float GetLeadRadius( void ) const;			    // range where movement leading begins - beyond this just head right for the subject
	virtual float GetMaxPathLength( void ) const;		    // return maximum path length
	virtual Vector PredictSubjectPosition( INextBot *bot, IBaseEntity *subject ) const;	// try to cutoff our chase subject, knowing our relative positions and velocities
	virtual bool IsRepathNeeded( INextBot *bot, IBaseEntity *subject ) const;			// return true if situation has changed enough to warrant recomputing the current path

	virtual float GetLifetime( void ) const;			    // Return duration this path is valid. Path will become invalid at its earliest opportunity once this duration elapses. Zero = infinite lifetime

	virtual void Invalidate( void );					    // (EXTEND) cause the path to become invalid
private:
	void RefreshPath( INextBot *bot, IBaseEntity *subject, const IPathCost &cost, Vector *pPredictedSubjectPos );

	CountdownTimers m_failTimer;							// throttle re-pathing if last path attempt failed
	CountdownTimers m_throttleTimer;						// require a minimum time between re-paths
	CountdownTimers m_lifetimeTimer;
	EHANDLE m_lastPathSubject;							    // the subject used to compute the current/last path
	SubjectChaseType m_chaseHow;
};

inline IChasePath::IChasePath( SubjectChaseType chaseHow )
{
	m_failTimer.Invalidate();
	m_throttleTimer.Invalidate();
	m_lifetimeTimer.Invalidate();
	m_lastPathSubject = NULL;
	m_chaseHow = chaseHow;
}

inline void IChasePath::Update(INextBot *bot, IBaseEntity *subject, const IPathCost &cost, Vector *pPredictedSubjectPos)
{
    RefreshPath(bot, subject, cost, pPredictedSubjectPos);
    IPathFollower::Update(bot);
}

inline float IChasePath::GetLeadRadius(void) const
{
    return 500.0f;
}

inline float IChasePath::GetMaxPathLength( void ) const
{
	// no limit
	return 0.0f;
}

inline bool IChasePath::IsRepathNeeded(INextBot *bot, IBaseEntity *subject) const
{
	// the closer we get, the more accurate our path needs to be
	Vector to = reinterpret_cast<IBaseEntity*>(subject)->GetAbsOrigin() - bot->GetPosition();

	const float minTolerance = 0.0f; // 25.0f;
	const float toleranceRate = 0.33f; // 1.0f; // 0.15f;

	float tolerance = minTolerance + toleranceRate * to.Length();

	return ( reinterpret_cast<IBaseEntity*>(subject)->GetAbsOrigin() - GetEndPosition() ).IsLengthGreaterThan( tolerance );
}

inline float IChasePath::GetLifetime( void ) const
{
	// infinite duration
	return 0.0f;
}

inline void IChasePath::Invalidate( void )
{
	// path is gone, repath at earliest opportunity
	m_throttleTimer.Invalidate();
	m_lifetimeTimer.Invalidate();

	// extend
	IPathFollower::Invalidate();
}

inline void IChasePath::RefreshPath(INextBot *bot, IBaseEntity *subject, const IPathCost &cost, Vector *pPredictedSubjectPos)
{
	ILocomotion *mover = bot->GetLocomotionInterface();

	// don't change our path if we're on a ladder
	if ( IsValid() && mover->IsUsingLadder() )
	{
		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg("%3.2f: bot(#%d) IChasePath::RefreshPath failed. Bot is on a ladder.\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex());
		}

		// don't allow repath until a moment AFTER we have left the ladder
		m_throttleTimer.Start( 1.0f );

		return;
	}

	if ( subject == NULL )
	{
		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg("%3.2f: bot(#%d) CasePath::RefreshPath failed. No subject.\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex());
		}

		return;
	}

	if ( !m_failTimer.IsElapsed() )
	{
		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg("%3.2f: bot(#%d) ChasePath::RefreshPath failed. Fail timer not elapsed.\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex());
		}

		return;
	}

	// if our path subject changed, repath immediately
	if ( (CBaseEntity *)subject != m_lastPathSubject )
	{
		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg("%3.2f: bot(#%d) Chase path subject changed (from %p to %p).\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex(), m_lastPathSubject.Get(), subject);
		}

		Invalidate();

		// new subject, fresh attempt
		m_failTimer.Invalidate();
	}

	if ( IsValid() && !m_throttleTimer.IsElapsed() )
	{
		if(ukr_next_bot_debug.GetBool())
		{
			DevMsg("%3.2f: bot(#%d) ChasePath::RefreshPath failed. Rate throttled.\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex());
		}

		return;
	}

	if ( IsValid() && m_lifetimeTimer.HasStarted() && m_lifetimeTimer.IsElapsed() )
	{
		// this path's lifetime has elapsed
		Invalidate();
	}
	
	if ( !IsValid() || IsRepathNeeded( bot, subject ) )
	{
		// the situation has changed - try a new path
		bool isPath;
		Vector pathTarget = reinterpret_cast<IBaseEntity*>(subject)->GetAbsOrigin();

		if ( m_chaseHow == LEAD_SUBJECT )
		{
			pathTarget = pPredictedSubjectPos ? *pPredictedSubjectPos : PredictSubjectPosition( bot, subject );
			isPath = Compute( bot, pathTarget, cost, GetMaxPathLength() );
		}
		else if ( reinterpret_cast<IBaseEntity*>(subject)->MyCombatCharacterPointer() && reinterpret_cast<IBaseCombatCharacter*>(reinterpret_cast<IBaseEntity*>(subject)->MyCombatCharacterPointer())->GetLastKnownArea() )
		{
			isPath = Compute( bot, reinterpret_cast<IBaseEntity*>(subject)->MyCombatCharacterPointer(), cost, GetMaxPathLength() );
		}
		else
		{
			isPath = Compute( bot, pathTarget, cost, GetMaxPathLength() );
		}

		if ( isPath )
		{
			if(ukr_next_bot_debug.GetBool())
			{
				DevMsg("%3.2f: bot(#%d) REPATH\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex());
			}

			m_lastPathSubject = (CBaseEntity*)subject;

			const float minRepathInterval = 0.5f;
			m_throttleTimer.Start( minRepathInterval );

			// track the lifetime of this new path
			float lifetime = GetLifetime();
			if ( lifetime > 0.0f )
			{
				m_lifetimeTimer.Start( lifetime );
			}
			else
			{
				m_lifetimeTimer.Invalidate();
			}
		}
		else
		{
			// can't reach subject - throttle retry based on range to subject
			m_failTimer.Start( 0.005f * ( bot->GetRangeTo( (CBaseEntity*)subject ) ) );
			
			// allow bot to react to path failure
			bot->OnMoveToFailure( this, FAIL_NO_PATH_EXISTS );
			if(ukr_next_bot_debug.GetBool())
			{
				DevMsg("%3.2f: bot(#%d) REPATH FAILED\n", g_pGlobals->curtime, reinterpret_cast<IBaseEntity*>(bot->GetEntity())->entindex());
			}
			Invalidate();
		}
	}
}

/*Прямий шлях переслідування*/
class DirectChasePath : public IChasePath
{
public:

	DirectChasePath( IChasePath::SubjectChaseType chaseHow = IChasePath::DONT_LEAD_SUBJECT ) : IChasePath( chaseHow )
	{

	}

	//-------------------------------------------------------------------------------------------------------
	virtual void Update( INextBot *me, IBaseEntity *victim, const IPathCost &pathCost, Vector *pPredictedSubjectPos = NULL )	// update path to chase target and move bot along path
	{
		Assert( !pPredictedSubjectPos );
		bool bComputedPredictedPosition;
		Vector vecPredictedPosition;
		if ( !DirectChase( &bComputedPredictedPosition, &vecPredictedPosition, me, victim ) )
		{
			// path around obstacles to reach our victim
			IChasePath::Update( me, victim, pathCost, bComputedPredictedPosition ? &vecPredictedPosition : NULL );
		}
		NotifyVictim( me, victim );
	}

	//-------------------------------------------------------------------------------------------------------
	bool DirectChase( bool *pPredictedPositionComputed, Vector *pPredictedPos, INextBot *me, IBaseEntity *victim )		// if there is nothing between us and our victim, run directly at them
	{
		*pPredictedPositionComputed = false;

		ILocomotion *mover = me->GetLocomotionInterface();

		if ( me->IsImmobile() || mover->IsScrambling() )
		{
			return false;
		}

		if ( IsDiscontinuityAhead( me, CLIMB_UP ) )
		{
			return false;
		}

		if ( IsDiscontinuityAhead( me, JUMP_OVER_GAP ) )
		{
			return false;
		}

		Vector leadVictimPos = PredictSubjectPosition( me, victim );

		// Don't want to have to compute the predicted position twice.
		*pPredictedPositionComputed = true;
		*pPredictedPos = leadVictimPos;

		if ( !mover->IsPotentiallyTraversable( mover->GetFeet(), leadVictimPos  ) )
		{
			return false;
		}

		// the way is clear - move directly towards our victim
		mover->FaceTowards( leadVictimPos );
		mover->Approach( leadVictimPos );

		me->GetBodyInterface()->AimHeadTowards( (CBaseEntity*)victim );

		// old path is no longer useful since we've moved off of it
		Invalidate();

		return true;
	}

	//-------------------------------------------------------------------------------------------------------
	virtual bool IsRepathNeeded( INextBot *bot, IBaseEntity *subject ) const			// return true if situation has changed enough to warrant recomputing the current path
	{
		if ( IChasePath::IsRepathNeeded( bot, subject ) )
		{
			return true;
		}

		return bot->GetLocomotionInterface()->IsStuck() && bot->GetLocomotionInterface()->GetStuckDuration() > 2.0f;
	}

	//-------------------------------------------------------------------------------------------------------
	/**
	 * Determine exactly where the path goes between the given two areas
	 * on the path. Return this point in 'crossPos'.
	 */
	virtual void ComputeAreaCrossing( INextBot *bot, const INavArea *from, const Vector &fromPos, const INavArea *to, Nav_DirType dir, Vector *crossPos ) const
	{
		Vector center;
		float halfWidth;
		from->ComputePortal( to, dir, &center, &halfWidth );

		*crossPos = center;
	}

	void NotifyVictim( INextBot *me, IBaseEntity *victim );
};


#endif