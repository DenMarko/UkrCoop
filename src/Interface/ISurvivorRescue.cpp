#include "ISurvivorRescue.h"
#include "INavMesh.h"

bool ISurvivorRescue::IsBehindClosedDoor(void)
{
    if(m_hDoorList.Count())
    {
        for (int iCount = 0; iCount < m_hDoorList.Count(); iCount++)
        {
            IBaseEntity *iEnt = m_hDoorList[iCount].Get();
            if(iEnt && iEnt->ClassMatches("prop_door_rotating"))
            {
                DoorState_t m_eDoorState = access_member<DoorState_t>(iEnt, 1664);
                if(m_eDoorState == DOOR_STATE_CLOSED)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool ISurvivorRescue::CloseDoors(void)
{
	if(m_hDoorList.Count())
	{
        for (int iCount = 0; iCount < m_hDoorList.Count(); iCount++)
        {
            IBaseEntity *iEnt = m_hDoorList[iCount].Get();
            if(iEnt && iEnt->ClassMatches("prop_door_rotating"))
            {
                DoorState_t m_eDoorState = access_member<DoorState_t>(iEnt, 1664);
                if(m_eDoorState == DOOR_STATE_OPEN)
                {
                    variant_t mVar;
                    iEnt->AcceptInput("Close", nullptr, nullptr, mVar, 0);
                }
            }
        }
	}

    return true;
}

IBaseEntity *ISurvivorRescue::GetSurvivor() const
{
    return m_survivor;
}

INavArea *ISurvivorRescue::GetRescueNavArea(void)
{
    auto pNavMesh = g_HL2->GetTheNavMesh();
    return pNavMesh->GetNearestNavArea(GetAbsOrigin(), false, 350.0, true, true);
}
