#include "ICheckpoint.h"

bool ICheckpoint::ContainsArea(INavArea *area)
{
    if(m_areas.Count() <= 0) return false;

    for(int i = 0; i < m_areas.Count(); ++i)
    {
        if(m_areas[i] == area)
        {
            return true;
        }
    }

    return false;
}