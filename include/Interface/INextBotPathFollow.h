#ifndef _HEADER_NEXT_BOT_PATH_FOLLOW_INCLUDE_
#define _HEADER_NEXT_BOT_PATH_FOLLOW_INCLUDE_

#include "INextBotPath.h"

class INextBot;
class ILocomotion;


class IPathFollower : public Path
{
public:
	IPathFollower( void );
	virtual ~IPathFollower();

	virtual void Invalidate( void );
	virtual void Draw( const Path::Segment *start = NULL ) const;
	virtual void OnPathChanged( INextBot *bot, Path::ResultType result );

	virtual void Update( INextBot *bot );

	virtual const Path::Segment *GetCurrentGoal( void ) const;

	virtual void SetMinLookAheadDistance( float value );
	
	virtual CBaseEntity *GetHindrance( void ) const;

	virtual bool IsDiscontinuityAhead( INextBot *bot, Path::SegmentType type, float range = -1.0f ) const;

	void SetGoalTolerance( float range );

private:
	const Path::Segment *m_goal;
	float m_minLookAheadRange;

	bool CheckProgress( INextBot *bot );
	bool IsAtGoal( INextBot *bot ) const;

	bool m_isOnStairs;

	CountdownTimers m_avoidTimer;	

	CountdownTimers m_waitTimer;
	CHandle< CBaseEntity > m_hindrance;
	
	bool m_didAvoidCheck;
	Vector m_leftFrom;
	Vector m_leftTo;
	bool m_isLeftClear;
	Vector m_rightFrom;
	Vector m_rightTo;
	bool m_isRightClear;
	Vector m_hullMin, m_hullMax;

	void AdjustSpeed( INextBot *bot );

	Vector Avoid( INextBot *bot, const Vector &goalPos, const Vector &forward, const Vector &left );
	bool Climbing( INextBot *bot, const Path::Segment *goal, const Vector &forward, const Vector &left, float goalRange );
	bool JumpOverGaps( INextBot *bot, const Path::Segment *goal, const Vector &forward, const Vector &left, float goalRange );

	bool LadderUpdate( INextBot *bot );
	CBaseEntity *FindBlocker( INextBot *bot );

	float m_goalTolerance;

	ConVarRef NextBotAllowAvoiding;
	ConVarRef NextBotAllowClimbing;
	ConVarRef NextBotAllowGapJumping;
	ConVarRef NextBotLadderAlignRange;
};

inline void IPathFollower::SetGoalTolerance(float range)
{
    m_goalTolerance = range;
}

inline const Path::Segment *IPathFollower::GetCurrentGoal(void) const
{
    return m_goal;
}

inline void IPathFollower::SetMinLookAheadDistance(float value)
{
    m_minLookAheadRange = value;
}

inline CBaseEntity *IPathFollower::GetHindrance(void) const
{
    return m_hindrance;
}

#endif