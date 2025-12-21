#ifndef _INCLUDE_BODY_H_
#define _INCLUDE_BODY_H_
#include "INextBotComponent.h"
#include "IBaseEntity.h"

class IBody : public INextBotComponent
{
public:
    virtual ~IBody() {}

	virtual void        Reset( void ) { INextBotComponent::Reset(); }
	virtual void        Update( void ) { }
	virtual bool        SetPosition( const Vector &pos );

	virtual const Vector &GetEyePosition( void ) const;
	virtual const Vector &GetViewVector( void ) const;

	enum LookAtPriorityType
	{
		BORING,				// нудно
		INTERESTING,		// цікаво
		IMPORTANT,			// важливо
		CRITICAL,			// критично
		MANDATORY			// обов'язково
	};

	virtual void AimHeadTowards( const Vector &lookAtPos, LookAtPriorityType priority = BORING, float duration = 0.0f, INextBotReply *replyWhenAimed = nullptr, const char *reason = nullptr );
	virtual void AimHeadTowards( CBaseEntity *subject, LookAtPriorityType priority = BORING, float duration = 0.0f, INextBotReply *replyWhenAimed = nullptr, const char *reason = nullptr );
	virtual bool IsHeadAimingOnTarget( void ) const;
	virtual bool IsHeadSteady( void ) const { return false; }
	virtual float GetHeadSteadyDuration( void ) const { return 0.f; }
	virtual float GetMaxHeadAngularVelocity( void ) const { return 100.f; }
	virtual bool StartActivity( Activity act, unsigned int flags = 0 ) {return false; }
	virtual int SelectAnimationSequence( Activity act ) const { return 0; }
	virtual Activity GetActivity( void ) const { return ACT_INVALID; }
	virtual bool IsActivity( Activity act ) const { return false; }
	virtual bool HasActivityType( unsigned int flags ) const { return false; }
	
    enum PostureType
	{
		STAND,			// стоячий
		CROUCH,			// кручка
		SIT,			// сидячий
		CRAWL,			// повзати
		LIE				// брегня
	};

	virtual void SetDesiredPosture( PostureType posture ) {}
	virtual PostureType GetDesiredPosture( void ) const { return IBody::STAND; }
	virtual bool IsDesiredPosture( PostureType posture ) const { return false; }
	virtual bool IsInDesiredPosture( void ) const { return false; }
	virtual PostureType GetActualPosture( void ) const { return IBody::STAND; }
	virtual bool IsActualPosture( PostureType posture ) const { return false; }
	virtual bool IsPostureMobile( void ) const { return false; }
	virtual bool IsPostureChanging( void ) const { return false; }
	
    enum ArousalType
	{
		NEUTRAL,		// нейтральний
		ALERT,			// попердження
		INTENSE			// інтенсивний
	};

	virtual void SetArousal( ArousalType arousal ) {}
	virtual ArousalType GetArousal( void ) const { return IBody::NEUTRAL; }
	virtual bool IsArousal( ArousalType arousal ) const { return false; }
	virtual float GetHullWidth( void ) const { return 0.f; }
	virtual float GetHullHeight( void ) const;
	virtual float GetStandHullHeight( void ) const { return 0.f; }
	virtual float GetCrouchHullHeight( void ) const { return 0.f; }
	virtual const Vector &GetHullMins( void ) const;
	virtual const Vector &GetHullMaxs( void ) const;
	virtual unsigned int GetSolidMask( void ) const { return 0; }
    virtual CBaseEntity *GetEntity(void) { return nullptr; }
};

inline float IBody::GetHullHeight( void ) const
{
	return 0.f;
}

inline const Vector& IBody::GetHullMins( void ) const 
{
	return vec3_origin;
}

inline const Vector &IBody::GetHullMaxs( void ) const
{
	return vec3_origin;
}

#endif