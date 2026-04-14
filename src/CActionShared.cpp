#include "CActionShared.h"
#include "CActionHook.h"

ActionProcessed g_ActionProcess;
CActionShared* g_pActionProcess = nullptr;

CActionShared::CActionShared()
{
    g_pActionProcess = this;
}

INextBotEventResponder *CActionShared::FirstContainedResponder(void) const
{
    Autoswap guard(this);
    return reinterpret_cast<INextBotAction_ptr>(const_cast<CActionShared *>(this))->FirstContainedResponder();
}

INextBotEventResponder *CActionShared::NextContainedResponder(INextBotEventResponder *current) const
{
    Autoswap guard(this);
    return reinterpret_cast<INextBotAction_ptr>(const_cast<CActionShared *>(this))->NextContainedResponder(current);
}

const char *CActionShared::GetName(void) const
{
    Autoswap guard(this);
    return reinterpret_cast<INextBotAction_ptr>(const_cast<CActionShared *>(this))->GetName();
}

bool CActionShared::IsNamed(const char *name) const
{
    Autoswap guard(this);
    return reinterpret_cast<INextBotAction_ptr>(const_cast<CActionShared *>(this))->IsNamed(name);
}

const char *CActionShared::GetFullName(void) const
{
    Autoswap guard(this);
    return reinterpret_cast<INextBotAction_ptr>(const_cast<CActionShared *>(this))->GetFullName();
}

ActionResult<IBaseCombatCharacter> CActionShared::OnStart(IBaseCombatCharacter *me, Action<IBaseCombatCharacter> *priorAction)
{
    Autoswap guard(this);
    ActionResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnStart(this, me, priorAction);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnStart(me, priorAction);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnStart(this, me, priorAction);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

ActionResult<IBaseCombatCharacter> CActionShared::Update(IBaseCombatCharacter *me, float interval)
{
    Autoswap guard(this);
    ActionResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->Update(this, me, interval);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->Update(me, interval);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->Update(this, me, interval);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

void CActionShared::OnEnd(IBaseCombatCharacter *me, Action<IBaseCombatCharacter> *nextAction)
{
    Autoswap guard(this);
    ACTION_RESULT type = ARES_IGNORED;
    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnEnd(this, me, nextAction);
                if(r >= ARES_OVERRIDE)
                {
                    type = r;
                }
            }
        }
    }

    if(type != ARES_SUPERCEDE)
        reinterpret_cast<INextBotAction_ptr>(this)->OnEnd(me, nextAction);

    type = ARES_IGNORED;
    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                mList->Element(i)->OnEnd(this, me, nextAction);
            }
        }
    }
}

ActionResult<IBaseCombatCharacter> CActionShared::OnSuspend(IBaseCombatCharacter *me, Action<IBaseCombatCharacter> *interruptingAction)
{
    Autoswap guard(this);
    ActionResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSuspend(this, me, interruptingAction);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnSuspend(me, interruptingAction);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSuspend(this, me, interruptingAction);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

ActionResult<IBaseCombatCharacter> CActionShared::OnResume(IBaseCombatCharacter *me, Action<IBaseCombatCharacter> *interruptingAction)
{
    Autoswap guard(this);
    ActionResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnResume(this, me, interruptingAction);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnResume(me, interruptingAction);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnResume(this, me, interruptingAction);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

Action<IBaseCombatCharacter> *CActionShared::InitialContainedAction(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    Action<IBaseCombatCharacter> *OriginalResult = nullptr, *postResult = nullptr, *preResult = nullptr;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->InitialContainedAction(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->InitialContainedAction(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->InitialContainedAction(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnLeaveGround(IBaseCombatCharacter *me, CBaseEntity *ground)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnLeaveGround(this, me, ground);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnLeaveGround(me, ground);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnLeaveGround(this, me, ground);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnLandOnGround(IBaseCombatCharacter *me, CBaseEntity *ground)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnLandOnGround(this, me, ground);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnLandOnGround(me, ground);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnLandOnGround(this, me, ground);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnContact(IBaseCombatCharacter *me, CBaseEntity *other, CGameTrace *result)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnContact(this, me, other, result);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnContact(me, other, result);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnContact(this, me, other, result);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnMoveToSuccess(IBaseCombatCharacter *me, const Path *path)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnMoveToSuccess(this, me, path);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnMoveToSuccess(me, path);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnMoveToSuccess(this, me, path);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnMoveToFailure(IBaseCombatCharacter *me, const Path *path, MoveToFailureType reason)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnMoveToFailure(this, me, path, reason);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnMoveToFailure(me, path, reason);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnMoveToFailure(this, me, path, reason);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnStuck(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnStuck(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnStuck(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnStuck(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnUnStuck(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnUnStuck(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnUnStuck(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnUnStuck(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnPostureChanged(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnPostureChanged(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnPostureChanged(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnPostureChanged(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnAnimationActivityComplete(IBaseCombatCharacter *me, int activity)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnAnimationActivityComplete(this, me, activity);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnAnimationActivityComplete(me, activity);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnAnimationActivityComplete(this, me, activity);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnAnimationActivityInterrupted(IBaseCombatCharacter *me, int activity)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnAnimationActivityInterrupted(this, me, activity);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnAnimationActivityInterrupted(me, activity);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnAnimationActivityInterrupted(this, me, activity);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnAnimationEvent(IBaseCombatCharacter *me, animevent_t *event)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnAnimationEvent(this, me, event);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnAnimationEvent(me, event);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnAnimationEvent(this, me, event);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnIgnite(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnIgnite(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnIgnite(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnIgnite(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnInjured(IBaseCombatCharacter *me, const CTakeDamageInfo &info)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnInjured(this, me, info);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnInjured(me, info);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnInjured(this, me, info);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnKilled(IBaseCombatCharacter *me, const CTakeDamageInfo &info)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnKilled(this, me, info);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnKilled(me, info);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnKilled(this, me, info);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnOtherKilled(IBaseCombatCharacter *me, CBaseCombatCharacter *victim, const CTakeDamageInfo &info)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnOtherKilled(this, me, victim, info);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnOtherKilled(me, victim, info);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnOtherKilled(this, me, victim, info);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnSight(IBaseCombatCharacter *me, CBaseEntity *subject)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSight(this, me, subject);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnSight(me, subject);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSight(this, me, subject);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnLostSight(IBaseCombatCharacter *me, CBaseEntity *subject)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnLostSight(this, me, subject);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnLostSight(me, subject);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnLostSight(this, me, subject);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnThreatChanged(IBaseCombatCharacter *me, CBaseEntity *subject)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnThreatChanged(this, me, subject);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnThreatChanged(me, subject);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnThreatChanged(this, me, subject);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnSound(IBaseCombatCharacter *me, CBaseEntity *source, const Vector &pos, KeyValues *keys)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSound(this, me, source, pos, keys);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnSound(me, source, pos, keys);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSound(this, me, source, pos, keys);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnSpokeConcept(IBaseCombatCharacter *me, CBaseCombatCharacter *who, AIConcept_t aiconcept, AI_Response *response)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSpokeConcept(this, me, who, aiconcept, response);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnSpokeConcept(me, who, aiconcept, response);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnSpokeConcept(this, me, who, aiconcept, response);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnNavAreaChanged(IBaseCombatCharacter *me, CNavArea *newArea, CNavArea *oldArea)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnNavAreaChanged(this, me, newArea, oldArea);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnNavAreaChanged(me, newArea, oldArea);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnNavAreaChanged(this, me, newArea, oldArea);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnModelChanged(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnModelChanged(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnModelChanged(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnModelChanged(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnPickUp(IBaseCombatCharacter *me, CBaseEntity *item, CBaseCombatCharacter *giver)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnPickUp(this, me, item, giver);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnPickUp(me, item, giver);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnPickUp(this, me, item, giver);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnDrop(IBaseCombatCharacter *me, CBaseEntity *item)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnDrop(this, me, item);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnDrop(me, item);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnDrop(this, me, item);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnShoved(IBaseCombatCharacter *me, CBaseEntity *pusher)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnShoved(this, me, pusher);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnShoved(me, pusher);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnShoved(this, me, pusher);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnBlinded(IBaseCombatCharacter *me, CBaseEntity *blinder)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnBlinded(this, me, blinder);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnBlinded(me, blinder);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnBlinded(this, me, blinder);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnCommandAttack(IBaseCombatCharacter *me, CBaseEntity *victim)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandAttack(this, me, victim);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnCommandAttack(me, victim);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandAttack(this, me, victim);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnCommandApproach(IBaseCombatCharacter *me, const Vector &pos, float range)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandApproach(this, me, pos, range);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnCommandApproach(me, pos, range);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandApproach(this, me, pos, range);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnCommandApproach(IBaseCombatCharacter *me, CBaseEntity *goal)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandApproach(this, me, goal);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnCommandApproach(me, goal);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandApproach(this, me, goal);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnCommandRetreat(IBaseCombatCharacter *me, CBaseEntity *threat, float range)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandRetreat(this, me, threat, range);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnCommandRetreat(me, threat, range);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandRetreat(this, me, threat, range);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnCommandPause(IBaseCombatCharacter *me, float duration)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandPause(this, me, duration);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnCommandPause(me, duration);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandPause(this, me, duration);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

EventDesiredResult<IBaseCombatCharacter> ActionProcessed::OnCommandResume(IBaseCombatCharacter *me)
{
    Autoswap guard(this);
    EventDesiredResult<IBaseCombatCharacter> OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(this, false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandResume(this, me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(this)->OnCommandResume(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(this, true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->OnCommandResume(this, me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

bool ActionProcessed::IsAbleToBlockMovementOf(const INextBot *botInMotion) const
{
    Autoswap guard(this);
    return reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->IsAbleToBlockMovementOf(botInMotion);
}

QueryResultType ActionProcessed::ShouldPickUp(const INextBot *me, CBaseEntity *item) const
{
    Autoswap guard(this);
    QueryResultType OriginalResult = ANSWER_UNDEFINED, postResult = ANSWER_UNDEFINED, preResult = ANSWER_UNDEFINED;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed *>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->ShouldPickUp(const_cast<ActionProcessed *>(this), me, item);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->ShouldPickUp(me, item);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed *>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->ShouldPickUp(const_cast<ActionProcessed*>(this), me, item);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

QueryResultType ActionProcessed::ShouldHurry(const INextBot *me) const
{
    Autoswap guard(this);
    QueryResultType OriginalResult = ANSWER_UNDEFINED, postResult = ANSWER_UNDEFINED, preResult = ANSWER_UNDEFINED;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->ShouldHurry(const_cast<ActionProcessed*>(this), me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->ShouldHurry(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->ShouldHurry(const_cast<ActionProcessed*>(this), me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

QueryResultType ActionProcessed::IsHindrance(const INextBot *me, CBaseEntity *blocker) const
{
    Autoswap guard(this);
    QueryResultType OriginalResult = ANSWER_UNDEFINED, postResult = ANSWER_UNDEFINED, preResult = ANSWER_UNDEFINED;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->IsHindrance(const_cast<ActionProcessed*>(this), me, blocker);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->IsHindrance(me, blocker);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->IsHindrance(const_cast<ActionProcessed*>(this), me, blocker);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

Vector ActionProcessed::SelectTargetPoint(const INextBot *me, const CBaseCombatCharacter *subject) const
{
    Autoswap guard(this);
    Vector OriginalResult, postResult, preResult;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->SelectTargetPoint(const_cast<ActionProcessed*>(this), me, subject);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->SelectTargetPoint(me, subject);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->SelectTargetPoint(const_cast<ActionProcessed*>(this), me, subject);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

QueryResultType ActionProcessed::IsPositionAllowed(const INextBot *me, const Vector &pos) const
{
    Autoswap guard(this);
    QueryResultType OriginalResult = ANSWER_UNDEFINED, postResult = ANSWER_UNDEFINED, preResult = ANSWER_UNDEFINED;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->IsPositionAllowed(const_cast<ActionProcessed*>(this), me, pos);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->IsPositionAllowed(me, pos);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->IsPositionAllowed(const_cast<ActionProcessed*>(this), me, pos);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

PathFollower *ActionProcessed::QueryCurrentPath(const INextBot *me) const
{
    Autoswap guard(this);
    PathFollower *OriginalResult = nullptr, *postResult = nullptr, *preResult = nullptr;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->QueryCurrentPath(const_cast<ActionProcessed*>(this), me);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = r.m_result;
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->QueryCurrentPath(me);
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->QueryCurrentPath(const_cast<ActionProcessed*>(this), me);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = r.m_result;
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}

const CBaseCombatCharacter *ActionProcessed::SelectMoreDangerousThreat(const INextBot *me, const CBaseCombatCharacter *subject, const CBaseCombatCharacter *threat1, const CBaseCombatCharacter *threat2) const
{
    Autoswap guard(this);
    CBaseCombatCharacter *OriginalResult = nullptr, *postResult = nullptr, *preResult = nullptr;
    ACTION_RESULT status_type = ARES_IGNORED, post_status = ARES_IGNORED;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), false);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->SelectMoreDangerousThreat(const_cast<ActionProcessed*>(this), me, subject, threat1, threat2);

                if(r.eType > status_type)
                    status_type = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    preResult = const_cast<CBaseCombatCharacter *>(r.m_result);
            }
        }
    }

    if(status_type != ARES_SUPERCEDE)
        OriginalResult = const_cast<CBaseCombatCharacter*>(reinterpret_cast<INextBotAction_ptr>(const_cast<ActionProcessed*>(this))->SelectMoreDangerousThreat(me, subject, threat1, threat2));
    else
        OriginalResult = preResult;

    {
        auto mList = g_pActionManager->GetCallBackList(const_cast<ActionProcessed*>(this), true);
        if(mList && mList->Count() > 0)
        {
            for(int i = 0; i < mList->Count(); i++)
            {
                auto r = mList->Element(i)->SelectMoreDangerousThreat(const_cast<ActionProcessed*>(this), me, subject, threat1, threat2);

                if(r.eType > status_type)
                    post_status = r.eType;

                if(r.eType >= ARES_OVERRIDE)
                    postResult = const_cast<CBaseCombatCharacter *>(r.m_result);
            }
        }
    }

    return (status_type >= ARES_OVERRIDE ? preResult : post_status >= ARES_OVERRIDE ? postResult : OriginalResult);
}
