#include "IBaseTempEntity.h"

const char *IBaseTempEntity::GetName(void)
{
    return m_pszName ? m_pszName : "Unnamed";
}

IBaseTempEntity *IBaseTempEntity::GetNext(void)
{
    return m_pNext;
}