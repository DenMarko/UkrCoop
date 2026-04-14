#include "MyWitch.h"

WitchBlinded::WitchBlinded(Action<IWitch> *pAngry, const char *szNameSound)
{
    m_timer1.Invalidate();
    m_timer2.Invalidate();

    m_pNextAction = pAngry;
    m_szNameSound = szNameSound;
}

ActionResult<IWitch> WitchBlinded::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    flValue = RandomFloat(-40.f, 40.f);
    float flRand = RandomFloat(8.f, 12.f);

    m_timer1.Start(flRand);
    me->GetBodyInterface()->SetArousal(IBody::INTENSE);

    return Continue();
}

ActionResult<IWitch> WitchBlinded::Update(IWitch *me, float interval)
{
    auto pBody = me->GetBodyInterface();
    if(!pBody->IsActivity(ACT_TERROR_RUN_ON_FIRE))
    {
        pBody->StartActivity(ACT_TERROR_RUN_ON_FIRE, 1u);
    }

    me->SetLocalAngles(me->GetLocalAngles() + QAngle(0.f, interval + flValue, 0.f));
    if(m_timer1.IsElapsed())
    {
        me->Vocalize(m_szNameSound, true);
        float flRand = RandomFloat(1.f, 3.f);
        m_timer1.Start(flRand);
    }

    if(!m_timer2.IsElapsed() || !m_timer2.HasStarted())
    {
        return Continue();
    }

    auto pNextActions = m_pNextAction;
    if(pNextActions)
    {
        m_pNextAction = nullptr;
        return ChangeTo(pNextActions, "Blindness wore off");
    }
    return Done();
}

EventDesiredResult<IWitch> WitchBlinded::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchBlinded::OnSound(IWitch *me, CBaseEntity *source, const Vector &pos, KeyValues *keys)
{
    return TryToSustain();
}

EventDesiredResult<IWitch> WitchBlinded::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    ITerrorPlayer *pPlayer = ToTerrorPlayer((IBaseEntity*) pusher);
    return TrySuspendFor(new WitchShoved(pPlayer), RESULT_IMPORTANT, "Shoved by someone while blind");
}

EventDesiredResult<IWitch> WitchBlinded::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    IBaseEntity* pOther = (IBaseEntity*)other;
    if(pOther->MyCombatCharacterPointer())
    {
        return TryChangeTo(new WitchShoved(pOther), RESULT_TRY, "Bumped into another Actor while blinded");
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchBlinded::OnBlinded(IWitch *me, CBaseEntity *blinder)
{
    float flRand = RandomFloat(8.f, 12.f);
    if(flRand > m_timer2.GetRemainingTime())
    {
        m_timer2.Start(flRand);
    }

    return TryToSustain();
}
