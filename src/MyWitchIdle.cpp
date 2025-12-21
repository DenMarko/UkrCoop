#include "MyWitch.h"

ActionResult<IWitch> WitchIdle::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    float &m_rage = access_member<float>(me, 3632);
    if(m_rage > 0.0f)
    {
        return ChangeTo(new WitchAngry(0.0f));
    }

    me->SetHarasser(nullptr);
    IWitchBody *pBody = me->GetBodyInterface();
    if(pBody)
    {
        pBody->SetArousal(IBody::NEUTRAL);
        if(!pBody->StartActivity(ACT_TERROR_WITCH_IDLE))
        {
            return ChangeTo(new WitchAngry(0.0f));
        }
        pBody->SetDesiredPosture(IBody::SIT);
    }
    m_soundDespair.Start(2.0f);
    me->SetFlexWeight("presser", 1.0f);

    m_target_set.Invalidate();
    m_Target.Term();

    return Continue();
}

EventDesiredResult<IWitch> WitchIdle::OnBlinded(IWitch *me, CBaseEntity *blinder)
{
    me->ChangeRageLevel(1.0f);
    return TryChangeTo(new WitchBlinded(new WitchAngry(0.0f), "WitchZombie.GrowlHigh"), RESULT_CRITICAL);
}

ActionResult<IWitch> WitchIdle::Update(IWitch *me, float interval)
{
    IWitchLocomotion *pLocomotion = me->GetLocomotionInterface();
    
    pLocomotion->Stop();
    pLocomotion->SetVelocity(vec3_origin);
    me->SetAbsVelocity(vec3_origin);

    if(m_soundDespair.IsElapsed())
    {
        me->EmitSound("WitchZombie.Despair");
        float flRandVal = RandomFloat(4.f, 6.f);
        m_soundDespair.Start(flRandVal);
        g_CallHelper->CSoundEnt_InsertSound(SOUND_DANGER, me->GetAbsOrigin(), 2048, flRandVal, (CBaseEntity*)me, 10);
    }

    float flRang = 0.0;
    INextBot *pBot = me->MyNextBotPointer();
    IVision *pVision = me->GetVisionInterface();
    ITerrorPlayer* pPlayer = (ITerrorPlayer*)pVision->GetPrimaryRecognizedThreat();
    if(!pPlayer)
    {
        if(m_target_set.HasStarted())
        {
            if(m_target_set.IsElapsed())
            {
                pPlayer = m_Target != NULL ? m_Target.Get() : nullptr;
                m_target_set.Invalidate();
            }
        }

        if(!pPlayer)
        {
            pPlayer = GetClosestSurvivor(pBot->GetPosition());
            if(!pPlayer 
            || !pBot->IsRangeLessThan((CBaseEntity*)pPlayer, z_witch_personal_space.GetFloat()) 
            || !pVision->IsLineOfSightClearToEntity((CBaseEntity*)pPlayer))
            {
                return Continue();
            }
        }

        auto pBody = me->GetBodyInterface();
        pBody->AimHeadTowards((CBaseEntity*)pPlayer, IBody::IMPORTANT);
        me->EmitSound("WitchZombie.Surprised");
        flRang = 1.0f;
    }

    float flToBestRang = me->IsHostileToMe(pPlayer) ? z_witch_threat_hostile_range.GetFloat() : z_witch_threat_normal_range.GetFloat();

    if( !pBot->IsRangeLessThan((CBaseEntity*)pPlayer, flToBestRang) )
    {
        return Continue();
    } 

    return ChangeTo(new WitchAngry(flRang));
}

// Called when this action is ended. Sets the "presser" flex weight to 0.
void WitchIdle::OnEnd(IWitch *me, Action<IWitch> *nextAction)
{
    me->SetFlexWeight("presser", 0.0f);
}

EventDesiredResult<IWitch> WitchIdle::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    return TryContinue();
}

EventDesiredResult<IWitch> WitchIdle::OnCommandAttack(IWitch *me, CBaseEntity *victim)
{
    return TryContinue();
}

ActionResult<IWitch> WitchIdle::OnResume(IWitch *me, Action<IWitch> *interruptingAction)
{
    float &m_rage = access_member<float>(me, 3632);
    if(m_rage > 0.0f)
    {
        return ChangeTo(new WitchAngry(0.f), "Resumed Idle while still mad");
    }

    return Continue();
}

EventDesiredResult<IWitch> WitchIdle::OnSound(IWitch *me, CBaseEntity *source, const Vector &pos, KeyValues *keys)
{
    if(!IsValidEnemy(source))
    {
        return TryContinue();
    }

    if(!m_target_set.HasStarted())
    {
        ITerrorPlayer *pPlayer = (ITerrorPlayer*)source;
        if(pPlayer->GetTimeSinceLastFiredWeapon() < 1.0f 
        && (access_member<char>(pPlayer, 11078) & 4) == 0)
        {
            auto pBot = me->MyNextBotPointer();
            float flRangTo = pBot->GetRangeTo(source);
            if(z_witch_threat_normal_range.GetFloat() > flRangTo)
            {
                m_target_set.Start(0.4f);
                m_Target = pPlayer->GetRefEHandle();
            }
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchIdle::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    if(!pusher) {
        return TryContinue();
    }

    ITerrorPlayer* pTerror = nullptr;
    if((pTerror = access_dynamic_cast<ITerrorPlayer>((IBasePlayer*)pusher, "CTerrorPlayer")) != nullptr && pTerror->GetTeamNumber() == 3) {
        return TryContinue();
    }

    if(g_pWitchEvil->IsWitchEvil(me->entindex()))
    {
        return TrySuspendFor(new WitchEvilAttack(pTerror), RESULT_IMPORTANT);
    }

    return TrySuspendFor(new WitchAttack(pTerror), RESULT_IMPORTANT);
}

EventDesiredResult<IWitch> WitchIdle::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    if(info.GetDamage() > 50.f 
    && (info.GetDamageType() & DMG_BULLET) != 0 
    && access_member<int>(me, 1740) == 1)
    {
        if(IsValidEnemy(info.GetAttacker()))
        {
            me->ChangeRageLevel(1.0f);
            me->EmitSound("WitchZombie.HeadShot");

            CBaseEntity* pAttacker = info.GetAttacker();
            IBaseEntity *pAttack = nullptr;
            if(pAttacker)
            {
                pAttack = (IBaseEntity *)pAttacker;
            }
            
            if(g_pWitchEvil->IsWitchEvil(me->entindex()))
            {
                return TrySuspendFor(new WitchShoved(new WitchEvilAttack((ITerrorPlayer*)pAttack), pAttack), RESULT_IMPORTANT, "Reacting to high-powered rifle headshot");
            }

            return TrySuspendFor(new WitchShoved(new WitchAttack((ITerrorPlayer*)pAttack), pAttack), RESULT_IMPORTANT, "Reacting to high-powered rifle headshot");
        }
    }
    else
    {
        ITerrorPlayer *pAttacker = GetVirtualClass<ITerrorPlayer>(info.GetAttacker());
        if(IsValidEnemy(pAttacker))
        {
            if(g_pWitchEvil->IsWitchEvil(me->entindex()))
            {
                return TrySuspendFor(new WitchEvilAttack(pAttacker), RESULT_IMPORTANT, "Attacking Survivor that injured me");
            }
            
            return TrySuspendFor(new WitchAttack(pAttacker), RESULT_IMPORTANT, "Attacking Survivor that injured me");
        }
    }
    return TryContinue();
}
