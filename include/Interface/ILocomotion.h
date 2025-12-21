#ifndef _INCLUDE_LOCOMOTION_H_
#define _INCLUDE_LOCOMOTION_H_
#include "INextBotComponent.h"
#include "IBaseEntity.h"

class CNavLadder;
class CNavArea;
class QAngle;

class ILocomotion : public INextBotComponent
{
public:
    virtual ~ILocomotion() {}

	virtual void OnLeaveGround( CBaseEntity *ground ) { }
	virtual void OnLandOnGround( CBaseEntity *ground ) { }
	virtual void Reset( void );
	virtual void Update( void );
	virtual void Approach( const Vector &goalPos, float goalWeight = 1.0f );
	virtual void DriveTo( const Vector &pos );
	virtual bool ClimbUpToLedge( const Vector &landingGoal, const Vector &landingForward, const CBaseEntity *obstacle ) { return true; }
	virtual void JumpAcrossGap( const Vector &landingGoal, const Vector &landingForward ) { }
	virtual void Jump( void ) {}
	virtual bool IsClimbingOrJumping( void ) const { return false; }
	virtual bool IsClimbingUpToLedge( void ) const { return false; }
	virtual bool IsJumpingAcrossGap( void ) const { return false; }
	virtual bool IsScrambling( void ) const { return !IsOnGround() || IsClimbingOrJumping() || IsAscendingOrDescendingLadder(); }
	virtual void Run( void ) { }
	virtual void Walk( void ) { }
	virtual void Stop( void ) { }
	virtual bool IsRunning( void ) const { return false; }
	virtual void SetDesiredSpeed( float speed ) { }
	virtual float GetDesiredSpeed( void ) const { return 0.f; }
	virtual void SetSpeedLimit( float speed ) { }
	virtual float GetSpeedLimit( void ) const { return 1000.f; }
	virtual bool IsOnGround( void ) const { return false; }
	virtual CBaseEntity *GetGround( void ) const { return nullptr; }
	virtual const Vector &GetGroundNormal( void ) const { return vec3_origin; }
	virtual float GetGroundSpeed( void ) const { return m_groundSpeed; }
	virtual const Vector &GetGroundMotionVector( void ) const { return m_groundMotionVector; }
	virtual void ClimbLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) { }
	virtual void DescendLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) { }
	virtual bool IsUsingLadder( void ) const { return false; }
	virtual bool IsAscendingOrDescendingLadder( void ) const { return false; }
	virtual bool IsAbleToAutoCenterOnLadder( void ) const { return false; }

	virtual void FaceTowards( const Vector &target ) { }
	virtual void SetDesiredLean( const QAngle &lean ) { }
	virtual const QAngle &GetDesiredLean( void ) const { return vec3_angle; }
	virtual const Vector &GetFeet( void ) const;
	virtual float GetStepHeight( void ) const { return 0.f; }
	virtual float GetMaxJumpHeight( void ) const { return 0.f; }
	virtual float GetDeathDropHeight( void ) const { return 0.f; }
	virtual float GetRunSpeed( void ) const { return 0.f; }
	virtual float GetWalkSpeed( void ) const { return 0.f; }
	virtual float GetMaxAcceleration( void ) const { return 0.f; }
	virtual float GetMaxDeceleration( void ) const { return 0.f; }
	virtual const Vector &GetVelocity( void ) const { return vec3_origin; }
	virtual float GetSpeed( void ) const { return m_speed; }
	virtual const Vector &GetMotionVector( void ) const { return m_motionVector; }

	virtual bool IsAreaTraversable( const CNavArea *baseArea ) const;

	virtual float GetTraversableSlopeLimit( void ) const { return 0.6f; }
	enum TraverseWhenType 
	{ 
		IMMEDIATELY,
		EVENTUALLY
	};
	virtual bool IsPotentiallyTraversable( const Vector &from, const Vector &to, TraverseWhenType when = EVENTUALLY, float *fraction = nullptr ) const;
	virtual bool HasPotentialGap( const Vector &from, const Vector &to, float *fraction = nullptr ) const;
	virtual bool IsGap( const Vector &pos, const Vector &forward ) const;
	virtual bool IsEntityTraversable( CBaseEntity *obstacle, TraverseWhenType when = EVENTUALLY ) const;
	virtual bool IsStuck( void ) const { return m_isStuck; }
	virtual float GetStuckDuration( void ) const { return ( IsStuck() ) ? m_stuckTimer.GetElapsedTime() : 0.0f; }
	virtual void ClearStuckStatus( const char *reason = "" );

	virtual bool IsAttemptingToMove( void ) const;
protected:
	virtual void AdjustPosture( const Vector &moveGoal );

private:
	Vector m_motionVector;
	Vector m_groundMotionVector;
	float m_speed;
	float m_groundSpeed;

	// stuck monitoring
	bool m_isStuck;									// if true, we are stuck
	IntervalTimers m_stuckTimer;						// how long we've been stuck
	CountdownTimers m_stillStuckTimer;				// for resending stuck events
	Vector m_stuckPos;								// where we got stuck
	IntervalTimers m_moveRequestTimer;
};

class PlayerLocomotion : public ILocomotion
{
public:
	~PlayerLocomotion() {}

	virtual void Reset( void ) = 0;
	virtual void Update( void ) = 0;
	virtual void Approach( const Vector &goalPos, float goalWeight = 1.0f ) = 0;
	virtual void DriveTo( const Vector &pos ) = 0;
	virtual bool ClimbUpToLedge( const Vector &landingGoal, const Vector &landingForward, const CBaseEntity *obstacle ) = 0;
	virtual void JumpAcrossGap( const Vector &landingGoal, const Vector &landingForward ) = 0;
	virtual void Jump( void )  = 0;
	virtual bool IsClimbingOrJumping( void ) const = 0;
	virtual bool IsClimbingUpToLedge( void ) const = 0;
	virtual bool IsJumpingAcrossGap( void ) const = 0;
	virtual void Run( void ) = 0;
	virtual void Walk( void ) = 0;
	virtual void Stop( void ) = 0;
	virtual bool IsRunning( void ) const = 0;
	virtual void SetDesiredSpeed( float speed ) = 0;
	virtual float GetDesiredSpeed( void ) const = 0;
	virtual bool IsOnGround( void ) const = 0;
	virtual CBaseEntity *GetGround( void ) const = 0;
	virtual const Vector &GetGroundNormal( void ) const = 0;
	virtual void ClimbLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) = 0;
	virtual void DescendLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) = 0;
	virtual bool IsUsingLadder( void ) const = 0;
	virtual bool IsAscendingOrDescendingLadder( void ) const = 0;
	virtual bool IsAbleToAutoCenterOnLadder( void ) const = 0;

	virtual void FaceTowards( const Vector &target ) = 0;
	virtual void SetDesiredLean( const QAngle &lean ) = 0;
	virtual const QAngle &GetDesiredLean( void ) const = 0;
	virtual const Vector &GetFeet( void ) const = 0;
	virtual float GetStepHeight( void ) const = 0;
	virtual float GetMaxJumpHeight( void ) const = 0;
	virtual float GetDeathDropHeight( void ) const = 0;
	virtual float GetRunSpeed( void ) const = 0;
	virtual float GetWalkSpeed( void ) const = 0;
	virtual float GetMaxAcceleration( void ) const = 0;
	virtual float GetMaxDeceleration( void ) const = 0;
	virtual const Vector &GetVelocity( void ) const = 0;
	virtual void AdjustPosture( const Vector &moveGoal ) = 0;
	virtual void SetMinimumSpeedLimit( float speed ) = 0;
	virtual void SetMaximumSpeedLimit( float speed ) = 0;

};

#endif