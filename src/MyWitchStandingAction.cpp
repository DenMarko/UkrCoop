#include "MyWitch.h"

WitchStandingAction::WitchStandingAction(Action<IWitch> *pKillIncalVictim, Activity activity, const char *szSound) : 
    m_ChangeAction(pKillIncalVictim), nActivity(activity), bStartToChange(false), szNameSound(szSound), hVictim(nullptr)
{
}

WitchStandingAction::WitchStandingAction(Action<IWitch> *pKillIncalVictim, Activity activity, const ITerrorPlayer* pVictim) : 
    m_ChangeAction(pKillIncalVictim), nActivity(activity), bStartToChange(false), szNameSound(nullptr)
{
    if(pVictim)
        hVictim = pVictim->GetRefEHandle();
}

WitchStandingAction::WitchStandingAction(Action<IWitch> *pKillIncalVictim, Activity activity, float flTimer) : 
    m_ChangeAction(pKillIncalVictim), nActivity(activity), bStartToChange(false), szNameSound(nullptr), hVictim(nullptr)
{
    m_timer.Start(flTimer);
}

ActionResult<IWitch> WitchStandingAction::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    me->GetBodyInterface()->StartActivity(nActivity, 1u);

    if(szNameSound)
        me->EmitSound(szNameSound);

    return Continue();
}

ActionResult<IWitch> WitchStandingAction::Update(IWitch *me, float interval)
{
    if(hVictim)
    {
        if(!hVictim->IsIncapacitated())
        {
            return Done("Our victim stood up.");
        }
    }
    
    if(m_timer.HasStarted() && m_timer.IsElapsed())
    {
        bStartToChange = true;
    }
    
    if(bStartToChange)
    {
        auto pChangeAction = m_ChangeAction;
        if(pChangeAction)
        {
            m_ChangeAction = nullptr;
            return ChangeTo(pChangeAction);
        }
        else
        {
            return Done();
        }
    }

    return Continue();
}

EventDesiredResult<IWitch> WitchStandingAction::OnAnimationActivityComplete(IWitch *me, int activity)
{
    if(nActivity == activity)
        bStartToChange = true;

    return TryContinue();
}
