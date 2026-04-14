#ifndef _HEADER_ACTION_HANDLE_INCLUDE_
#define _HEADER_ACTION_HANDLE_INCLUDE_

#include "extension.h"
#include "Interface/INextBotBehavior.h"
#include "Interface/ISurvivorBot.h"
#include "amtl/am-hashmap.h"

typedef Behavior<IBaseCombatCharacter> INextBotBehaviors;
typedef Action<IBaseCombatCharacter> INextBotAction;

typedef INextBotBehaviors* INextBotBehaviors_ptr;
typedef INextBotAction* INextBotAction_ptr;

enum ACTION_RESULT
{
	ARES_IGNORED = 0,		// plugin didn't take any action
	ARES_HANDLED,			// plugin did something, but real function should still be called
	ARES_OVERRIDE,			// call real function, but use my return value
	ARES_SUPERCEDE			// skip real function; use my return value
};


struct CActionHandle_Vector
{
    CActionHandle_Vector(ACTION_RESULT type, Vector &result) : eType(type)
    {
		m_result = result;
	}

	CActionHandle_Vector(ACTION_RESULT type = ARES_IGNORED) : eType(type)
	{
		m_result.Init();
	}

	ACTION_RESULT eType;
	Vector m_result;
};

template<typename T>
struct CActionHandle_ptr
{
    CActionHandle_ptr(ACTION_RESULT type, T &result) : eType(type), m_result(result)
    { }

	CActionHandle_ptr(ACTION_RESULT type = ARES_IGNORED) : eType(type), m_result(nullptr)
	{ }

	ACTION_RESULT eType;
	T m_result;
};

template<typename T>
struct CActionHandle
{
    CActionHandle(ACTION_RESULT type, const T &result) : eType(type), m_result(result)
	{ }

	CActionHandle(ACTION_RESULT type = ARES_IGNORED) : eType(type)
	{
		m_result = T();
	}

	ACTION_RESULT eType;
	T m_result;
};



class IHandleAction
{
public:
	virtual CActionHandle<ActionResult<IBaseCombatCharacter>> OnStart(INextBotAction_ptr pThis, IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* priorAction)
	{
		return CActionHandle<ActionResult<IBaseCombatCharacter>>(ARES_IGNORED);
	}

	virtual CActionHandle<ActionResult<IBaseCombatCharacter>> Update(INextBotAction_ptr pThis, IBaseCombatCharacter* me, float interval)
	{
		return CActionHandle<ActionResult<IBaseCombatCharacter>>(ARES_IGNORED);
	}

	virtual ACTION_RESULT OnEnd(INextBotAction_ptr pThis, IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* nextAction)
	{
		return ARES_IGNORED;
	}

	virtual CActionHandle<ActionResult<IBaseCombatCharacter>> OnSuspend(INextBotAction_ptr pThis, IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* interruptingAction)
	{
		return CActionHandle<ActionResult<IBaseCombatCharacter>>(ARES_IGNORED);
	}

	virtual CActionHandle<ActionResult<IBaseCombatCharacter>> OnResume(INextBotAction_ptr pThis, IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* interruptingAction)
	{
		return CActionHandle<ActionResult<IBaseCombatCharacter>>(ARES_IGNORED);
	}

	virtual CActionHandle_ptr<Action< IBaseCombatCharacter >*> InitialContainedAction(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle_ptr<Action<IBaseCombatCharacter>*>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnLeaveGround(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* ground)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnLandOnGround(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* ground)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnContact(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* other, CGameTrace* result = NULL)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnMoveToSuccess(INextBotAction_ptr pThis, IBaseCombatCharacter* me, const Path* path)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnMoveToFailure(INextBotAction_ptr pThis, IBaseCombatCharacter* me, const Path* path, MoveToFailureType reason)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnStuck(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnUnStuck(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnPostureChanged(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnAnimationActivityComplete(INextBotAction_ptr pThis, IBaseCombatCharacter* me, int activity)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnAnimationActivityInterrupted(INextBotAction_ptr pThis, IBaseCombatCharacter* me, int activity)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnAnimationEvent(INextBotAction_ptr pThis, IBaseCombatCharacter* me, animevent_t* event)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnIgnite(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnInjured(INextBotAction_ptr pThis, IBaseCombatCharacter* me, const CTakeDamageInfo& info)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnKilled(INextBotAction_ptr pThis, IBaseCombatCharacter* me, const CTakeDamageInfo& info)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnOtherKilled(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseCombatCharacter* victim, const CTakeDamageInfo& info)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnSight(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* subject)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnLostSight(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* subject)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnThreatChanged(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* subject)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnSound(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* source, const Vector& pos, KeyValues* keys)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnSpokeConcept(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseCombatCharacter* who, AIConcept_t aiconcept, AI_Response* response)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnNavAreaChanged(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CNavArea* newArea, CNavArea* oldArea)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnModelChanged(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnPickUp(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* item, CBaseCombatCharacter* giver)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnDrop(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* item)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnShoved(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* pusher)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnBlinded(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* blinder)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnCommandAttack(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* victim)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnCommandApproach(INextBotAction_ptr pThis, IBaseCombatCharacter* me, const Vector& pos, float range)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnCommandApproach(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* goal)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnCommandRetreat(INextBotAction_ptr pThis, IBaseCombatCharacter* me, CBaseEntity* threat, float range)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnCommandPause(INextBotAction_ptr pThis, IBaseCombatCharacter* me, float duration)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}

	virtual CActionHandle<EventDesiredResult< IBaseCombatCharacter >>  OnCommandResume(INextBotAction_ptr pThis, IBaseCombatCharacter* me)
	{
		return CActionHandle<EventDesiredResult< IBaseCombatCharacter >>(ARES_IGNORED);
	}
	
public:
	virtual CActionHandle<QueryResultType> ShouldPickUp(INextBotAction_ptr pThis, const INextBot* me, CBaseEntity* item) const
	{
		return CActionHandle<QueryResultType>(ARES_IGNORED);
	}

	virtual CActionHandle<QueryResultType> ShouldHurry(INextBotAction_ptr pThis, const INextBot* me) const
	{
		return CActionHandle<QueryResultType>(ARES_IGNORED);
	}

	virtual CActionHandle<QueryResultType> IsHindrance(INextBotAction_ptr pThis, const INextBot* me, CBaseEntity* blocker) const
	{
		return CActionHandle<QueryResultType>(ARES_IGNORED);
	}

	virtual CActionHandle_Vector SelectTargetPoint(INextBotAction_ptr pThis, const INextBot* me, const CBaseCombatCharacter* subject) const
	{
		return CActionHandle_Vector(ARES_IGNORED);
	}

	virtual CActionHandle<QueryResultType> IsPositionAllowed(INextBotAction_ptr pThis, const INextBot* me, const Vector& pos) const
	{
		return CActionHandle<QueryResultType>(ARES_IGNORED);
	}

	virtual CActionHandle_ptr<PathFollower*> QueryCurrentPath(INextBotAction_ptr pThis, const INextBot* me) const
	{
		return CActionHandle_ptr<PathFollower*>(ARES_IGNORED);
	}

	virtual CActionHandle_ptr<CBaseCombatCharacter*> SelectMoreDangerousThreat(INextBotAction_ptr pThis, const INextBot* me, const CBaseCombatCharacter* subject, const CBaseCombatCharacter* threat1, const CBaseCombatCharacter* threat2) const
	{
		return CActionHandle_ptr<CBaseCombatCharacter*>(ARES_IGNORED);
	}
};

#endif