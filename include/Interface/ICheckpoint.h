#ifndef _HEADER_CHECKPOINT_INCLUDE_
#define _HEADER_CHECKPOINT_INCLUDE_
#include "IBaseEntity.h"

class ICheckpoint 
{
public:
    bool ContainsArea(INavArea *area);

    inline Vector GetMinCorner( void ) const { return m_minCorner; }
    inline Vector GetMaxCorner( void ) const { return m_maxCorner; }

    inline bool IsOutward() const { return m_isOutward; }

private:
    CUtlVector<INavArea*> m_areas;  // Навігаційні області в чекпоінті
    
    // Offset 0x14 (20): Загальна площа всіх областей
    float m_totalSpawnArea;              
    
    // Offset 0x18-0x2C (24-44): Bounding box чекпоінта
    Vector m_minCorner;   // Мінімальний кут (x,y,z)
    Vector m_maxCorner;   // Максимальний кут (x,y,z)
    
    // Offset 0x30 (48): Vector для областей спавну
    CUtlVector<INavArea*> m_spawnAreas;    // Області де можна спавнити гравців
    
    // Offset 0x44 (68): Загальна площа областей спавну
    float m_totalSpawnAreaSize;
    
    // Offset 0x48 (72): Vector для дверей чекпоінта  
    CUtlVector<EHANDLE> m_checkpointDoors;      // Двері що контролюються чекпоінтом
    
    // Offset 0x5C-0x5F (92-95): Флаги стану
    bool m_hasBeenUsed;       // Чи був використаний цей чекпоінт
    bool m_isActive;          // Чи активний чекпоінт (закритий для зомбі)
    bool m_allSurvivorsIn;    // Чи всі вижившi в чекпоінті
    bool m_isOutward;         // Чи це вихідний чекпоінт (до зміни рівня)
};


#endif //_HEADER_CHECKPOINT_INCLUDE_