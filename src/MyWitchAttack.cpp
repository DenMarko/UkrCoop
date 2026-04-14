#include "MyWitch.h"

const char *SurvivorCharacterName(int8_t character)
{
    switch (character)
    {
    case 0:
        return "NamVet";
    case 1:
        return "TeenGirl";
    case 2:
        return "Biker";
    case 3:
        return "Manager";
    }

    return "Unknown";
}

WitchAttack::WitchAttack(ITerrorPlayer *pAttacker, bool bStartSoundPlay) : 
    m_path(IChasePath::LEAD_SUBJECT),
    m_soundPlay(bStartSoundPlay), 
    m_ChangeVictim("z_witch_allow_change_victim"), 
    m_AttackRange("z_witch_attack_range")
{
    SetVictim(pAttacker);
}

EventDesiredResult<IWitch> WitchAttack::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    if(!other)
    {
        return TryToSustain();
    }

    ITerrorPlayer *pEnt = (ITerrorPlayer*)other;
    if(g_CallHelper->IsBreakableEntity(other, false, true)
    || pEnt->ClassMatches("prop_door*")
    || pEnt->ClassMatches("func_door*"))
    {
        if(result && result->fraction < 0.8f)
        {
            if( !me->IsPlayingGesture(ACT_TERROR_ATTACK) )
                me->AddGesture(ACT_TERROR_ATTACK, true);
        }

        return TryToSustain();
    }

    if(!pEnt->IsPlayer())
    {
        if(pEnt->MyCombatCharacterPointer() || pEnt->GetTeamNumber() != 2)
            if(!me->IsPlayingGesture(ACT_TERROR_ATTACK))
                me->AddGesture(ACT_TERROR_ATTACK, true);
    
        return TryToSustain();
    }

    auto pVictim = GetVictim();
    if(pVictim)
    {
        if(pVictim->entindex() != pEnt->entindex())
        {
            if(!pEnt->IsStaggering())
            {
                const CBaseHandle &hEntity = pEnt->GetRefEHandle();
                if(hEntity != nullptr)
                {
                    if(!m_PlayerColect.HasElement(hEntity))
                    {
                        pEnt->OnStaggered(me);
                        m_PlayerColect.AddToTail(pEnt->GetRefEHandle());
                        return TryToSustain();
                    }
                }

                if(m_ChangeVictim.GetBool())
                {
                    if(m_interval2.IsGreaterThen(5.f))
                    {
                        m_PlayerColect.RemoveAll();
                        m_interval2.Start();
                        SetVictim(pEnt);
                        m_path.Invalidate();
                    }
                }
            }
        }
    }

    return TryToSustain();
}

ActionResult<IWitch> WitchAttack::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    ITerrorPlayer *pPlayerByCharacter = nullptr;
    CHandle<ITerrorPlayer> &hValues = access_member<CHandle<ITerrorPlayer>>(me, 910*4);
    if(hValues.Get() != nullptr)
    {
        if(hValues->IsPlayer())
            pPlayerByCharacter = hValues.Get();
    }

    if(!pPlayerByCharacter)
    {
        pPlayerByCharacter = GetVictim();
        if(pPlayerByCharacter && pPlayerByCharacter->IsPlayer())
        {
            auto other_responce = pPlayerByCharacter->GetOtherResponsibleForMovement();
            if(other_responce)
            {
                pPlayerByCharacter = other_responce;
            }
        }
    }

    me->SetHarasser(pPlayerByCharacter);

    auto pVictim = GetVictim();
    if(pVictim)
    {
        if(pVictim->IsPlayer())
        {
            if(pVictim->GetTeamNumber() == 2 && !me->IsOnFire())
            {
                WitchAttackMusic OnMusic;
                ForEachSurvivor(OnMusic);
            }
        }
    }

    IBody *pBody = me->GetBodyInterface();
    pBody->SetArousal(IBody::INTENSE);
    pBody->SetDesiredPosture(IBody::STAND);
    if(m_soundPlay)
        me->EmitSound("WitchZombie.Scream");

    m_timer1.Start(1.0f);
    m_interval1.Start();

    me->SetFlexWeight("bite", 1.0f);

    AI_CriteriaSet *criter = new AI_CriteriaSet();
    if(pPlayerByCharacter)
    {
        int8_t m_survivorCharacter = access_member<int8_t>(pPlayerByCharacter, 6472);
        criter->AppendCriteria(SurvivorCharacterName(m_survivorCharacter));
    }
    CAI_Concept mConcept("WitchStartAttack");
    CFollowupTargetSpec_t targetSpec;
    targetSpec.m_iTargetType = kDRT_ANY;
    targetSpec.m_hHandle.Term();

    g_CallHelper->CResponseQueue_Add(mConcept, criter, 0, targetSpec, me);

    return Continue();
}

ActionResult<IWitch> WitchAttack::Update(IWitch *me, float interval)
{
    auto pLocomotio = me->GetLocomotionInterface();
    auto pBody = me->GetBodyInterface();
    auto pBot = me->MyNextBotPointer();

    auto pVictim = GetVictim();
    if(!pVictim || !pVictim->IsPlayer())
    {
        if(me->IsOnFire())
        {
            return Done("Lost our victim");
        }
        return SuspendFor(new WitchRetreat(), "Lost our victim");
    }

    pBody->AimHeadTowards((CBaseEntity*)pVictim, IBody::IMPORTANT, 1.f);
    if(!pVictim->IsAlive())
    {
        if(pVictim->GetTeamNumber() == 2)
        {
            if(me->IsOnFire())
            {
                return Done("Killed our Survivor victim");
            }

            return ChangeTo(new WitchRetreat(), "Retreating in horror from killing a Survivor");
        }
        else
        {
            return Done("Killed Infected that was harrassing me");
        }
    }

    me->ChangeRageLevel(1.f);
    if(m_timer1.IsElapsed())
    {
        me->EmitSound("WitchZombie.Rage");
        m_timer1.Start(RandomFloat(1.f, 2.f));
    }

    if(pVictim->IsStill(true) 
    && !pLocomotio->IsScrambling() 
    && !pBot->IsRangeGreaterThan((CBaseEntity*)pVictim, 30.f))
    {
        pLocomotio->SetVelocity(vec3_origin);
        me->SetAbsVelocity(vec3_origin);
        pLocomotio->FaceTowards(pVictim->GetAbsOrigin());
        if(pVictim->IsIncapacitated())
        {
            me->EmitSound("WitchZombie.RageBeforeKillingFrenzy");
            return SuspendFor(new WitchStandingAction(new WitchKillIncapVictim(pVictim), ACT_TERROR_WITCH_KILL_DISPLAY, pVictim), "Victim is incapacitated - going in for the kill");
        }
    }
    else
    {
        if(pVictim->IsIncapacitated() && pBot->GetRangeSquaredTo((CBaseEntity*)pVictim) <= 3500)
        {
            pLocomotio->Walk();
        }
        else
        {
            pLocomotio->Run();
        }
        WitchPathCost cost(pBot);

        m_path.Update(pBot, pVictim, cost);
    }

    if(!pLocomotio->IsScrambling())
    {
        float flAttackRang = 1.5 * m_AttackRange.GetFloat();
        if(pBot->IsRangeLessThan((CBaseEntity*)pVictim, flAttackRang))
        {
            if(!me->IsPlayingGesture(ACT_TERROR_ATTACK))
            {
                auto pVision = me->GetVisionInterface();
                if(pVision->IsLineOfSightClearToEntity((CBaseEntity*)pVictim))
                {
                    me->AddGesture(ACT_TERROR_ATTACK);
                    m_PlayerColect.RemoveAll();
                }
            }
        }

        if(pBody->IsActualPosture(IBody::STAND))
        {
            if(me->IsOnFire())
            {
                if(!pBody->IsActivity(ACT_TERROR_RUN_ON_FIRE_INTENSE))
                {
                    pBody->StartActivity(ACT_TERROR_RUN_ON_FIRE_INTENSE);
                }
            }
            else if(!pBody->IsActivity(ACT_TERROR_RUN_INTENSE))
            {
                pBody->StartActivity(ACT_TERROR_RUN_INTENSE);
            }
        }
        else if(!pBody->IsActivity(ACT_TERROR_CROUCH_RUN_INTENSE))
        {
            pBody->StartActivity(ACT_TERROR_CROUCH_RUN_INTENSE);
        }
    }

    return Continue();
}

EventDesiredResult<IWitch> WitchAttack::OnMoveToFailure(IWitch *me, const Path *path, MoveToFailureType reason)
{
    if(reason)
    {
        if(reason == FAIL_FELL_OFF)
        {
            m_path.Invalidate();
        }
        return TryContinue();
    }

    if(me->IsOnFire())
    {
        return TryDone(RESULT_IMPORTANT, "Attack path failed while we're on fire");
    }

    return TryChangeTo(new WitchRetreat(), RESULT_CRITICAL);
}

void WitchAttack::SetVictim(ITerrorPlayer *victim)
{
    if(victim)
    {
        hAttacker = victim->GetRefEHandle();
        if(victim->IsPlayer() && victim->GetTeamNumber() == 2)
        {
            cCharacter = access_member<int8_t>(victim, 6472);
        }
        else
        {
            cCharacter = 4;
        }
    }
    else
    {
        hAttacker.Term();
        cCharacter = 4;
    }
}

ITerrorPlayer *WitchAttack::GetVictim()
{
    ITerrorPlayer *pVictim = nullptr;
    if(cCharacter != 4)
    {
        pVictim = ITerrorPlayer::GetPlayerByCharacter(hAttacker.Get(), cCharacter);
        if(pVictim == nullptr)
            pVictim = hAttacker.Get();
    }
    else
    {
        pVictim = hAttacker.Get();
    }

    if(pVictim)
        return pVictim;
        
    return nullptr;
}

EventDesiredResult<IWitch> WitchAttack::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchAttack::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchAttack::OnMoveToSuccess(IWitch *me, const Path *path)
{
    auto pBody = me->GetBodyInterface();
    if(!pBody->IsActivity(ACT_TERROR_RUN_INTENSE))
        pBody->StartActivity(ACT_TERROR_RUN_INTENSE, 0);

    return TryContinue();
}

EventDesiredResult<IWitch> WitchAttack::OnCommandAttack(IWitch *me, CBaseEntity *victim)
{
    return TryToSustain(RESULT_TRY);
}

void WitchAttack::OnEnd(IWitch *me, Action<IWitch> *nextAction)
{
    me->RemoveGesture(ACT_TERROR_ATTACK);
    me->SetFlexWeight("bite", 0.0);
}

EventDesiredResult<IWitch> WitchAttack::OnAnimationEvent(IWitch *me, animevent_t *pEvent)
{
    if(pEvent->event == 45)
    {
        me->DoAttack(hAttacker.Get());
    }
    return TryContinue();
}
