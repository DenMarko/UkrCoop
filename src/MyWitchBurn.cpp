#include "MyWitch.h"

WitchBurn::WitchBurn(IBaseEntity *pAttacker)
{
    if(pAttacker)
        hAttacker = pAttacker->GetRefEHandle();
}

ActionResult<IWitch> WitchBurn::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    val1 = RandomFloat(-1.f, 8.5f);
    WitchBurningMusic func;
    ForEachSurvivor(func);

    if(hAttacker)
    {
        if(hAttacker->IsPlayer())
        {
            if(g_pWitchEvil->IsWitchEvil(me->entindex()))
            {
                return SuspendFor(new WitchEvilAttack(hAttacker), "Attacking person who caught me on fire");
            }
            
            return SuspendFor(new WitchAttack(hAttacker), "Attacking person who caught me on fire");
        }
    }
    return Continue();
}

ActionResult<IWitch> WitchBurn::Update(IWitch *me, float interval)
{
    IBody *pBody = me->GetBodyInterface();
    if(!pBody->IsActivity(ACT_TERROR_RUN_ON_FIRE))
        pBody->StartActivity(ACT_TERROR_RUN_ON_FIRE, 1U);

    float v5 = interval * val1;
    QAngle VecAng, angLocal = me->GetLocalAngles();
    VecAng.x = angLocal.x + 0.0f;
    VecAng.y = v5 + angLocal.y;
    VecAng.z = angLocal.z + 0.0f;

    me->SetLocalAngles(VecAng);

    return Continue();
}

EventDesiredResult<IWitch> WitchBurn::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    return TryToSustain(RESULT_TRY);
}

EventDesiredResult<IWitch> WitchBurn::OnBlinded(IWitch *me, CBaseEntity *blinder)
{
    return TryToSustain(RESULT_TRY);
}

EventDesiredResult<IWitch> WitchBurn::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    IBaseEntity* pOther = (IBaseEntity*)other;
    if(other && pOther->IsPlayer())
    {
        GetVirtualClass<ITerrorPlayer>(other)->OnStaggered(me);
    }

    return TryToSustain();
}

EventDesiredResult<IWitch> WitchBurn::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    return TrySuspendFor(new WitchShoved(GetVirtualClass<IBaseEntity>(pusher)));
}
