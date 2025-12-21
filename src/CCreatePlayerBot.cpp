#include "CCreatePlayerBot.h"
#include "Interface/ITerrorPlayer.h"
#include "Interface/IWitch.h"
#include "Interface/INavArea.h"
#include "Interface/IGameRules.h"
#include "HL2.h"

enum TeamName {
    Team_Invalid = -1,
    Team_Unassigned,
    Team_Spectate,
    Team_Survivor,
    Team_Infected
};

CBotPlayerCreate g_BotPlayerCreate;
CBotPlayerCreate* g_pBotCreator = &g_BotPlayerCreate;

char *AllocateName = nullptr;
void *AllocatePlayerEntity(edict_t *pEdict, const char *name)
{
    char szNames[32 + 1];
    Q_strncpy(szNames, AllocateName, 32u);
    delete AllocateName;

    g_HL2->Set_BasePlayer_PlayerEdict(pEdict);
    return servertools->CreateEntityByName(szNames);
}

bool IsSpaceForZombieHere(const Vector vecArea)
{
    const CViewVectors *vecView = g_HL2->GetGameRules()->GetViewVectors();

    trace_t tr;
    CTraceFilterSimples traceFilter(nullptr, 0);
    util_TraceHull(vecArea, vecArea, vecView->m_vHullMin, vecView->m_vHullMax, 33702411, &traceFilter, &tr);

    if(!tr.startsolid)
    {
        return tr.fraction >= 1.0;
    }
    return true;
}

bool UTIL_IsSpaceEmpty( IBaseEntity *pMainEnt, const Vector &vMin, const Vector &vMax, unsigned int mask, ITraceFilter *pFilter )
{
	Vector vHalfDims = ( vMax - vMin ) * 0.5f;
	Vector vCenter = vMin + vHalfDims;

	trace_t trace;
	util_TraceHull( vCenter, vCenter, -vHalfDims, vHalfDims, mask, pFilter, &trace );

	bool bClear = ( trace.fraction == 1 && trace.allsolid != 1 && (trace.startsolid != 1) );
	return bClear;
}

Vector DropToGround( IBaseEntity *pMainEnt, const Vector &vPos, const Vector &vMins, const Vector &vMaxs )
{
    trace_t trace;
    CTraceFilterSimples filter(pMainEnt, COLLISION_GROUP_NONE);
    util_TraceHull( vPos, vPos + Vector( 0, 0, -500 ), vMins, vMaxs, MASK_SOLID, &filter, &trace );
    return trace.endpos;
}


bool EntityPlacementTest( IBaseEntity *pMainEnt, const Vector &vOrigin, Vector &outPos, bool bDropToGround, unsigned int mask, ITraceFilter *pFilter )
{
    const CViewVectors *vecView = g_HL2->GetGameRules()->GetViewVectors();
    CTraceFilterSimples defaultFilter( pMainEnt, COLLISION_GROUP_NONE );
    if ( !pFilter )
    {
        pFilter = &defaultFilter;
    }

    Vector mins, maxs;
    if ( pMainEnt )
    {
        pMainEnt->CollisionProp()->WorldSpaceAABB( &mins, &maxs );
        mins -= pMainEnt->GetAbsOrigin();
        maxs -= pMainEnt->GetAbsOrigin();
    }
    else
    {
        mins = vecView->m_vHullMin;
        maxs = vecView->m_vHullMax;
    }

    float flPadSize = 5;
    Vector vTestMins = mins - Vector( flPadSize, flPadSize, flPadSize );
    Vector vTestMaxs = maxs + Vector( flPadSize, flPadSize, flPadSize );

    if ( UTIL_IsSpaceEmpty( pMainEnt, vOrigin + vTestMins, vOrigin + vTestMaxs, mask, pFilter ) )
    {
        if ( bDropToGround )
        {
            outPos = DropToGround( pMainEnt, vOrigin, vTestMins, vTestMaxs );
        }
        else
        {
            outPos = vOrigin;
        }
        return true;
    }

    Vector vDims = vTestMaxs - vTestMins;

    int iCurIteration = 0;
    int nMaxIterations = 15;
    
    int offset = 0;
    do
    {
        for ( int iDim=0; iDim < 3; iDim++ )
        {
            float flCurOffset = offset * vDims[iDim];

            for ( int iSign=0; iSign < 2; iSign++ )
            {
                Vector vBase = vOrigin;
                vBase[iDim] += (iSign*2-1) * flCurOffset;
            
                if ( UTIL_IsSpaceEmpty( pMainEnt, vBase + vTestMins, vBase + vTestMaxs, mask, pFilter ) )
                {
                    trace_t tr;
                    util_TraceLine( vOrigin, vBase, mask, pFilter, &tr );

                    if ( tr.fraction != 1.0 )
                    {
                        continue;
                    }
                    
                    if ( bDropToGround )
                        outPos = DropToGround( pMainEnt, vBase, vTestMins, vTestMaxs );
                    else
                        outPos = vBase;

                    return true;
                }
            }
        }

        ++offset;
    } while ( iCurIteration++ < nMaxIterations );

    return false;
}


ITerrorPlayer *CBotPlayerCreate::SpawnTank(const Vector &vecOrigion, const QAngle &vecAngle)
{
    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Tank", "tank");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecOrigion);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassTank, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnTank(const INavArea *pNav, const QAngle &vecAngle)
{
    Vector vecSpawnPos;
    int i = 10;
    while( true )
    {
        vecSpawnPos = const_cast<INavArea*>(pNav)->FindRandomSpot();
        if(IsSpaceForZombieHere(vecSpawnPos)) {
            break;
        }

        Vector outPos;
        if(EntityPlacementTest(nullptr, vecSpawnPos, outPos, false, MASK_L4D_VISIBLE, nullptr)) {
            vecSpawnPos = outPos;
            break;
        }

        if(!--i)
            return nullptr;
    }

    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Tank", "tank");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecSpawnPos);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassTank, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnBoomer(const Vector &vecOrigion, const QAngle &vecAngle)
{
    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Boomer", "boomer");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecOrigion);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassBommer, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnBoomer(const INavArea *pNav, const QAngle &vecAngle)
{
    Vector vecSpawnPos;
    int i = 10;
    while( true )
    {
        vecSpawnPos = const_cast<INavArea*>(pNav)->FindRandomSpot();
        if(IsSpaceForZombieHere(vecSpawnPos)) {
            break;
        }

        Vector outPos;
        if(EntityPlacementTest(nullptr, vecSpawnPos, outPos, false, MASK_L4D_VISIBLE, nullptr)) {
            vecSpawnPos = outPos;
            break;
        }

        if(!--i)
            return nullptr;
    }

    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Boomer", "boomer");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecSpawnPos);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassBommer, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnSmoker(const Vector &vecOrigion, const QAngle &vecAngle)
{
    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Smoker", "smoker");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecOrigion);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassSmoker, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnSmoker(const INavArea *pNav, const QAngle &vecAngle)
{
    Vector vecSpawnPos;
    int i = 10;
    while( true )
    {
        vecSpawnPos = const_cast<INavArea*>(pNav)->FindRandomSpot();
        if(IsSpaceForZombieHere(vecSpawnPos)) {
            break;
        }

        Vector outPos;
        if(EntityPlacementTest(nullptr, vecSpawnPos, outPos, false, MASK_L4D_VISIBLE, nullptr)) {
            vecSpawnPos = outPos;
            break;
        }

        if(!--i)
            return nullptr;
    }

    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Smoker", "smoker");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecSpawnPos);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassSmoker, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnHunter(const Vector &vecOrigion, const QAngle &vecAngle)
{
    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Hunter", "hunter");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecOrigion);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassHunter, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

ITerrorPlayer *CBotPlayerCreate::SpawnHunter(const INavArea *pNav, const QAngle &vecAngle)
{
    Vector vecSpawnPos;
    int i = 10;
    while( true )
    {
        vecSpawnPos = const_cast<INavArea*>(pNav)->FindRandomSpot();
        if(IsSpaceForZombieHere(vecSpawnPos)) {
            break;
        }

        Vector outPos;
        if(EntityPlacementTest(nullptr, vecSpawnPos, outPos, false, MASK_L4D_VISIBLE, nullptr)) {
            vecSpawnPos = outPos;
            break;
        }

        if(!--i)
            return nullptr;
    }

    g_HL2->SetManualSpawn(true);
    ITerrorPlayer *pBot = NextBotCreatePlayerBot("Hunter", "hunter");
    if(pBot)
    {
        pBot->HandleCommand_JoinTeam(Team_Infected, NULL, false);
        pBot->State_Transition(0);
        pBot->SetAbsOrigin(vecSpawnPos);
        pBot->SetAbsAngles(vecAngle);
        g_HL2->SelectedZombieSpawn(pBot);
        pBot->Spawn();
        g_HL2->SelectedZombieSpawn(nullptr);
        Event_SpawnZombie(ZombieClassHunter, 1);

        g_HL2->SetManualSpawn(false);
        return pBot;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

IWitch *CBotPlayerCreate::SpawnWitch(const Vector &vecOrigion, const QAngle &vecAngle)
{
    g_HL2->SetManualSpawn(true);
    IWitch *pWitch = (IWitch*)servertools->CreateEntityByName("witch");
    if(pWitch)
    {
        pWitch->SetAbsOrigin(vecOrigion);
        pWitch->SetAbsAngles(vecAngle);
        servertools->DispatchSpawn(pWitch);
        Event_SpawnZombie(ZombieClassWitch, 1);

        g_HL2->SetManualSpawn(false);
        return pWitch;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

IWitch *CBotPlayerCreate::SpawnWitch(const INavArea *pNav, const QAngle &vecAngle)
{
    Vector vecSpawnPos;
    int i = 10;
    while( true ) {
        vecSpawnPos = const_cast<INavArea*>(pNav)->FindRandomSpot();
        if(IsSpaceForZombieHere(vecSpawnPos)) {
            break;
        }

        Vector outPos;
        if(EntityPlacementTest(nullptr, vecSpawnPos, outPos, false, MASK_L4D_VISIBLE, nullptr)) {
            vecSpawnPos = outPos;
            break;
        }

        if(!--i)
            return nullptr;
    }

    g_HL2->SetManualSpawn(true);
    IWitch *pWitch = (IWitch*)servertools->CreateEntityByName("witch");
    if(pWitch)
    {
        pWitch->SetAbsOrigin(vecSpawnPos);
        pWitch->SetAbsAngles(vecAngle);
        servertools->DispatchSpawn(pWitch);
        Event_SpawnZombie(ZombieClassWitch, 1);

        g_HL2->SetManualSpawn(false);
        return pWitch;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

IInfected *CBotPlayerCreate::SpawnCommon(const Vector &vecOrigion, const QAngle &vecAngle)
{
    g_HL2->SetManualSpawn(true);
    IInfected *pCommon = (IInfected*)servertools->CreateEntityByName("infected");
    if(pCommon)
    {
        pCommon->SetAbsOrigin(vecOrigion);
        pCommon->SetAbsAngles(vecAngle);
        servertools->DispatchSpawn(pCommon);
        pCommon->Update();
        pCommon->AttackSurvivorTeam();        
        Event_SpawnZombie(ZombieClassCommon, 1);

        g_HL2->SetManualSpawn(false);
        return pCommon;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

IInfected *CBotPlayerCreate::SpawnCommon(const INavArea *pNav, const QAngle &vecAngle)
{
    Vector vecSpawnPos;
    int i = 10;
    while( true )
    {
        vecSpawnPos = const_cast<INavArea*>(pNav)->FindRandomSpot();
        if(IsSpaceForZombieHere(vecSpawnPos)) {
            break;
        }

        Vector outPos;
        if(EntityPlacementTest(nullptr, vecSpawnPos, outPos, false, MASK_L4D_VISIBLE, nullptr)) {
            vecSpawnPos = outPos;
            break;
        }

        if(!--i)
            return nullptr;
    }

    g_HL2->SetManualSpawn(true);
    IInfected *pCommon = (IInfected*)servertools->CreateEntityByName("infected");
    if(pCommon)
    {
        pCommon->SetAbsOrigin(vecSpawnPos);
        pCommon->SetAbsAngles(vecAngle);
        servertools->DispatchSpawn(pCommon);
        pCommon->Update();
        pCommon->AttackSurvivorTeam();        
        Event_SpawnZombie(ZombieClassCommon, 1);

        g_HL2->SetManualSpawn(false);
        return pCommon;
    }

    g_HL2->SetManualSpawn(false);
    return nullptr;
}

const char *SurvivorCharacterDisplayName(int nCharacter)
{
    switch (nCharacter)
    {
    case 0:
        return "Bill";
    case 1:
        return "Zoey";
    case 2:
        return "Francis";
    case 3:
        return "Louis";
    }
    return "Survivor";
}

ITerrorPlayer *CBotPlayerCreate::SpawnSurvivor()
{
    int iRandCharact = RandomInt(0, 3);
    const char* szCharacterName = SurvivorCharacterDisplayName(iRandCharact);
    ITerrorPlayer* pSurvivorBot = NextBotCreatePlayerBot(szCharacterName, "survivor_bot");
    if(pSurvivorBot)
    {
        pSurvivorBot->HandleCommand_JoinTeam(Team_Survivor, szCharacterName, false);
        return nullptr;
    }

    return pSurvivorBot;
}

ITerrorPlayer *CBotPlayerCreate::NextBotCreatePlayerBot(const char *szClassName, const char *szName)
{
    AllocateName = new char[32+1];
    Q_strncpy(AllocateName, szName, 32u);

    g_HL2->ClientPutInServerOverride(AllocatePlayerEntity);
    edict_t* pEdict = engine->CreateFakeClient(szClassName);
    g_HL2->ClientPutInServerOverride(NULL);

    ITerrorPlayer *pEntity = EdictToTerrorPlayer(pEdict);
    if(!pEntity) {
        return nullptr;
    }
    
    pEntity->pl.netname = g_HL2->AllocPooledString(szClassName);
    pEntity->ClearFlags();
    pEntity->AddFlag(FL_CLIENT|FL_FAKECLIENT);

    return pEntity;
}

void CBotPlayerCreate::Event_SpawnZombie(int nType, int nVal)
{
    if(nType <= 5)
    {
        int pThis = (int)g_HL2->GetGameStats();
        DWORD result = pThis + 4 * nType;
        *(DWORD*)(result + 840) += nVal;
    }
}

ITerrorPlayer *CBotPlayerCreate::EdictToTerrorPlayer(edict_t *pEdict)
{
    if(!pEdict) {
        return nullptr;
    }

    IServerUnknown* pUnk = pEdict->GetUnknown();
    if(!pUnk) {
        return nullptr;
    }

    return reinterpret_cast<ITerrorPlayer*>(pUnk->GetBaseEntity());
}
