#ifndef _INCLUDE_ZOMBIE_BOT_BODY_
#define _INCLUDE_ZOMBIE_BOT_BODY_
#include "IBody.h"

class IZombieBotBody : public IBody
{
public:
    virtual ~IZombieBotBody() {}

	virtual void        OnLeaveGround( CBaseEntity *ground ) = 0;														// викликається, коли бот залишає землю з будь-якої причини
	virtual void        OnLandOnGround( CBaseEntity *ground ) = 0;														// викликається, коли бот приземляється на землю після того, як був у повітрі
	virtual void        OnAnimationEvent( animevent_t *event ) = 0;													// коли подія анімації QC-файлу запускається поточною послідовністю анімації
	virtual void        OnInjured( const CTakeDamageInfo &info ) = 0;													// коли бот чимось пошкоджений
	virtual void        OnModelChanged( void ) = 0;																	// коли модель сутності була змінена
	virtual void        Reset( void ) = 0;
	virtual void        Update( void ) = 0;
	virtual bool        SetPosition( const Vector &pos ) = 0;
	virtual const Vector &GetEyePosition( void ) const = 0;
	virtual const Vector &GetViewVector( void ) const = 0;
	virtual void        AimHeadTowards( const Vector &lookAtPos, LookAtPriorityType priority = BORING, float duration = 0.0f, INextBotReply *replyWhenAimed = nullptr, const char *reason = nullptr ) = 0;
	virtual void        AimHeadTowards( CBaseEntity *subject, LookAtPriorityType priority = BORING, float duration = 0.0f, INextBotReply *replyWhenAimed = nullptr, const char *reason = nullptr ) = 0;
	virtual bool        StartActivity( Activity act, unsigned int flags = 0 ) = 0;
	virtual int         SelectAnimationSequence( Activity act ) const = 0;
	virtual Activity    GetActivity( void ) const = 0;
	virtual bool        IsActivity( Activity act ) const = 0;
	virtual bool        HasActivityType( unsigned int flags ) const = 0;
	virtual void        SetDesiredPosture( PostureType posture ) = 0;
	virtual PostureType GetDesiredPosture( void ) const = 0;
	virtual bool        IsDesiredPosture( PostureType posture ) const = 0;
	virtual bool        IsInDesiredPosture( void ) const = 0;
	virtual PostureType GetActualPosture( void ) const = 0;
	virtual bool        IsActualPosture( PostureType posture ) const = 0;
	virtual bool        IsPostureMobile( void ) const = 0;
	virtual bool        IsPostureChanging( void ) const = 0;
	virtual void        SetArousal( ArousalType arousal ) = 0;
	virtual ArousalType GetArousal( void ) const = 0;
	virtual bool        IsArousal( ArousalType arousal ) const = 0;
	virtual float       GetHullWidth( void ) const = 0;
	virtual float       GetHullHeight( void ) const = 0;
	virtual float       GetStandHullHeight( void ) const = 0;
	virtual float       GetCrouchHullHeight( void ) const = 0;
	virtual const Vector &GetHullMins( void ) const = 0;
	virtual const Vector &GetHullMaxs( void ) const = 0;
	virtual unsigned int GetSolidMask( void ) const = 0;
    virtual CBaseEntity *GetEntity(void) = 0;
};

class IWitchBody : public IZombieBotBody
{
public:
    virtual ~IWitchBody() {}
    virtual void OnInjured( const CTakeDamageInfo &info ) = 0;													// коли бот чимось пошкоджений
};

class PlayerBody : public IBody
{
public:
	virtual ~PlayerBody() {}

	virtual void        Reset( void ) = 0;
	virtual void		Upkeep( void ) = 0;
	virtual bool        SetPosition( const Vector &pos ) = 0;
	virtual const Vector &GetEyePosition( void ) const = 0;
	virtual const Vector &GetViewVector( void ) const = 0;
	virtual void        AimHeadTowards( const Vector &lookAtPos, LookAtPriorityType priority = BORING, float duration = 0.0f, INextBotReply *replyWhenAimed = nullptr, const char *reason = nullptr ) = 0;
	virtual void        AimHeadTowards( CBaseEntity *subject, LookAtPriorityType priority = BORING, float duration = 0.0f, INextBotReply *replyWhenAimed = nullptr, const char *reason = nullptr ) = 0;
	virtual bool        IsHeadAimingOnTarget( void ) const = 0;
	virtual bool 		IsHeadSteady( void ) const = 0;
	virtual float 		GetHeadSteadyDuration( void ) const = 0;
	virtual float 		GetMaxHeadAngularVelocity( void ) const = 0;
	virtual bool        StartActivity( Activity act, unsigned int flags = 0 ) = 0;
	virtual Activity    GetActivity( void ) const = 0;
	virtual bool        IsActivity( Activity act ) const = 0;
	virtual bool        HasActivityType( unsigned int flags ) const = 0;
	virtual void        SetDesiredPosture( PostureType posture ) = 0;
	virtual PostureType GetDesiredPosture( void ) const = 0;
	virtual bool        IsDesiredPosture( PostureType posture ) const = 0;
	virtual bool        IsInDesiredPosture( void ) const = 0;
	virtual PostureType GetActualPosture( void ) const = 0;
	virtual bool        IsActualPosture( PostureType posture ) const = 0;
	virtual bool        IsPostureMobile( void ) const = 0;
	virtual bool        IsPostureChanging( void ) const = 0;
	virtual void        SetArousal( ArousalType arousal ) = 0;
	virtual ArousalType GetArousal( void ) const = 0;
	virtual bool        IsArousal( ArousalType arousal ) const = 0;
	virtual float       GetHullWidth( void ) const = 0;
	virtual float       GetHullHeight( void ) const = 0;
	virtual float       GetStandHullHeight( void ) const = 0;
	virtual float       GetCrouchHullHeight( void ) const = 0;
	virtual const Vector &GetHullMins( void ) const = 0;
	virtual const Vector &GetHullMaxs( void ) const = 0;
	virtual unsigned int GetSolidMask( void ) const = 0;
    virtual CBaseEntity *GetEntity(void) = 0;
};


#endif  //_INCLUDE_ZOMBIE_BOT_BODY_