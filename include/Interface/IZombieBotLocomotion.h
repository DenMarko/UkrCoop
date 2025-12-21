#ifndef _HEADER_ZOMBIE_BOT_LOCOMOTION_INCLUDE_
#define _HEADER_ZOMBIE_BOT_LOCOMOTION_INCLUDE_
#include "ILocomotion.h"

class IZombieBotLocomotion : public ILocomotion
{
public:
    virtual ~IZombieBotLocomotion() {}

  	virtual void OnLeaveGround( CBaseEntity *ground ) = 0;
	virtual void OnLandOnGround( CBaseEntity *ground ) = 0;
	virtual void OnContact( CBaseEntity *other, CGameTrace *result = nullptr ) = 0;								// викликається, коли бот торкається "іншого"
	
	virtual void OnMoveToSuccess( const Path *path ) = 0;														// викликається, коли бот досягає кінця заданого шляху
	virtual void OnMoveToFailure( const Path *path, MoveToFailureType reason ) = 0;								// викликається, коли бот не досягає кінця заданого шляху
	virtual void OnAnimationActivityComplete( int activity ) = 0;												// коли анімаційна діяльність закінчилася
	virtual void OnNavAreaChanged( CNavArea *newArea, CNavArea *oldArea ) = 0;									// коли бот входить у нову область навігації
	virtual void Reset( void ) = 0;
	virtual void Update( void ) = 0;
	virtual void Approach( const Vector &goalPos, float goalWeight = 1.0f ) = 0;
	virtual void DriveTo( const Vector &pos ) = 0;
	virtual bool ClimbUpToLedge( const Vector &landingGoal, const Vector &landingForward, const CBaseEntity *obstacle ) = 0;
	virtual void JumpAcrossGap( const Vector &landingGoal, const Vector &landingForward ) = 0;
	virtual void Jump( void ) = 0;
	virtual bool IsClimbingOrJumping( void ) const = 0;
	virtual bool IsClimbingUpToLedge( void ) const = 0;
	virtual bool IsJumpingAcrossGap( void ) const = 0;
	virtual void Run( void ) = 0;
	virtual void Walk( void ) = 0;
	virtual void Stop( void ) = 0;
	virtual bool IsRunning( void ) const = 0;
	virtual void SetDesiredSpeed( float speed ) = 0;
	virtual float GetDesiredSpeed( void ) const = 0;
	virtual float GetSpeedLimit( void ) const = 0;
	virtual bool IsOnGround( void ) const = 0;
	virtual CBaseEntity *GetGround( void ) const = 0;
	virtual const Vector &GetGroundNormal( void ) const = 0;
	virtual void ClimbLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) = 0;
	virtual void DescendLadder( const CNavLadder *ladder, const CNavArea *dismountGoal ) = 0;
	virtual bool IsUsingLadder( void ) const = 0;
	virtual bool IsAscendingOrDescendingLadder( void ) const = 0;
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
	virtual bool IsAreaTraversable( const CNavArea *baseArea ) const = 0;
	virtual bool IsEntityTraversable( CBaseEntity *obstacle, TraverseWhenType when = EVENTUALLY ) const = 0;

	virtual const Vector &GetAcceleration(void) const = 0;
	virtual void SetAcceleration(const Vector &) = 0;

};


#endif // _HEADER_ZOMBIE_BOT_LOCOMOTION_INCLUDE_