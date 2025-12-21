#include "IBossZombiePlayerBot.h"

class AllHumanSurvivorAreIncapacitatedScan
{
public:
    AllHumanSurvivorAreIncapacitatedScan() : bAllIncapacitated(true) {}

    virtual bool operator()(ITerrorPlayer* pPlayer)
    {
        if(!pPlayer->IsPlayer())
            return true;

        if(!pPlayer->IsConnected())
            return true;

        if(pPlayer->GetTeamNumber() != 2)
            return true;

        if(!pPlayer->IsAlive())
            return true;

        if(!pPlayer->IsIncapacitated())
        {
            if(!pPlayer->IsBot())
            {
                bAllIncapacitated = false;
                return false;
            }
        }

        return true;
    }

    inline bool AllIncapacitated() const { return bAllIncapacitated; }
private:
    bool bAllIncapacitated;
};

class ClosestVisibleSurvivorScan
{
public:
    ClosestVisibleSurvivorScan(
        const Vector &vOrigin, 
        bool bMustBeNotIncapacitated = true, 
        bool bMustBeNotCloseArea = true, 
        IBaseEntity* pIgnore = nullptr
    ) : m_vOrigin(vOrigin), 
        m_pClosest(nullptr), 
        m_flClosestDistSq(1e+08), 
        m_pIgnore(pIgnore), 
        m_bMustBeNotIncapacitated(bMustBeNotIncapacitated), 
        m_bMustBeNotCloseArea(bMustBeNotCloseArea) {}

    virtual bool operator()(ITerrorPlayer* pPlayer)
    {
        if(m_pIgnore == pPlayer)
            return true;

        if(!pPlayer->IsAlive())
            return true;

        if(pPlayer->GetTeamNumber() != 2)
            return true;

        if( m_bMustBeNotIncapacitated )
        {
            if( pPlayer->IsIncapacitated() )
            {
                return true;
            }
        }

        if( m_bMustBeNotCloseArea )
        {
            INavArea *pArea = (INavArea*)pPlayer->GetLastKnownArea();
            if( !pArea || access_member<int>(pArea, 305) < 0)
                return true;
        }

        float flDistSq = m_vOrigin.DistTo(pPlayer->WorldSpaceCenter());
        if(m_flClosestDistSq > flDistSq)
        {
            NextBotTraceFilterIgnoreActors filter(nullptr, COLLISION_GROUP_NONE);
            trace_t trace;
            util_TraceLine(m_vOrigin, pPlayer->WorldSpaceCenter(), (CONTENTS_SOLID|CONTENTS_BLOCKLOS|CONTENTS_IGNORE_NODRAW_OPAQUE|CONTENTS_MOVEABLE|CONTENTS_MONSTER), &filter, &trace);

            if(trace.fraction >= 1.f && !trace.startsolid)
            {
                m_flClosestDistSq = flDistSq;
                m_pClosest = pPlayer;
            }
        }

        return true;
    }

    inline ITerrorPlayer* GetClosest() const { return m_pClosest; }
private:                                    // vTable + 0
    Vector m_vOrigin;                       // + 4 8 12
    ITerrorPlayer* m_pClosest;              // + 16
    float m_flClosestDistSq;                // + 20
    IBaseEntity* m_pIgnore;                 // + 24
    bool m_bMustBeNotIncapacitated;         // + 28
    bool m_bMustBeNotCloseArea;             // + 29
};

ITerrorPlayer *IBossZombiePlayerBot::ChooseVictim(ITerrorPlayer *pVictim)
{
    ConVarRef nb_blind("nb_blind");
    extern ITerrorPlayer *GetClosestSurvivor(const Vector& pos, bool Incapaci = false, bool LastArea = false);

    if(nb_blind.GetBool() || access_member<bool>(this, 11192))
        return nullptr;

    CountdownTimers &m_victimChoiceTimer = access_member<CountdownTimers>(this, 11924);
    if(m_victimChoiceTimer.HasStarted() && !m_victimChoiceTimer.IsElapsed()) {
        return pVictim;
    }

    m_victimChoiceTimer.Start(0.3f);

    AllHumanSurvivorAreIncapacitatedScan IncapScan;
    ForEachTerrorPlayer(IncapScan);

    INextBot *pNextBot = MyNextBotPointer();
    ClosestVisibleSurvivorScan scan(pNextBot->GetPosition(), IncapScan.AllIncapacitated());
    ForEachTerrorPlayer(scan);
    ITerrorPlayer* pClosest = scan.GetClosest();

    if(!pClosest)
    {
        if((pClosest = GetClosestSurvivor(pNextBot->GetPosition())) == nullptr)
        {
            ITerrorPlayer* mClosesSurvivor = GetClosestSurvivor(pNextBot->GetPosition(), true);
            if(mClosesSurvivor && (mClosesSurvivor->IsBot() || !IncapScan.AllIncapacitated()))
            {
                pClosest = mClosesSurvivor;
            }
            else if(!pClosest)
            {
                return nullptr;
            }
        }
    }

    if(pNextBot->IsRangeGreaterThan((CBaseEntity*)pClosest, 500.f))
    {
        ITerrorPlayer* mClosesSurvivor = GetClosestSurvivor(pNextBot->GetPosition(), true);
        if(mClosesSurvivor && (mClosesSurvivor->IsBot() || !IncapScan.AllIncapacitated()))
        {
            pClosest = mClosesSurvivor;
        }
        else if(!pClosest)
        {
            return nullptr;
        }
    }
    
    if(!pVictim || !pVictim->IsAlive())
    {
        DevMsg("%3.2f: %s switching is not alive victim\n", g_pGlobals->curtime, GetDebugIdentifier());
        return pClosest;
    }

    CountdownTimers &m_timer8172 = access_member<CountdownTimers>(pVictim, 8172);
    if(!m_timer8172.IsElapsed() || pVictim == pClosest)
        return pVictim;

    if(pVictim->IsIncapacitated() && pClosest->IsPlayer())
    {
        if(!pClosest->IsIncapacitated())
        {
            DevMsg("%3.2f: %s switching to mobile victim\n", g_pGlobals->curtime, GetDebugIdentifier());
            return pClosest;
        }
    }

    float flDistSqClosest = GetAbsOrigin().DistTo(pClosest->GetAbsOrigin());
    float flDistSqVictim = GetAbsOrigin().DistTo(pVictim->GetAbsOrigin());
    IVision *pVision = pNextBot->GetVisionInterface();
    if((flDistSqVictim - flDistSqClosest) <= 200.f || !pVision->IsAbleToSee((CBaseEntity*)pClosest, IVision::DISREGARD_FOV))
    {
        if(pVision->IsAbleToSee((CBaseEntity*)pClosest, IVision::DISREGARD_FOV))
        {
            if(!pVision->IsAbleToSee((CBaseEntity*)pVictim, IVision::DISREGARD_FOV))
            {
                DevMsg("%3.2f: %s switching to directly visible victim\n", g_pGlobals->curtime, GetDebugIdentifier());
                return pClosest;
            }
        }

        return pVictim;
    }

    DevMsg("%3.2f: %s switching to closer victim\n", g_pGlobals->curtime, GetDebugIdentifier());
    return pClosest;
}

bool IBossZombiePlayerBot::IsBehind(ITerrorPlayer *pVictim)
{
    if(pVictim)
    {
        if(500.f + GetFlowDistance(0) < pVictim->GetFlowDistance(0))
        {
            return true;
        }
    }
    return false;
}
