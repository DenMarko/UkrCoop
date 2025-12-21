#include "MyTank.h"
#include "Interface/ICheckpoint.h"

NOINLINE ICheckpoint *GetInitialCheckpoint()
{
    INavMesh *pTheNavMesh = g_HL2->GetTheNavMesh();
    if(!pTheNavMesh)
        return nullptr;

    if(g_CallHelper->Director_IsTransitioned())
    {
        char *checkpointName = g_CallHelper->GetTransitionedLandmarkName();
        if(checkpointName && checkpointName[0] != '\0')
        {
            IBaseEntity *pEntity = g_CallHelper->FindEntityByName(nullptr, checkpointName);
            if(pEntity != nullptr)
            {
                Vector vecWorldCenter = pEntity->WorldSpaceCenter();
                CUtlVector<ICheckpoint *> &stack = access_member< CUtlVector<ICheckpoint *> >(pTheNavMesh, 389 * 4);
                ICheckpoint *closeArea = nullptr;
                float closeDistSq = 0.0f;
                bool firstArea = true;

                for (int i = 0; i < stack.Count(); ++i)
                {
                    ICheckpoint *m_checkPoint = stack[i];
                    Vector areaCorner = (m_checkPoint->GetMinCorner() + m_checkPoint->GetMaxCorner()) * 0.5f;
                    float distSq = areaCorner.DistToSqr(vecWorldCenter);

                    if(firstArea)
                    {
                        closeArea = m_checkPoint;
                        closeDistSq = distSq;
                        firstArea = false;
                    }
                    else if(distSq < closeDistSq)
                    {
                        if(!m_checkPoint->IsOutward())
                        {
                            closeArea = m_checkPoint;
                            closeDistSq = distSq;
                        }
                    }
                }
                return closeArea;
            }
        }
    }

    return nullptr;
}

NOINLINE ICheckpoint *GetLastCheckpoint()
{
    INavMesh *pTheNavMesh = g_HL2->GetTheNavMesh();
    if(!pTheNavMesh)
        return nullptr;

    CUtlVector<ICheckpoint*> &stack = access_member< CUtlVector<ICheckpoint*> >(pTheNavMesh, 389 * 4);
    for (int i = 0; i < stack.Count(); ++i)
    {
        ICheckpoint *m_area = stack[i];
        if(m_area->IsOutward())
        {
            return m_area;
        }
    }
    return nullptr;
}

class HighestFlowDistance
{
private:
    float m_flBestDistance;             // +4  - максимальна знайдена відстань
    int m_nTargetTeam;                  // +8  - номер команди для фільтрації
    int m_nPlayerCount;                 // +12  - лічильник знайдених гравців
    CHandle<ITerrorPlayer> m_pPlayer;   // +16 - гравець з найбільшою відстанню
    int m_nFlags;                       // +20 - прапорці фільтрації
    FlowType m_nFlowType;               // +24 - тип flow distance для порівняння

    ConVarRef survivor_skills;
public:
    HighestFlowDistance(int targetTeam, int flags, FlowType flowType)
        : m_flBestDistance(-9999.0f), m_nTargetTeam(targetTeam), m_nPlayerCount(0), m_nFlags(flags), m_nFlowType(flowType), survivor_skills("survivor_skills")
    {
        m_pPlayer.Term();
    }

    virtual bool operator()(ITerrorPlayer* client)
    {
        if(!client->IsPlayer())
            return true;

        if(!client->IsConnected())
            return true;

        if(!client->IsAlive())
            return true;

        if(client->GetTeamNumber() != m_nTargetTeam)
            return true;

        if((m_nFlags & 1) != 0)
        {
            if(client->IsIncapacitated() )
            {
                if(!survivor_skills.GetInt() || access_member<int>(client, 2768*4) != 4)
                    return true;
            }
        }

        if((m_nFlags & 2) != 0)
        {
            if(!client->IsIncapacitated())
                return true;
        }
        
        INavArea *pArea = (INavArea*)client->GetLastKnownArea();
        if(!pArea || access_member<float>(pArea, 348) <= -9999.0f)
            return true;

        if((m_nFlags & 4) != 0)
        {
            if(GetInitialCheckpoint()->ContainsArea(pArea))
                return true;
        }

        if((m_nFlags & 8) != 0)
        {
            if(GetLastCheckpoint()->ContainsArea(pArea))
                return true;
        }

        float flDistance = access_array_member<float>(pArea, 348, m_nFlowType);
        if(m_pPlayer != NULL)
        {
            if(flDistance <= m_flBestDistance)
                return true;
        }

        m_pPlayer = client->GetRefEHandle();
        m_flBestDistance = flDistance;
        m_nPlayerCount++;
        return true;
    }

    ITerrorPlayer* GetPlayer() const { return m_pPlayer; }
};

ITerrorPlayer* GetHighestFlowSurvivor(FlowType FlowType)
{
    HighestFlowDistance finder(2, 0, FlowType);
    ForEachTerrorPlayer(finder);
    return finder.GetPlayer();
}

TankIdle::TankIdle()
{
}

TankIdle::~TankIdle()
{
}

ActionResult<ITank> TankIdle::Update(ITank *me, float interval)
{
    if(g_CallHelper->Director_IsVisibleToTeam(me, 2, 7))
    {
        return ChangeTo(new TankAttack(), "We see a victim!");
    }

    if(me->IsBehind(GetHighestFlowSurvivor(TOWARD_GOAL)))
    {
        return ChangeTo(new TankAttack(), "The Survivors have passed me by!");
    }

    return Continue();
}

ActionResult<ITank> TankIdle::OnStart(ITank *me, Action<ITank> *priorAction)
{
    me->GetBodyInterface()->SetArousal(IBody::NEUTRAL);
    me->GetBodyInterface()->SetDesiredPosture(IBody::STAND);

    return Continue();
}

EventDesiredResult<ITank> TankIdle::OnCommandAttack(ITank *me, CBaseEntity *victim)
{
    return TryContinue();
}

EventDesiredResult<ITank> TankIdle::OnCommandApproach(ITank *me, const Vector &pos, float maxDistance)
{
    return TrySuspendFor(new BehaviorMoveTo<ITank, InfectedPathCost>(pos), RESULT_CRITICAL, "Debug move to");
}

EventDesiredResult<ITank> TankIdle::OnInjured(ITank *me, const CTakeDamageInfo &info)
{
    if((info.GetDamageType() & DMG_BURN) != 0)
    {
        if(info.GetAttacker() == nullptr)
        {
            return TryToSustain(RESULT_TRY);
        }
        
        IBaseEntity* pEntity = (IBaseEntity*)info.GetAttacker();
        if(pEntity->IsPlayer())
        {
            CZombieIgnite event;
            event.Set(engine->GetPlayerUserId(pEntity->edict()), pEntity->entindex(), "Tank");
        }
    }

    if(info.GetAttacker() == nullptr || reinterpret_cast<IBaseEntity*>(info.GetAttacker())->GetTeamNumber() != 2)
        return TryToSustain(RESULT_TRY);

    return TryChangeTo(new TankAttack(), RESULT_CRITICAL, "Ouch!");
}

EventDesiredResult<ITank> TankIdle::OnShoved(ITank *me, CBaseEntity *pusher)
{
    return TryChangeTo(new TankAttack(), RESULT_CRITICAL, "Don't push me!");
}

EventDesiredResult<ITank> TankIdle::OnContact(ITank *me, CBaseEntity *other, CGameTrace *result)
{
    IBaseEntity* pEntity = GetVirtualClass<IBaseEntity>(other);
    if(!pEntity || !pEntity->IsPlayer() || pEntity->GetTeamNumber() != 2)
    {
        return TryToSustain(RESULT_TRY);
    }

    return TryChangeTo(new TankAttack(), RESULT_CRITICAL, "Don't touch me!");
}
