#include "IBaseAbility.h"

void IBaseAbility::StartActivationTimer(float flDelay, float flDuration)
{
    if(flDuration == 0.f)
    {
        m_nextActivationTimer.Start(flDelay);
    } else {
        m_nextActivationTimer.StartFromTime((g_pGlobals->curtime - flDuration + flDelay), flDuration);
    }
}

void ITongue::ResetTongueTimer()
{
    StartActivationTimer(GetAbilityActivationDelay(), 0.f);
}

float ITongue::GetAbilityActivationDelay()
{
    extern bool IsVersusMode();
    ConVarRef tongue_hit_delay("tongue_hit_delay");
    ConVarRef tongue_miss_delay("tongue_miss_delay");

    IBaseEntity *pOwner = nullptr;
    if( (pOwner = m_owner.Get()) == NULL)
        return tongue_hit_delay.GetFloat();

    CHandle<IBaseEntity> &m_tongueVictim = access_member<CHandle<IBaseEntity>>(pOwner, 8512);
    IBaseEntity *pVictim = nullptr;
    if((pVictim = m_tongueVictim.Get()) == NULL || !IsVersusMode())
        return tongue_hit_delay.GetFloat();

    CHandle<IBaseEntity> &m_tongueOwner = access_member<CHandle<IBaseEntity>>(pVictim, 8516);
    bool &m_isHangingFromTongue = access_member<bool>(pVictim, 8540);
    if(m_tongueOwner != NULL && !m_isHangingFromTongue)
    {
        IntervalTimers &m_tongueVictimTimer = access_member<IntervalTimers>(pVictim, 8520);
        if(m_tongueVictimTimer.IsGreaterThen( 1.f ))
            return tongue_hit_delay.GetFloat();
    }

    if(m_isHangingFromTongue)
        return tongue_hit_delay.GetFloat();

    return tongue_miss_delay.GetFloat();
}

void ITongue::SnapTongueBackToMouth()
{
}
