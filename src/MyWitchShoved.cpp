#include "MyWitch.h"

WitchShoved::WitchShoved(IBaseEntity *hShover) : m_vecShow(vec3_origin), z_shove_friend_speed("z_shove_friend_speed")
{
    if(hShover)
    {
        m_hShower = hShover->GetRefEHandle();
        m_vecShow = hShover->WorldSpaceCenter();
    }
    m_pChangeAction = nullptr;
}

WitchShoved::WitchShoved(Action<IWitch> *pNextActio, IBaseEntity *hShover) : m_vecShow(vec3_origin), z_shove_friend_speed("z_shove_friend_speed")
{
    if(hShover)
    {
        m_hShower = hShover->GetRefEHandle();
        m_vecShow = hShover->WorldSpaceCenter();
    }
    m_pChangeAction = pNextActio ? pNextActio : nullptr;
}

ActionResult<IWitch> WitchShoved::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    IWitchLocomotion *pLocomotion = me->GetLocomotionInterface();
    if(pLocomotion->IsUsingLadder())
    {
        auto pNext = m_pChangeAction;
        if(pNext)
        {
            m_pChangeAction = nullptr;
            return ChangeTo(pNext);
        }
        else
        {
            return Done();
        }
    }

    if(!StartShovedActivity(me))
    {
        auto pNext = m_pChangeAction;
        if(pNext)
        {
            m_pChangeAction = nullptr;
            return ChangeTo(pNext);
        }
        else
        {
            return Done();
        }
    }

    me->EmitSound("Weapon.HitInfected");

    if(me->GetTeamNumber() != 3 || me->GetClass() == ZombieClassWitch)
    {
        if(!me->IsOnFire())
            me->EmitSound("WitchZombie.Rage");
    }

    return Continue();
}

EventDesiredResult<IWitch> WitchShoved::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchShoved::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    float flDist = me->GetAbsVelocity().Length();
    if(flDist > z_shove_friend_speed.GetFloat())
    {
        auto pNextBot = reinterpret_cast<IBaseEntity*>(other)->MyNextBotPointer();
        if(pNextBot)
            pNextBot->OnShoved((CBaseEntity*)me);
    }

    if(flDist > 50.0f && result->DidHitWorld() && !result->startsolid && result->startpos.z < 0.5)
    {
        auto pBody = me->GetBodyInterface();
        if(pBody->IsActivity(ACT_TERROR_SHOVED_BACKWARD))
            pBody->StartActivity(ACT_TERROR_SHOVED_BACKWARD_INTO_WALL, 1U);
        else if(pBody->IsActivity(ACT_TERROR_SHOVED_FORWARD))
            pBody->StartActivity(ACT_TERROR_SHOVED_FORWARD_INTO_WALL, 1U);
        else if(pBody->IsActivity(ACT_TERROR_SHOVED_LEFTWARD))
            pBody->StartActivity(ACT_TERROR_SHOVED_LEFTWARD_INTO_WALL, 1U);
        else if(pBody->IsActivity(ACT_TERROR_SHOVED_RIGHTWARD))
            pBody->StartActivity(ACT_TERROR_SHOVED_RIGHTWARD_INTO_WALL, 1U);
    }

    return TryToSustain();
}

EventDesiredResult<IWitch> WitchShoved::OnAnimationActivityComplete(IWitch *me, int activity)
{
    if(activity < ACT_TERROR_SHOVED_FORWARD)
    {
        return TryContinue();
    }

    if(activity > ACT_TERROR_SHOVED_RIGHTWARD_INTO_WALL)
    {
        if(activity <= ACT_TERROR_SHOVED_LEFTWARD_FROM_SIT)
        {
            auto pBody = me->GetBodyInterface();
            pBody->SetDesiredPosture(IBody::STAND);

            auto pNext = m_pChangeAction;
            if(pNext)
            {
                m_pChangeAction = nullptr;
                return TryChangeTo(pNext);
            }
            return TryDone();
        }
        return TryContinue();
    }

    auto pNext = m_pChangeAction;
    if(pNext)
    {
        m_pChangeAction = nullptr;
        return TryChangeTo(pNext);
    }
    return TryDone();
}

EventDesiredResult<IWitch> WitchShoved::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    StartShovedActivity(me);
    return TryToSustain();
}

bool WitchShoved::IsAbleToBlockMovementOf(const INextBot *botInMotion) const
{
    if(botInMotion)
    {
        auto pBody = botInMotion->GetBodyInterface();
        if(pBody)
        {
            if(pBody->IsActivity(ACT_TERROR_SHOVED_FORWARD)
            || pBody->IsActivity(ACT_TERROR_SHOVED_BACKWARD)
            || pBody->IsActivity(ACT_TERROR_SHOVED_LEFTWARD)
            || pBody->IsActivity(ACT_TERROR_SHOVED_RIGHTWARD))
            {
                return false;
            }
        }
    }
    return true;
}

bool WitchShoved::StartShovedActivity(IWitch *me)
{
    QAngle angTo;
    IBody *pBody = me->GetBodyInterface();

    Vector los = m_vecShow - me->GetAbsOrigin();
    los.z = 0.0f;
    VectorNormalize(los);

    IBaseCombatCharacter *pEntity = (IBaseCombatCharacter*)me->GetEntity();
    Vector facingDir = pEntity->BodyDirection2D();
    float flDot = DotProduct(los, facingDir);

    auto TryStartActivityOne = [&](Activity primaryAct, IBody::PostureType desiredPosture) 
    {
        if(pBody->StartActivity(primaryAct, 1U))
        {
            pBody->SetDesiredPosture(desiredPosture);
            return true;
        }
        return false;
    };

    auto TryStartActivityTwo = [&](Activity primaryAct, Activity fallbackAct, IBody::PostureType desiredPosture)
    {
        if(!pBody->StartActivity(primaryAct, 1U))
        {
            if(!pBody->StartActivity(fallbackAct, 1U))
                return false;
    
            pBody->SetDesiredPosture(IBody::STAND);
        }
        pBody->SetDesiredPosture(desiredPosture);
        return true;
    };

    if(flDot > 0.7071f)
    {
        if(!pBody->IsActualPosture(IBody::STAND))
        {
            if(!pBody->IsActualPosture(IBody::SIT))
                return false;

            if(!TryStartActivityTwo(ACT_TERROR_SHOVED_BACKWARD_FROM_SIT, ACT_TERROR_SHOVED_BACKWARD, IBody::SIT))
                return false;
        }
        else 
        {
            if(!TryStartActivityOne(ACT_TERROR_SHOVED_BACKWARD, IBody::STAND))
                return false;
        }

        VectorAngles(los, angTo);
        me->SetAbsAngles(angTo);
        return true;
    }
    else
    {
        if(flDot < -0.7071f)
        {
            if(pBody->IsActualPosture(IBody::STAND) || pBody->IsActualPosture(IBody::SIT))
            {
                if(TryStartActivityOne(ACT_TERROR_SHOVED_FORWARD, IBody::STAND))
                {
                    VectorAngles(-los, angTo);
                    me->SetAbsAngles(angTo);
                    return true;
                }
            }
        }
        else
        {
            float fldot1 = los.y * facingDir.x - facingDir.y * los.x;
            if(fldot1 <= 0.7071f)
            {
                if(pBody->IsActualPosture(IBody::STAND))
                {
                    if(!TryStartActivityOne(ACT_TERROR_SHOVED_LEFTWARD, IBody::STAND))
                        return false;
                }
                else
                {
                    if(!pBody->IsActualPosture(IBody::SIT))
                        return false;

                    if(!TryStartActivityTwo(ACT_TERROR_SHOVED_LEFTWARD_FROM_SIT, ACT_TERROR_SHOVED_LEFTWARD, IBody::SIT))
                        return false;
                }
    
                VectorAngles(Vector(-los.y, los.x, 0.f), angTo);
                me->SetAbsAngles(angTo);
                return true;
            }
            else
            {
                if(pBody->IsActualPosture(IBody::STAND))
                {
                    if(!TryStartActivityOne(ACT_TERROR_SHOVED_RIGHTWARD, IBody::STAND))
                        return false;
                }
                else
                {
                    if(!pBody->IsActualPosture(IBody::SIT))
                        return false;

                    if(!TryStartActivityTwo(ACT_TERROR_SHOVED_RIGHTWARD_FROM_SIT, ACT_TERROR_SHOVED_RIGHTWARD, IBody::SIT))
                        return false;
                }
    
                VectorAngles(Vector(los.y, -los.x, 0.f), angTo);
                me->SetAbsAngles(angTo);
                return true;
            }
        }
    }
    return false;
}
