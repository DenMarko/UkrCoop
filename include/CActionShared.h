#ifndef _HEADER_ACTION_SHARED_INCLUDE_
#define _HEADER_ACTION_SHARED_INCLUDE_

#include "CActionHandle.h"

class CActionShared : public Action<IBaseCombatCharacter>
{
public:
    CActionShared();
	virtual ~CActionShared() {}

private:
	virtual INextBotEventResponder* FirstContainedResponder(void) const;
	virtual INextBotEventResponder* NextContainedResponder(INextBotEventResponder* current) const;

protected:
	virtual const char* GetName(void) const;
	virtual bool IsNamed(const char* name) const;
	virtual const char* GetFullName(void) const;

public:
	virtual ActionResult< IBaseCombatCharacter >	OnStart(IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* priorAction);
	virtual ActionResult< IBaseCombatCharacter >	Update(IBaseCombatCharacter* me, float interval);
	virtual void									OnEnd(IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* nextAction);
	virtual ActionResult< IBaseCombatCharacter >	OnSuspend(IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* interruptingAction);
	virtual ActionResult< IBaseCombatCharacter >	OnResume(IBaseCombatCharacter* me, Action< IBaseCombatCharacter >* interruptingAction);
	virtual Action< IBaseCombatCharacter >* 		InitialContainedAction(IBaseCombatCharacter* me);
};

class ActionProcessed : public CActionShared
{
public:
	virtual EventDesiredResult< IBaseCombatCharacter > OnLeaveGround(IBaseCombatCharacter* me, CBaseEntity* ground);
	virtual EventDesiredResult< IBaseCombatCharacter > OnLandOnGround(IBaseCombatCharacter* me, CBaseEntity* ground);
	virtual EventDesiredResult< IBaseCombatCharacter > OnContact(IBaseCombatCharacter* me, CBaseEntity* other, CGameTrace* result = NULL);
	virtual EventDesiredResult< IBaseCombatCharacter > OnMoveToSuccess(IBaseCombatCharacter* me, const Path* path);
	virtual EventDesiredResult< IBaseCombatCharacter > OnMoveToFailure(IBaseCombatCharacter* me, const Path* path, MoveToFailureType reason);
	virtual EventDesiredResult< IBaseCombatCharacter > OnStuck(IBaseCombatCharacter* me);
	virtual EventDesiredResult< IBaseCombatCharacter > OnUnStuck(IBaseCombatCharacter* me);
	virtual EventDesiredResult< IBaseCombatCharacter > OnPostureChanged(IBaseCombatCharacter* me);
	virtual EventDesiredResult< IBaseCombatCharacter > OnAnimationActivityComplete(IBaseCombatCharacter* me, int activity);
	virtual EventDesiredResult< IBaseCombatCharacter > OnAnimationActivityInterrupted(IBaseCombatCharacter* me, int activity);
	virtual EventDesiredResult< IBaseCombatCharacter > OnAnimationEvent(IBaseCombatCharacter* me, animevent_t* event);
	virtual EventDesiredResult< IBaseCombatCharacter > OnIgnite(IBaseCombatCharacter* me);
	virtual EventDesiredResult< IBaseCombatCharacter > OnInjured(IBaseCombatCharacter* me, const CTakeDamageInfo& info);
	virtual EventDesiredResult< IBaseCombatCharacter > OnKilled(IBaseCombatCharacter* me, const CTakeDamageInfo& info);
	virtual EventDesiredResult< IBaseCombatCharacter > OnOtherKilled(IBaseCombatCharacter* me, CBaseCombatCharacter* victim, const CTakeDamageInfo& info);
	virtual EventDesiredResult< IBaseCombatCharacter > OnSight(IBaseCombatCharacter* me, CBaseEntity* subject);
	virtual EventDesiredResult< IBaseCombatCharacter > OnLostSight(IBaseCombatCharacter* me, CBaseEntity* subject);
	virtual EventDesiredResult< IBaseCombatCharacter > OnThreatChanged(IBaseCombatCharacter* me, CBaseEntity* subject);
	virtual EventDesiredResult< IBaseCombatCharacter > OnSound(IBaseCombatCharacter* me, CBaseEntity* source, const Vector& pos, KeyValues* keys);
	virtual EventDesiredResult< IBaseCombatCharacter > OnSpokeConcept(IBaseCombatCharacter* me, CBaseCombatCharacter* who, AIConcept_t aiconcept, AI_Response* response);
	virtual EventDesiredResult< IBaseCombatCharacter > OnNavAreaChanged(IBaseCombatCharacter* me, CNavArea* newArea, CNavArea* oldArea);
	virtual EventDesiredResult< IBaseCombatCharacter > OnModelChanged(IBaseCombatCharacter* me);
	virtual EventDesiredResult< IBaseCombatCharacter > OnPickUp(IBaseCombatCharacter* me, CBaseEntity* item, CBaseCombatCharacter* giver);
	virtual EventDesiredResult< IBaseCombatCharacter > OnDrop(IBaseCombatCharacter* me, CBaseEntity* item);
	virtual EventDesiredResult< IBaseCombatCharacter > OnShoved(IBaseCombatCharacter* me, CBaseEntity* pusher);
	virtual EventDesiredResult< IBaseCombatCharacter > OnBlinded(IBaseCombatCharacter* me, CBaseEntity* blinder);
	virtual EventDesiredResult< IBaseCombatCharacter > OnCommandAttack(IBaseCombatCharacter* me, CBaseEntity* victim);
	virtual EventDesiredResult< IBaseCombatCharacter > OnCommandApproach(IBaseCombatCharacter* me, const Vector& pos, float range);
	virtual EventDesiredResult< IBaseCombatCharacter > OnCommandApproach(IBaseCombatCharacter* me, CBaseEntity* goal);
	virtual EventDesiredResult< IBaseCombatCharacter > OnCommandRetreat(IBaseCombatCharacter* me, CBaseEntity* threat, float range);
	virtual EventDesiredResult< IBaseCombatCharacter > OnCommandPause(IBaseCombatCharacter* me, float duration);
	virtual EventDesiredResult< IBaseCombatCharacter > OnCommandResume(IBaseCombatCharacter* me);
	virtual bool IsAbleToBlockMovementOf(const INextBot* botInMotion) const;

public:
	virtual QueryResultType			ShouldPickUp(const INextBot* me, CBaseEntity* item) const;
	virtual QueryResultType			ShouldHurry(const INextBot* me) const;
	virtual QueryResultType			IsHindrance(const INextBot* me, CBaseEntity* blocker) const;
	virtual Vector					SelectTargetPoint(const INextBot* me, const CBaseCombatCharacter* subject) const;
	virtual QueryResultType			IsPositionAllowed(const INextBot* me, const Vector& pos) const;
	virtual PathFollower*           QueryCurrentPath(const INextBot* me) const;
	virtual const CBaseCombatCharacter*     SelectMoreDangerousThreat(const INextBot* me, const CBaseCombatCharacter* subject, const CBaseCombatCharacter* threat1, const CBaseCombatCharacter* threat2) const;
};

extern CActionShared* g_pActionProcess;

#endif