#include "MyWitch.h"

class EventKillWitch : public EVENTS::CBaseEvent
{
public:
    EventKillWitch() : EVENTS::CBaseEvent("witch_killed") { }
    virtual ~EventKillWitch() { }

    void Set(int nUserid, int nWitchid, bool bOneshot)
    {
        if(pEvent)
        {
            pEvent->SetInt("userid", nUserid);
            pEvent->SetInt("witchid", nWitchid);
            pEvent->SetBool("oneshot", bOneshot);
        }
    }
};

extern void CalculateMeleeDamageForce( CTakeDamageInfo *info, const Vector &vecMeleeDir, const Vector &vecForceOrigin, float flScale );

class MyZombieDead
{
    IBaseEntity *m_Victim;
    unsigned int m_nValue;

    ConVarRef near_range;
    ConVarRef far_range;
public:
    MyZombieDead(IBaseEntity *pVictim, unsigned int val);
    virtual bool operator()(IBaseEntity* i);
};

MyZombieDead::MyZombieDead(IBaseEntity *pVictim, unsigned int val)
  : m_Victim(pVictim), 
    m_nValue(val), 
    near_range("intensity_enemy_death_near_range"),
    far_range("intensity_enemy_death_far_range")
{}

bool MyZombieDead::operator()(IBaseEntity *i)
{
    if(!i->IsAlive() || i->GetTeamNumber() != 2)
    {
        return true;
    }

    float flRadius = (i->GetAbsOrigin() - m_Victim->GetAbsOrigin()).NormalizeInPlace();
    Intensity *pIntensity = reinterpret_cast<Intensity*>((char*)i + 7288);

    if(near_range.GetFloat() <= flRadius)
    {
        if(far_range.GetFloat() <= flRadius)
        {
            return true;
        }

        if((m_nValue - 4) <= 1)
        {
            pIntensity->Increase(Intensity::High);
            return true;
        }
    }
    else
    {
        if((m_nValue - 4) <= 1)
        {
            pIntensity->Increase(Intensity::Max);
            return true;
        }

        if(m_nValue)
        {
            pIntensity->Increase(Intensity::Medium);
            return true;
        }

        if(m_Victim->GetAbsVelocity().LengthSqr() > 22500.f)
        {
            pIntensity->Increase(Intensity::Medium);
            return true;
        }
    }

    pIntensity->Increase(Intensity::Low);
    return true;
}

WitchDying::WitchDying(const CTakeDamageInfo &info)
{
    m_info = info;

    IBaseEntity* pEnt = (IBaseEntity*)info.GetAttacker();
    if(!pEnt || pEnt->IsPlayer())
    {
        nValue_32 = RandomInt(0, 10);
    }
    else
    {
        nValue_32 = 999;
    }

    PlayerIndex = -1;
    if(pEnt)
    {
        if(pEnt->IsPlayer())
            PlayerIndex = engine->GetPlayerUserId(pEnt->edict());
    }
}

ActionResult<IWitch> WitchDying::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    if(!me->GetLifeState())
    {
        me->Event_Killed(m_info);
    }

    MyZombieDead dead(me, 4);

    for(int i = 0; i <= g_pGlobals->maxClients; ++i)
    {
        IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(i);
        if(!pPlayer)
            continue;

        if(pPlayer->IsPlayer() && pPlayer->IsConnected() && !dead(pPlayer))
            break;
    }

    float v10 = access_member<float>(me, 911*4);
    DWORD v11 = access_member<DWORD>(me, 915*4);
    bool m_oneshot = false;

    if(v11 > 0)
    {
        DWORD *v12 = (DWORD*)*((DWORD*)me + 912);
        if(*v12 != static_cast<unsigned int>(PlayerIndex))
        {
            bool found = false;
            DWORD v14 = 0;
            DWORD v13 = 0;
            for(v14 = 0, v13 = 1; v14 < v11; ++v14, ++v13)
            {
                if(v12[v13] == static_cast<unsigned int>(PlayerIndex))
                {
                    found = true;
                    break;
                }
            }
            
            if(!found)
            {
                m_oneshot = false;
            }
        }

        m_oneshot = false;
        if(v11 == 1)
        {
            m_oneshot = v10 + 0.1 > g_pGlobals->curtime;
        }
    }

    EventKillWitch *pWitchKill = new EventKillWitch;
    pWitchKill->Set(PlayerIndex, me->entindex(), m_oneshot);
    delete pWitchKill;

    if(!TryToStartDeathThroes(me))
    {
        BecomeRagdoll(me);
    }
    me->EmitSound("WitchZombie.Die");

    return Continue();
}

ActionResult<IWitch> WitchDying::Update(IWitch *me, float interval)
{
    QAngle angRotate, angTest(0.f, interval * flValue_31, 0.f);

    angRotate = angTest + me->GetLocalAngles();
    me->SetLocalAngles(angRotate);

    return Continue();
}

EventDesiredResult<IWitch> WitchDying::OnAnimationActivityComplete(IWitch *me, int activity)
{
    if (activity == ACT_TERROR_DIE_FROM_STAND)
    {
        me->BecomeRagdoll(CTakeDamageInfoHack(), vec3_origin);
    }

    volatile int check = activity;

    if(check >= ACT_TERROR_DIE_FROM_STAND 
    && activity >= ACT_TERROR_DIE_WHILE_RUNNING 
    && activity <= ACT_TERROR_DIE_RIGHTWARD_FROM_SHOTGUN)
    {
        me->BecomeRagdoll(CTakeDamageInfoHack(), vec3_origin);
    }
    return TryContinue();
}

EventDesiredResult<IWitch> WitchDying::OnAnimationEvent(IWitch *me, animevent_t *event)
{
    if(event->event == 47)
    {
        if(--nValue_32 <= 0)
        {
            me->BecomeRagdoll(CTakeDamageInfoHack(), vec3_origin);
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchDying::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    IBaseEntity *pOther = (IBaseEntity*)other;
    if(other)
    {
        if(pOther->MyNextBotPointer())
        {
            if(pOther->IsPlayer())
            {
                ComputeShoveForce(me, pOther->WorldSpaceCenter());
            }
            me->BecomeRagdoll(CTakeDamageInfoHack(), vec3_origin);
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchDying::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    IBaseEntity* pShover = (IBaseEntity*)pusher;
    ComputeShoveForce(me, pShover->GetAbsOrigin());

    me->BecomeRagdoll(CTakeDamageInfoHack(), vec3_origin);

    return TryContinue();
}

bool WitchDying::TryToStartDeathThroes(IWitch *me)
{
    if(PlayerIndex != -1)
    {
        IBaseCombatCharacter *pPlayer = GetVirtualClass<IBaseCombatCharacter>(m_info.GetAttacker());
        if(pPlayer
        && pPlayer->IsPlayer()
        && pPlayer->GetTeamNumber() == 3
        && pPlayer->GetClass() == ZombieClassTank)
        {
            return false;
        }
    }

    if(!me->IsOnFire() && (m_info.GetDamageType() & DMG_BLAST) == 0)
    {
        IWitchBody *pBody = me->GetBodyInterface();
        if(pBody)
        {
            if(pBody->IsArousal(IBody::INTENSE))
            {
                Activity activity = ACT_INVALID;
                if(me->GetAbsVelocity().LengthSqr() > 22500.f)
                {
                    flValue_31 = RandomFloat(-70, 70);
                    activity = ACT_TERROR_DIE_WHILE_RUNNING;
                }
                else
                {
                    flValue_31 = 0;
                    activity = ACT_TERROR_DIE_FROM_STAND;
                }

                if(pBody->StartActivity(activity, 5u))
                {
                    m_info.SetDamageForce(vec3_origin);
                    return true;
                }
            }
        }
    }

    return false;
}

void WitchDying::ComputeShoveForce(IWitch *me, const Vector &ShoveDir)
{
    IBaseCombatCharacter* pCharacter = (IBaseCombatCharacter*)me->GetEntity();
    const Vector &vec_worldSpaceCenter = pCharacter->WorldSpaceCenter();
    Vector vecMeleDir = (vec_worldSpaceCenter - ShoveDir);
    vecMeleDir.NormalizeInPlace();

    CTakeDamageInfoHack info(pCharacter, pCharacter, 50.f, 128, vec_worldSpaceCenter, (vecMeleDir * 3.f));

    CalculateMeleeDamageForce(&info, vecMeleDir, ShoveDir, 1.0);

    m_info = info;
}

bool WitchDying::BecomeRagdoll(IWitch *me)
{
    return me->BecomeRagdoll(CTakeDamageInfoHack(), vec3_origin);
}

bool WitchDying::IsStumbling(IWitch *me)
{
    Activity iActivity = me->GetBodyInterface()->GetActivity();
    return (iActivity >= ACT_TERROR_SHOVED_FORWARD && iActivity <= ACT_TERROR_SHOVED_LEFTWARD);
}
