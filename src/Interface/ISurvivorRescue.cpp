#include "ISurvivorRescue.h"
#include "INavMesh.h"

bool ISurvivorRescue::IsBehindClosedDoor(void)
{
    if(!m_hDoorList.Count())
    {
        return false;
    }

    int iClosedDoors = 0;
    int iTotalDoors = m_hDoorList.Count();
    for (int iCount = 0; iCount < iTotalDoors; iCount++)
    {
        IBaseEntity *iEnt = m_hDoorList[iCount].Get();
        if(!iEnt || !iEnt->ClassMatches("prop_door_rotating"))
        {
            continue;
        }

        DoorState_t m_eDoorState = access_member<DoorState_t>(iEnt, 1664);
        if(m_eDoorState != DOOR_STATE_CLOSED)
        {
            continue;
        }

        iClosedDoors++;
    }

    if(iClosedDoors >= iTotalDoors)
    {
        return true;
    }

    return false;
}

bool ISurvivorRescue::CloseDoors(void)
{
	if(!m_hDoorList.Count())
	{
        return false;
	}

    bool bClosedAnyDoor = false;
    for (int iCount = 0; iCount < m_hDoorList.Count(); iCount++)
    {
        IBaseEntity *iEnt = m_hDoorList[iCount].Get();
        if(!iEnt || !iEnt->ClassMatches("prop_door_rotating"))
        {
            continue;
        }

        DoorState_t m_eDoorState = access_member<DoorState_t>(iEnt, 1664);
        if(m_eDoorState != DOOR_STATE_OPEN)
        {
            continue;
        }

        variant_t variant;
        iEnt->AcceptInput("Close", nullptr, nullptr, variant, 0);
        bClosedAnyDoor = true;
    }

    return bClosedAnyDoor;
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
