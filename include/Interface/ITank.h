#ifndef _INCLUDE_TANK_H_
#define _INCLUDE_TANK_H_
#include "IBossZombiePlayerBot.h"

class TankLocomotion : public PlayerLocomotion
{
public:
    ~TankLocomotion() {}

	virtual void OnAnimationActivityComplete( int activity ) = 0;
    virtual void OnAnimationActivityInterrupted( int activity ) = 0;
	virtual void Reset( void ) = 0;
	virtual void Update( void ) = 0;
	virtual bool ClimbUpToLedge( const Vector &landingGoal, const Vector &landingForward, const CBaseEntity *obstacle ) = 0;
    virtual bool IsOnGround( void ) const = 0;
    virtual float GetMaxJumpHeight( void ) const = 0;
    virtual bool IsAreaTraversable( const CNavArea *baseArea ) const = 0;
    virtual bool IsEntityTraversable( CBaseEntity *obstacle, TraverseWhenType when = EVENTUALLY ) const = 0;
};

class ITank : public IBossZombiePlayerBot
{
public:
	void EnterStasis( void ); 
	void LeaveStasis( void );
public:
    ~ITank() {}

	virtual ServerClass             *GetServerClass(void) = 0;
	virtual int					    YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual datamap_t*			    GetDataDescMap(void) = 0;
	virtual void				    Spawn( void ) = 0;
	virtual void				    Precache( void ) = 0;
	virtual void				    Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual int					    OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
    virtual int                     CanBeA(ZombieClassType eClass) const = 0;
    virtual CBaseCombatCharacter*   GetEntity(void) const = 0;
    virtual TankLocomotion      	*GetLocomotionInterface(void) const = 0;
	virtual PlayerBody  			*GetBodyInterface(void) const = 0;
	virtual IIntention  			*GetIntentionInterface(void) const = 0;
	virtual IZombieBotVision		*GetVisionInterface(void) const = 0;
    virtual bool                    IsAbleToBreak(const CBaseEntity *) const = 0;

};

#endif