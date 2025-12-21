#include "ICSPlayer.h"

void ICSPlayer::SetProgressBarTime(int barTime, const char* msg)
{
    int &m_iProgressBarDuration = access_member<int>(this, 5984);
    float &m_flProgressBarStartTime = access_member<float>(this, 5980);

    m_iProgressBarDuration = barTime;
    NetworkStateChanged(5984);
    m_flProgressBarStartTime = this->m_flSimulationTime;
    NetworkStateChanged(5980);

    SetLastProgressBarUpdateTime();
    Q_strncpy((char*)this + 5988, msg, 32);
}

bool ICSPlayer::HasSecondaryWeapon(void) const
{
    return Weapon_GetSlot(1) != nullptr;
}
