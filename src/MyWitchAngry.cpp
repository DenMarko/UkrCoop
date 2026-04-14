#include "MyWitch.h"

WitchAngry::WitchAngry(float rage) :
    z_witch_hostile_at_me_anger("z_witch_hostile_at_me_anger"),
    z_witch_anger_rate("z_witch_anger_rate"),
    z_witch_threat_normal_range("z_witch_threat_normal_range"),
    z_witch_threat_hostile_range("z_witch_threat_hostile_range"),
    z_witch_relax_rate("z_witch_relax_rate"),
    z_difficulty("z_difficulty")
{
    m_timer1.Invalidate();
    m_timer2.Invalidate();
    m_timer2.Start(rage);
}

void WitchAngry::Growl(IWitch *me)
{
    const char *szNameSound;
    float m_rage = access_member<float>(me, 3632);
    if(m_rage < 0.3f)
    {
        szNameSound = "WitchZombie.GrowlLow";
    }
    else
    {
        if(m_rage < 0.6f)
        {
            szNameSound = "WitchZombie.GrowlMedium";
            if(flCurentRage < 0.3f && m_timer2.GetTargetTime() != -1.f)
            {
                m_timer2.Invalidate();
            }
        }
        else
        {
            szNameSound = "WitchZombie.GrowlHigh";
            if(flCurentRage < 0.6f && m_timer2.GetTargetTime() != -1.f)
            {
                m_timer2.Invalidate();
            }
        }
    }

    if(m_timer2.IsElapsed())
    {
        me->EmitSound(szNameSound);
        if(m_rage >= 0.6)
        {
            m_timer2.Start(RandomFloat(1.5, 2));
        }
        else
        {
            m_timer2.Start(RandomFloat(0.75, 1.5));
        }
    }

    IBody *pbody = me->GetBodyInterface();
    if(pbody)
    {
        if(flCurentRage >= 0.54f || m_rage < 0.54f)
        {
            if(flCurentRage >= 0.54f && m_rage < 0.54f)
            {
                if(!pbody->IsActivity(ACT_TERROR_WITCH_ANGRY))
                {
                    pbody->StartActivity(ACT_TERROR_WITCH_ANGRY, 1u);
                }
            }
        }
        else
        {
            if(!pbody->IsActivity(ACT_TERROR_WITCH_ANGRY_HIGH))
            {
                pbody->StartActivity(ACT_TERROR_WITCH_ANGRY_HIGH, 1u);
            }
        }
    }

    flCurentRage = m_rage;
}

ActionResult<IWitch> WitchAngry::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    IBody *pBody = me->GetBodyInterface();
    if(pBody)
    {
        pBody->SetArousal(IBody::ALERT);
        pBody->StartActivity(ACT_TERROR_WITCH_ANGRY, 1u);
        pBody->SetDesiredPosture(IBody::STAND);
    }
    m_timer1.Start(1.0f);
    flCurentRage = 0.f;
    me->SetFlexWeight("wrinkler", 1.0f);
    CAI_Concept concepts("WitchStartAngry");
    CFollowupTargetSpec_t targetSpec;
    targetSpec.m_iTargetType = kDRT_ANY;
    targetSpec.m_hHandle.Term();

    g_CallHelper->CResponseQueue_Add(concepts, nullptr, 0.f, targetSpec, me);

    return Continue();
}

void WitchAngry::OnEnd(IWitch *me, Action<IWitch> *nextAction)
{
    me->SetFlexWeight("wrinkler", 0.0f);
}

ActionResult<IWitch> WitchAngry::OnSuspend(IWitch *me, Action<IWitch> *interruptingAction)
{
    me->SetFlexWeight("wrinkler", 0.0f);
    return Continue();
}

ActionResult<IWitch> WitchAngry::OnResume(IWitch *me, Action<IWitch> *interruptingAction)
{
    me->SetFlexWeight("wrinkler", 1.0f);
    return Continue();
}

EventDesiredResult<IWitch> WitchAngry::OnCommandAttack(IWitch *me, CBaseEntity *victim)
{
    return TryContinue(RESULT_TRY);
}

EventDesiredResult<IWitch> WitchAngry::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    if(other)
    {
        ITerrorPlayer *pPlayer = (ITerrorPlayer*)other;
        if(pPlayer->IsAlive() && pPlayer->IsPlayer())
        {
            float flRage = (z_witch_hostile_at_me_anger.GetFloat() * z_witch_anger_rate.GetFloat() * g_pGlobals->frametime);
            me->ChangeRageLevel(flRage);
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchAngry::OnBlinded(IWitch *me, CBaseEntity *blinder)
{
    Msg("[WitchAngry::OnBlinded]\n");
    me->ChangeRageLevel(1.0f);
    return TryChangeTo(new WitchBlinded(new WitchAngry(0.0f), "WitchZombie.GrowlHigh"), RESULT_CRITICAL);
}

EventDesiredResult<IWitch> WitchAngry::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    me->ChangeRageLevel(1.f);

    ITerrorPlayer *pPlayer = (ITerrorPlayer*)info.GetAttacker();
    if(!pPlayer || pPlayer->GetTeamNumber() != 2)
        return TryContinue();

    if(g_pWitchEvil->IsWitchEvil(me->entindex()))
    {
        return TrySuspendFor(new WitchEvilAttack(pPlayer));
    }
    return TrySuspendFor(new WitchAttack(pPlayer));
}

EventDesiredResult<IWitch> WitchAngry::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    me->ChangeRageLevel(1.0f);

    if(!pusher) {
        return TryContinue();
    }

    ITerrorPlayer* pTerror = nullptr;
    if((pTerror = access_dynamic_cast<ITerrorPlayer>((IBasePlayer*)pusher, "CTerrorPlayer")) != nullptr && 
        pTerror->GetTeamNumber() == 3)
    {
        return TryContinue();
    }

    if(g_pWitchEvil->IsWitchEvil(me->entindex()))
    {
        return TrySuspendFor(new WitchEvilAttack((ITerrorPlayer*)pusher));
    }
    return TrySuspendFor(new WitchAttack((ITerrorPlayer*)pusher));
}

ActionResult<IWitch> WitchAngry::Update(IWitch *me, float interval)
{
    Growl(me);
    if(me->GetTimeSinceLastInjury(2) < 5.0f)
    {
        ITerrorPlayer *pPlayer = reinterpret_cast<ITerrorPlayer*>(me->GetLastAttacker());
        if(pPlayer)
        {
            if(pPlayer->GetTeamNumber() == 2)
            {
                if(pPlayer->IsAlive())
                {
                    if(g_pWitchEvil->IsWitchEvil(me->entindex()))
                    {
                        return SuspendFor(new WitchEvilAttack(pPlayer), "Resumed Angry when recently attacked");
                    }
                    return SuspendFor(new WitchAttack(pPlayer), "Resumed Angry when recently attacked");
                }
            }
        }
    }

    float &m_rage = access_member<float>(me, 3632);
    IVision *pVision = me->GetVisionInterface();
    ITerrorPlayer* pPlayer = (ITerrorPlayer*)pVision->GetPrimaryRecognizedThreat();

    if(pPlayer)
    {
        float flNormRange = z_witch_threat_normal_range.GetFloat();
        float flRange = me->IsHostileToMe(pPlayer) ? z_witch_threat_hostile_range.GetFloat() : flNormRange;
        if(!me->MyNextBotPointer()->IsRangeGreaterThan((CBaseEntity*)pPlayer, flRange + 50.f)) {
            me->GetBodyInterface()->AimHeadTowards((CBaseEntity*)pPlayer, IBody::IMPORTANT, 1.f);
            
            float flAtMeAnger = 1.f;
            if(me->IsHostileToMe(pPlayer))
            {
                IBaseCombatCharacter *pCharacter = (IBaseCombatCharacter*)pPlayer->MyCombatCharacterPointer();
                if(pCharacter && pCharacter->IsLookingTowards((CBaseEntity*)me, 0.98f))
                    flAtMeAnger = z_witch_hostile_at_me_anger.GetFloat();
                else
                    flAtMeAnger = 0.5 * z_witch_hostile_at_me_anger.GetFloat();
            }

            float flRangeTo = me->MyNextBotPointer()->GetRangeTo((CBaseEntity *)pPlayer);
            if(flRangeTo > flNormRange)
            {
                flAtMeAnger = flAtMeAnger * (1.0f - (flRangeTo - flNormRange) / (z_witch_threat_hostile_range.GetFloat() - flNormRange));
            }

            if(g_Sample.my_bStrcmp(z_difficulty.GetString(), "Easy"))
            {
                flAtMeAnger = flAtMeAnger * 0.5;
            }

            me->ChangeRageLevel((flAtMeAnger * z_witch_anger_rate.GetFloat() * interval));
            if(m_timer1.IsElapsed() && m_rage >= 1.f)
            {
                if(g_pWitchEvil->IsWitchEvil(me->entindex()))
                {
                    return SuspendFor(new WitchEvilAttack(pPlayer));
                }

                return SuspendFor(new WitchAttack(pPlayer));
            }
            return Continue();
        }
    }

    if(!m_timer1.IsElapsed())
    {
        me->ChangeRageLevel(1.0f * z_witch_anger_rate.GetFloat() * interval);
        return Continue();
    }

    me->ChangeRageLevel(-z_witch_relax_rate.GetFloat() * interval);
    if(m_rage <= 0.0f)
    {
        return ChangeTo(new WitchIdle());
    }

    if(m_timer1.IsElapsed() && m_rage >= 1.f && pPlayer)
    {
        if(g_pWitchEvil->IsWitchEvil(me->entindex()))
        {
            return SuspendFor(new WitchEvilAttack(pPlayer));
        }
        return SuspendFor(new WitchAttack(pPlayer));
    }

    return Continue();
}
