#include "IInfected.h"
#include "HL2.h"

void IInfected::Vocalize(const char *szVocalise, bool bValue)
{
    ConVarRef muteInfected("z_mute_infected");

    if(muteInfected.GetBool())
        return;

    if(!m_vocaliseTimer.IsElapsed())
        return;
    
    ConVarRef zombieMobMax("z_mob_spawn_max_size");
    ConVarRef mixDynamicMaxCIEmitter("mix_dynamic_max_CI_emitters");
    ConVarRef mixDynamicCullMaxCIEmitter("mix_dynamic_cull_max_CI_emitters");

    if(mixDynamicMaxCIEmitter.GetInt() >= zombieMobMax.GetInt() || mixDynamicCullMaxCIEmitter.GetInt() == 0)
    {
        EmitSound(szVocalise);
    }
    else
    {
        EmitSound(m_recipFilter, entindex(), szVocalise);
    }

    float randValue = ::RandomFloat(1.f, 2.f);
    m_vocaliseTimer.Start(randValue);
}

bool IInfected::SetDamagedBodyGroupVariant(const char *bodygroupName, const char *variantSuffix)
{
    int bodygroupIndex = FindBodygroupByName(bodygroupName);
    if(bodygroupIndex < 0 || bodygroupIndex >= GetNumBodyGroups())
        return false;

    int currentValue = GetBodygroup(bodygroupIndex);
    const char *currentPartName = GetBodygroupPartName(bodygroupIndex, currentValue);

    if(currentPartName[0] == 'D')
        return false; // already damaged

    if(currentPartName[0] == 'E')
        currentPartName += 3;

    char partChar = 'X';
    int partNumber = -1;
    if(sscanf(currentPartName, "%c%d", &partChar, &partNumber) != 2)
        return false;

    CUtlVector<int> damagedVariant;
    int bodygroupCount = GetBodygroupCount(bodygroupIndex);

    for(int i = 0; i < bodygroupCount; i++)
    {
        const char *partName = GetBodygroupPartName(bodygroupIndex, i);

        char parsedChar;
        int parsedNumber = -1;
        char variantName[64];

        if(sscanf(partName, "D%c%d_%63s", &parsedChar, &parsedNumber, variantName) == 4)
        {
            if(partName[0] == 'D' && parsedNumber == partNumber &&
               V_strnicmp(variantName, variantSuffix, V_strlen(variantSuffix)) == 0)
            {
                damagedVariant.AddToTail(i);
            }
        }
    }

    if(damagedVariant.Count() > 0)
    {
        int randomIndex = RandomInt(0, damagedVariant.Count() - 1);
        SetBodygroup(bodygroupIndex, damagedVariant[randomIndex]);
        return true;
    }

    return false;
}

void IInfected::AttackSurvivorTeam()
{
    m_mobRush = true;
}

void IInfectedAnimationLayer::Update(IBaseAnimating *pAnim, IAnimationLayer *pLayer)
{
    if(pLayer->IsActive())
    {
        if(pLayer->m_nOrder == 15)
        {
            m_nOrder = 6;
            return;
        }

        if(m_nSequence != pLayer->m_nSequence)
        {
            m_nSequence = pLayer->m_nSequence;
            m_nOrder = pLayer->m_nOrder;
            m_bLooping = pLayer->m_bLooping;
            m_flStartTime = g_pGlobals->curtime;
            m_flCycle = pLayer->m_flCycle;
            return;
        }

        if(m_bLooping != pLayer->m_bLooping)
        {
            if(m_nOrder != 15)
            {
                if(m_bLooping)
                {
                    m_flCycle = pLayer->m_flCycle;
                    return;
                }

                if(m_flCycle > pLayer->m_flCycle)
                {
                    m_nOrder = pLayer->m_nOrder;
                    m_bLooping = pLayer->m_bLooping;
                    m_flStartTime = g_pGlobals->curtime;
                    m_flCycle = pLayer->m_flCycle;
                    return;
                }

                m_flCycle = pLayer->m_flCycle;
                return;
            }

            m_nOrder = pLayer->m_nOrder;
            m_bLooping = pLayer->m_bLooping;
            m_flStartTime = g_pGlobals->curtime;
            m_flCycle = pLayer->m_flCycle;
            return;
        }
    }

    m_nOrder = 6;
    return;
}
