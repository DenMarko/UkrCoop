#include "MyWitch.h"

WitchRetreat::WitchRetreat() : 
    m_minRetreatDuration("z_witch_retreat_min_duration"), 
    m_exitRange("z_witch_retreat_exit_range"), 
    m_exitHiddenDuration("z_witch_retreat_exit_hidden_duration"),
    m_maxRetreatRange("z_witch_max_retreat_range")
{}

ActionResult<IWitch> WitchRetreat::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    IBody *pBody = me->GetBodyInterface();
    pBody->SetArousal(IBody::ArousalType::INTENSE);
    pBody->SetDesiredPosture(IBody::PostureType::STAND);
    pBody->StartActivity(ACT_TERROR_WITCH_IDLE_PRE_RETREAT);

    if(BuildRetreatPath(me))
    {
        m_retreatStartTimer.Start();
        m_visibilityCheckTimer.Start();

        return Continue();
    }
    return Done();
}

void WitchRetreat::OnEnd(IWitch *me, Action<IWitch> *nextAction)
{
    WitchStopMusic stop;
    ForEachSurvivor(stop);
}

ActionResult<IWitch> WitchRetreat::Update(IWitch *me, float interval)
{
    auto pLocomotion = me->GetLocomotionInterface();
    auto pBody = me->GetBodyInterface();
    INextBot* pBot = me->MyNextBotPointer();
    if(m_retreatStartTimer.IsGreaterThen(m_minRetreatDuration.GetFloat()))
    {
        if(g_CallHelper->Director_IsVisibleToTeam(me, 2, 0, 0.f, nullptr))
        {
            m_visibilityCheckTimer.Start();
        }

        auto StopAndRemove = [&]()
        {
            WitchStopMusic music;
            ForEachSurvivor(music);
            g_CallHelper->UTIL_Remove(me);
        };

        ITerrorPlayer* pPlayer = GetClosestSurvivor(me->WorldSpaceCenter());
        if(pPlayer == nullptr)
        {
            StopAndRemove();
            return Done();
        }

        if(pBot->IsRangeGreaterThan((CBaseEntity*)pPlayer, m_exitRange.GetFloat()))
        {
            if(m_visibilityCheckTimer.IsGreaterThen(1.f))
            {
                StopAndRemove();
                return Done();
            }
        }

        if(m_visibilityCheckTimer.IsGreaterThen(m_exitHiddenDuration.GetFloat()))
        {
            StopAndRemove();
            return Done();
        }
    }

    if(m_horrorSoundTimer.IsElapsed())
    {
        float flRand = RandomFloat(1.5f, 2.5f);
        m_horrorSoundTimer.Start(flRand);
        me->EmitSound("WitchZombie.RetreatHorror");
    }

    if(pBody->IsActivity(ACT_TERROR_WITCH_IDLE_PRE_RETREAT))
    {
        pLocomotion->Stop();
        pLocomotion->SetVelocity(vec3_origin);
        me->SetAbsVelocity(vec3_origin);
    }
    else
    {
        m_path.Update(pBot);
        if(!pLocomotion->IsClimbingOrJumping() && !pLocomotion->IsUsingLadder() && pLocomotion->IsOnGround())
        {
            if(pBody->IsActualPosture(IBody::PostureType::STAND))
            {
                if(!pBody->IsActivity(ACT_TERROR_WITCH_RETREAT))
                    pBody->StartActivity(ACT_TERROR_WITCH_RETREAT);
            } else if(!pBody->IsActivity(ACT_TERROR_CROUCH_RUN_INTENSE)) {
                pBody->StartActivity(ACT_TERROR_CROUCH_RUN_INTENSE);
            }
        }
    }
    return Continue();
}

EventDesiredResult<IWitch> WitchRetreat::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    if(!other)
    {
        return TryContinue();
    }

    IBaseEntity *pEnt = (IBaseEntity*)other;
    if(g_CallHelper->IsBreakableEntity(other, false, true)
    || pEnt->ClassMatches("prop_door*")
    || pEnt->ClassMatches("func_door*"))
    {
        if(result && result->fraction < 0.8f)
        {
            if(!me->IsPlayingGesture(ACT_TERROR_ATTACK))
            {
                me->AddGesture(ACT_TERROR_ATTACK);
            }
        }
    }
    else
    {
        if(!pEnt->IsPlayer())
        {
            if(!pEnt->MyCombatCharacterPointer() || me->IsPlayingGesture(ACT_TERROR_ATTACK))
            {
                return TryContinue();
            }
            me->AddGesture(ACT_TERROR_ATTACK);
        }
        else
        {
            ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(pEnt);
            if(!pPlayer->IsStaggering())
            {
                auto hPlayer = pPlayer->GetRefEHandle();
                if(hPlayer != nullptr)
                {
                    if(!m_contactedPlayers.HasElement(hPlayer))
                    {
                        m_contactedPlayers.AddToTail(hPlayer);
                        pPlayer->OnStaggered(me);
                        return TryContinue();
                    }
                }

                if(!me->IsPlayingGesture(ACT_TERROR_ATTACK))
                {
                    m_contactedPlayers.RemoveAll();
                    me->AddGesture(ACT_TERROR_ATTACK);
                }
            }
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchRetreat::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchRetreat::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchRetreat::OnAnimationEvent(IWitch *me, animevent_t *event)
{
    if(event->event == 45)
        me->DoAttack(nullptr);

    return TryContinue();
}

EventDesiredResult<IWitch> WitchRetreat::OnMoveToSuccess(IWitch *me, const Path *path)
{
    if(BuildRetreatPath(me))
        return TryContinue();
    else
        return TryDone();
}

EventDesiredResult<IWitch> WitchRetreat::OnMoveToFailure(IWitch *me, const Path *path, MoveToFailureType reason)
{
    if(BuildRetreatPath(me))
        return TryContinue();
    else
        return TryDone();
}

INavArea *WitchRetreat::FindSafeArea(IWitch *me)
{
    WitchSafeScan scan(me);
    INextBot *pBot = me->MyNextBotPointer();

    SearchSurroundingAreas((INavArea*)me->GetLastKnownArea(), pBot->GetPosition(), scan, m_maxRetreatRange.GetFloat());

    return scan.GetBestArea();
}

bool WitchRetreat::BuildRetreatPath(IWitch *me)
{
    INavArea *pArea = FindSafeArea(me);
    if(!pArea) {
        return false;
    }

    INextBot* pBot = me->MyNextBotPointer();
    WitchPathCost cost(pBot);

    return m_path.Compute(pBot, pArea->GetCenter(), cost);
}
