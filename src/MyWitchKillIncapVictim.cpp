#include "MyWitch.h"

WitchKillIncapVictim::WitchKillIncapVictim(ITerrorPlayer *pVictim) : 
    z_witch_damage_per_kill_hit("z_witch_damage_per_kill_hit"),
    survivor_incap_health("survivor_incap_health"),
    z_difficulty("z_difficulty")
{
    if(pVictim)
        hVictim = pVictim->GetRefEHandle();
}

ActionResult<IWitch> WitchKillIncapVictim::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    auto pBody = me->GetBodyInterface();
    pBody->SetArousal(IBody::ArousalType::INTENSE);
    pBody->SetDesiredPosture(IBody::PostureType::STAND);
    pBody->StartActivity(ACT_TERROR_WITCH_KILL_LOOP, 1u);
    me->EmitSound("WitchZombie.Scream");

    m_timer1.Start(1.f);
    me->SetFlexWeight("bite", 1.0);

    return Continue();
}

ActionResult<IWitch> WitchKillIncapVictim::Update(IWitch *me, float interval)
{
    if(hVictim != NULL)
    {
        if(hVictim == NULL || !hVictim->IsIncapacitated())
        {
            return Done("Our victim stood up.");
        }
        
        INextBot *pBot = me->MyNextBotPointer();
        ITerrorPlayer* pVictim = hVictim.Get();
        if(pBot->GetRangeSquaredTo((CBaseEntity*)pVictim) > 3500.f)
        {
            return Done("Our victim has moved.");
        }

        IBody *pBody = me->GetBodyInterface();
        IWitchLocomotion *pLocomotion = me->GetLocomotionInterface();
        pBody->AimHeadTowards((CBaseEntity*)pVictim, IBody::IMPORTANT);

        pLocomotion->FaceTowards(pVictim->WorldSpaceCenter());
        if(pVictim->IsAlive())
        {
            me->ChangeRageLevel(1.f);
            if(m_timer1.IsElapsed())
            {
                me->EmitSound("WitchZombie.KillingFrenzy");
                float flRand = RandomFloat(.5f, 1.f);
                m_timer1.Start(flRand);
            }
            me->SetAbsVelocity(vec3_origin);
            if(!pBody->IsActivity(ACT_TERROR_WITCH_KILL_LOOP))
                pBody->StartActivity(ACT_TERROR_WITCH_KILL_LOOP, 1U);

            return Continue();
        }
        else
        {
            if(me->IsOnFire())
            {
                return Done("Victim is dead, and I'm on fire");
            }
            else
            {
                return ChangeTo(new WitchRetreat, "Retreating in horror from killing a Survivor");
            }
        }
    }

    return ChangeTo(new WitchRetreat, "Victim became NULL");
}

void WitchKillIncapVictim::OnEnd(IWitch *me, Action<IWitch> *nextAction)
{
    me->RemoveGesture(ACT_TERROR_ATTACK);
    me->SetFlexWeight("bite", 0.f);
}

EventDesiredResult<IWitch> WitchKillIncapVictim::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    return TryToSustain(RESULT_TRY);
}

EventDesiredResult<IWitch> WitchKillIncapVictim::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    return TryToSustain(RESULT_TRY);
}

EventDesiredResult<IWitch> WitchKillIncapVictim::OnCommandAttack(IWitch *me, CBaseEntity *victim)
{
    return TryToSustain(RESULT_TRY);
}

EventDesiredResult<IWitch> WitchKillIncapVictim::OnAnimationEvent(IWitch *me, animevent_t *event)
{
    if(event->event == 45)
    {
        if(hVictim)
        {
            float flDamage = z_witch_damage_per_kill_hit.GetFloat();

            if(g_Sample.my_bStrcmp(z_difficulty.GetString(), "Easy"))
            {
                flDamage = flDamage * 0.5;
            }
            else if(g_Sample.my_bStrcmp(z_difficulty.GetString(), "Hard"))
            {
                flDamage += flDamage;
            }
            else if(g_Sample.my_bStrcmp(z_difficulty.GetString(), "Impossible"))
            {
                flDamage = survivor_incap_health.GetFloat();
            }

            CTakeDamageInfoHack dmg(me, me, flDamage, DMG_SLASH);
            hVictim->TakeDamage(dmg);
            hVictim->EmitSound("WitchZombie.ShredVictim");
        }
    }

    return TryContinue();
}
