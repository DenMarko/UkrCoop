#include "ICheckpoint.h"

bool ICheckpoint::ContainsArea(INavArea *area)
{
    if(m_areas.Count() <= 0) return false;

    if(m_areas[0] == area) return true;

    for(int i = 1; i < m_areas.Count(); ++i)
    {
        if(m_areas[i] == area)
        {
            return true;
        }
    }

    return false;
}