#include "HL2.h"
#include "CLuaPropProxy.h"
#include "ConVar_l4d.h"
#include "LuaBridge/LuaBridge.h"
#include "CUserMessage.h"

#include "Interface/ITerrorPlayer.h"
#include "Interface/IGameRules.h"
#include "Interface/IBaseCombatWeapon.h"
#include "Interface/ITeam.h"

#include <ispatialpartition.h>

using namespace VCaller;

ICallHellpers*   g_CallHelper = nullptr;

CTakeDamageInfo::CTakeDamageInfo()
{}

CTakeDamageInfoHack::CTakeDamageInfoHack(CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType, const Vector &vecDamagPos, const Vector &vecDamagForce)
{
    Init(pInflictor, pAttacker, flDamage, bitsDamageType, vecDamagPos, vecDamagForce);
}

CTakeDamageInfoHack::CTakeDamageInfoHack(IBaseEntity *pInflictor, IBaseEntity *pAttacker, float flDamage, int bitsDamageType, const Vector &vecDamagPos, const Vector &vecDamagForce)
{
    Init((CBaseEntity*)pInflictor, (CBaseEntity*)pAttacker, flDamage, bitsDamageType, vecDamagPos, vecDamagForce);
}

CTakeDamageInfoHack::CTakeDamageInfoHack()
{
    m_hInflictor = nullptr;
    m_hAttacker = nullptr;
    m_flDamage = 0.f;
    m_flBaseDamage = BASEDAMAGE_NOT_SPECIFIED;
    m_bitsDamageType = 0;
    m_flMaxDamage = 0.f;
    m_vecDamageForce = vec3_origin;
    m_vecDamagePosition = vec3_origin;
    m_vecReportedPosition = vec3_origin;
    m_iAmmoType = -1;
    m_iDamageCustom = 0;
}

void CTakeDamageInfoHack::Init(CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType, const Vector &vecDamagPos, const Vector &vecDamagForce)
{
    m_hInflictor = pInflictor;
    if ( pAttacker )
    {
        m_hAttacker = pAttacker;
    }
    else
    {
        m_hAttacker = pInflictor;
    }

    m_flDamage = flDamage;
    m_flBaseDamage = BASEDAMAGE_NOT_SPECIFIED;
    m_bitsDamageType = bitsDamageType;
    m_flMaxDamage = flDamage;
    m_vecDamageForce = vecDamagForce;
    m_vecDamagePosition = vecDamagPos;
    m_vecReportedPosition = vec3_origin;
    m_iAmmoType = -1;
    m_iDamageCustom = 0;
}

CBaseEntity *HL2::GetBaseEntity(CBaseHandle handle)
{
    IServerUnknown *pUnk = static_cast<IServerUnknown *>(g_pEntityList->LookupEntity(handle));
    if(pUnk)
        return pUnk->GetBaseEntity();
    else
        return nullptr;
}

class CResponseQueue;

class CResponseQueueManager : public CAutoGameSystemPerFrame
{
public:
	CResponseQueueManager(char const *name) : CAutoGameSystemPerFrame( name )
	{
		m_pQueue = NULL;
	}
	virtual ~CResponseQueueManager(void) = 0;
	virtual void Shutdown() = 0;
	virtual void FrameUpdatePostEntityThink( void ) = 0;
	virtual void LevelInitPreEntity( void ) = 0;

	inline CResponseQueue *GetQueue(void) { Assert(m_pQueue); return m_pQueue; }

protected:
	CResponseQueue *m_pQueue;
};


HL2::HL2() : 
    g_pMasterMarker(nullptr), 
    g_pNextID(nullptr),
    g_pHidingSpotmasterMarker(nullptr),
    g_pOpenList(nullptr),
    g_pOpenListTail(nullptr),
    g_pClientPutInServerOverride(nullptr),
    g_pBasePlayer_PlayerEdict(nullptr),
    g_pSelectedZombieSpawn(nullptr),
    g_pGameStats(nullptr),
    g_pManualSpawn(nullptr),
    g_pSerchMarker(nullptr),
    g_pDirector(nullptr), 
    g_pZombiMeneger(nullptr), 
    g_pCollisionPaintHash(nullptr), 
    g_pPhysicsProps(nullptr), 
    g_pTeamGlobal(nullptr), 
    g_pPhysicsCollision(nullptr), 
    g_pDirtyKDTree(nullptr)
{
    lua_createtable(g_Sample.GetLuaState(), 0, 4);
    lua_pushinteger(g_Sample.GetLuaState(), DEST::HINTTEXT);
    lua_setfield(g_Sample.GetLuaState(), -2, "HINTTEXT");
    lua_pushinteger(g_Sample.GetLuaState(), DEST::CONSOLE);
    lua_setfield(g_Sample.GetLuaState(), -2, "CONSOLE");
    lua_pushinteger(g_Sample.GetLuaState(), DEST::CHAT);
    lua_setfield(g_Sample.GetLuaState(), -2, "CHAT");
    lua_pushinteger(g_Sample.GetLuaState(), DEST::CENTER);
    lua_setfield(g_Sample.GetLuaState(), -2, "CENTER");
    lua_setglobal(g_Sample.GetLuaState(), "DEST");

    lua_createtable(g_Sample.GetLuaState(), 0, 2);
    lua_pushinteger(g_Sample.GetLuaState(), Prop_Sends);
    lua_setfield(g_Sample.GetLuaState(), -2, "PropSend");
    lua_pushinteger(g_Sample.GetLuaState(), Prop_Datas);
    lua_setfield(g_Sample.GetLuaState(), -2, "PropData");
    lua_setglobal(g_Sample.GetLuaState(), "PropType");

    lua_createtable(g_Sample.GetLuaState(), 0, 8);
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_Int);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_Int");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_Float);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_Float");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_Vector);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_Vector");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_VectorXY);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_VectorXY");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_String);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_String");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_Array);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_Array");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_DataTable);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_DataTable");
    lua_pushinteger(g_Sample.GetLuaState(), SendPropType::DPT_NUMSendPropTypes);
    lua_setfield(g_Sample.GetLuaState(), -2, "DPT_NUMSendPropTypes");
    lua_setglobal(g_Sample.GetLuaState(), "SendPropType");

    luabridge::getGlobalNamespace(g_Sample.GetLuaState())
        .beginClass<CTakeDamageInfoHack>("TakeDamageInfo")
            .addConstructor<void(*)(CBaseEntity *, CBaseEntity *, float, int)>()
            .addProperty("Attacker",  std::function<const CBaseEntity* (const CTakeDamageInfoHack*)>([](const CTakeDamageInfoHack* pThisPtr)
            {
                int nAttack = pThisPtr->GetAttacker();
                edict_t *pEdict = g_Sample.PEntityOfEntIndex(nAttack);
                if(pEdict)
                    return pEdict->GetUnknown()->GetBaseEntity();

                return (CBaseEntity*)nullptr;
            }),
            std::function<void(CTakeDamageInfoHack*, CBaseEntity*)>([](CTakeDamageInfoHack* pThisPtr, CBaseEntity* attacer)
            {
                pThisPtr->SetAttacker(attacer);
            }))
            .addProperty("Inflictor", std::function<const CBaseEntity* (const CTakeDamageInfoHack*)>([](const CTakeDamageInfoHack* pThisPtr)
            {
                int nInflict = pThisPtr->GetInflictor();
                edict_t *pEdict = g_Sample.PEntityOfEntIndex(nInflict);
                if(pEdict)
                    return pEdict->GetUnknown()->GetBaseEntity();

                return (CBaseEntity*)nullptr;
            }),
            std::function<void(CTakeDamageInfoHack*, CBaseEntity*)>([](CTakeDamageInfoHack* pThisPtr, CBaseEntity* Inflictor)
            {
                pThisPtr->SetInflictor(Inflictor);
            }))
            .addProperty("Damage", std::function<float(const CTakeDamageInfoHack*)>([](const CTakeDamageInfoHack* pThisPtr)
            {
                return pThisPtr->GetDamage();
            }),
            std::function<void(CTakeDamageInfoHack*, float)>([](CTakeDamageInfoHack* pThisPtr, float damage)
            {
                pThisPtr->SetDamage(damage);
            }))
            .addProperty("DamageType", std::function<int(const CTakeDamageInfoHack*)>([](const CTakeDamageInfoHack *pThisPtr)
            {
                return pThisPtr->GetDamageType();
            }),
            std::function<void(CTakeDamageInfoHack*, int)>([](CTakeDamageInfoHack* pThisPtr, int type)
            {
                pThisPtr->SetDamageType(type);
            }))
        .endClass()
        .beginClass<HL2>("GameSDK")
            .addFunction("TextMsg", std::function<bool(HL2 *, int, int, const char *)>([](HL2 *p, int client, int dest, const char *msg)
            {
                if(client > 0 && client <= MAX_PLAYERS)
                {
                    return p->TextMsg(client, static_cast<DEST>(dest), msg);
                }
                m_sLog->LogToFileEx(false, "[LUA] ERROR TextMsg client index is not valid (%d)", client);
                return false;
            }))
            .addFunction("GetReserveLobby", &HL2::GetReserveLobby)
            .addFunction("UnReserveLobby", &HL2::UnReserveLobby)
            .addFunction("SetEntProp", std::function<int(HL2*, int32_t, int, const char*, int)>([](HL2* p, int32_t ent, int proptype, const char* prop, int value)
            {
                return p->SetEntProp(ent, static_cast<Prop_Types>(proptype), prop, value);
            }))
            .addFunction("GetEntProp", std::function<int(HL2*, int32_t, int, const char*)>([](HL2* p, int32_t entity, int type, const char *prop)
            {
                return p->GetEntProp(entity, static_cast<Prop_Types>(type), prop);
            }))
            .addFunction("Translate", std::function<bool(HL2 *, const char*, unsigned int, unsigned int, const char*, int)>([](HL2 *p, const char* buffer, unsigned int len, unsigned int param, const char* text, int client)
            {
                return p->Translate((char *)buffer, len, "%T", param, NULL, text, client);
            }))
            .addFunction("PrecacheGibsForModels", &HL2::PrecacheGibsForModel)
        .endClass()
        .beginClass<variant_t>("variant")
            .addConstructor<void(*)()>()
            .addFunction("SetBool", &variant_t::SetBool)
            .addFunction("SetString", std::function<void(variant_t *, const char*)>([](variant_t* pThisPtr, const char *str) { castable_string_t mStr(str); pThisPtr->SetString(mStr); }))
            .addFunction("SetInt", &variant_t::SetInt)
            .addFunction("SetFloat", &variant_t::SetFloat)
            .addFunction("SetEntity", &variant_t::SetEntity)
            .addFunction("SetVector3D", &variant_t::SetVector3D)
            .addFunction("SetPositionVector3D", &variant_t::SetPositionVector3D)
            .addFunction("SetColor", std::function<void(variant_t *, int, int, int, int)>([](variant_t* pThisPtr, int r, int g, int b, int a) { pThisPtr->SetColor32(r, g, b, a); }))
        .endClass();
    luabridge::setGlobal(g_Sample.GetLuaState(), this, "GameSDK");

    g_CallHelper = this;
}

HL2::~HL2()
{
    s_Stargged.Shutdown();
    s_TakeOver.Shutdown();
    s_HumenSpec.Shutdown();
    s_UnReserver.Shutdown();
    s_Reserver.Shutdown();
    s_VomitUpon.Shutdown();
    s_IsLiveSurvivorInside.Shutdown();
    s_HasAnySurvivorLeftSafeArea.Shutdown();
    s_OnMobRash.Shutdown();
    s_SpawnITMob.Shutdown();
    s_SetOrigin.Shutdown();
    s_GetSequenceMoveYam.Shutdown();
    s_AddStepDiscon.Shutdown();
    s_FindEntityByName.Shutdown();
    s_PhysicsTouchTriger.Shutdown();
    s_NPCPhysics_CreateSolver.Shutdown();
    s_LockStudioHdr.Shutdown();
    s_LookupPoseParam.Shutdown();
    s_HasPoseParam.Shutdown();
    s_SetSequence.Shutdown();
    s_SetPoseParam.Shutdown();
    s_UTIL_Remove.Shutdown();
    s_GetAmmoDef.Shutdown();
    s_PhysIsInCallback.Shutdown();
    s_PhysCallbackDamage.Shutdown();
    s_ApplyMultiDamag.Shutdown();
    s_ClearMultiDamag.Shutdown();
    s_GetWorldEntity.Shutdown();
    s_OnGSClientApprove.Shutdown();
    s_PrecacheGibsForModel.Shutdown();
    s_CreateDataObj.Shutdown();
    s_DestroyDataObj.Shutdown();
    s_SimThinkEntityChanged.Shutdown();
    s_EntityTouchAdd.Shutdown();
    s_EventListIndexForName.Shutdown();
    s_EventListRegisterPrivateEvent.Shutdown();
    s_EventListGetEventType.Shutdown();
    s_GetDataObject.Shutdown();
    s_AddEntityToGroundList.Shutdown();
    s_PhysicsRemoveGround.Shutdown();
    s_ReportEntityFlagsChanged.Shutdown();
    s_ReportPositionChanged.Shutdown();
    s_TransitionPlayerCount.Shutdown();
    s_TheNextBot.Shutdown();
    s_Select_Weighted_Sequence.Shutdown();
    s_CBaseEntity9EmitSound1.Shutdown();
    s_CBaseEntity9EmitSound2.Shutdown();
    s_CSoundEntInsertSound.Shutdown();
    s_CResponseQueueAdd.Shutdown();
    s_IsBreakableEntity.Shutdown();
    s_DirectorIsVisibleToTeam.Shutdown();
    s_SetDamagedBodyGroupVariant.Shutdown();
    s_State_Transition.Shutdown();
    s_GetFileWeaponInfoFromHandlet.Shutdown();
    s_MoveHelperServerv.Shutdown();
    s_PhysModelCreate.Shutdown();
    s_IsLineOfSightBetweenTwoEntitiesClear.Shutdown();
    s_Director_IsTransitioned.Shutdown();
    s_GetTransitionedLandmarkName.Shutdown();
    s_IsMotionControlledXY.Shutdown();
    s_IsMotionControlledZ.Shutdown();
    s_ActivityList_IndexForName.Shutdown();
    s_RegisterPrivateActivity.Shutdown();
    s_CEventQueue_AddEvent.Shutdown();
}

bool HL2::TextMsg(int client, DEST dest, const char *msg)
{
    if(client > 0 && client <= g_pGlobals->maxClients)
    {
        cell_t players[] = {client};
        CUserRecipientFilter user(players, 1);
        user.MakeReliable();

        if(dest == DEST::HINTTEXT)
        {
            CUserMessage *gHintText = nullptr;
            if((gHintText = new CUserMessage(user, "HintText")) != nullptr)
            {
                gHintText->MsgWriteString(msg);
                delete gHintText;
            }
        }
        else
        {
            CUserMessage *gTextMsg = nullptr;
            if((gTextMsg = new CUserMessage(user, "TextMsg")) != nullptr)
            {
                gTextMsg->MsgWriteByte(static_cast<int>(dest));
                gTextMsg->MsgWriteString(msg);
                delete gTextMsg;
            }
        }
        return true;
    }
    return false;
}

void HL2::OnSpawnITMob(int mobSize)
{
    if(SetupSpawnITMob())
    {
        ArgcBuffer<void *, int> pParam(g_pZombiMeneger, mobSize);
        s_SpawnITMob->Execute(pParam, nullptr);
    }
}

IPhysicsObjectPairHash *HL2::GetCollisionHash()
{
    if(g_pCollisionPaintHash == nullptr)
    {
        g_pGameConf->GetAddress("CollisionHash", &g_pCollisionPaintHash);
    }

    return reinterpret_cast<IPhysicsObjectPairHash*>(g_pCollisionPaintHash);
}

IPhysicsSurfaceProps *HL2::GetSurfaceProps()
{
    if(!g_pPhysicsProps)
    {
        g_pGameConf->GetAddress("IPhysicsProps", &g_pPhysicsProps);
    }

    return (IPhysicsSurfaceProps *)g_pPhysicsProps;
}

IPhysicsCollision *HL2::GetPhysicsCollision()
{
    if(!g_pPhysicsCollision)
    {
        g_pGameConf->GetAddress("PtrPhyscollision", &g_pPhysicsCollision);
    }

    return (IPhysicsCollision *)g_pPhysicsCollision;
}

unsigned int *HL2::GetNavArea_masterMarker()
{
    if(!g_pMasterMarker)
    {
        g_pGameConf->GetAddress("CNavArea_masterMarker", &g_pMasterMarker);
    }
#ifdef _DEBUG
    {
        void *addr1 = nullptr, *addr2 = nullptr;
        g_pGameConf->GetMemSig("TerrorNavArea_m_masterMarker", &addr1);
        g_pGameConf->GetMemSig("CNavArea_m_masterMarker", &addr2);

        Msg("TerrorNavArea_m_masterMarker 0x%x(%d)\nCNavArea_m_masterMarker 0x%x(%d)\n", addr1, *reinterpret_cast<unsigned int*>(addr1), addr2, *reinterpret_cast<unsigned int*>(addr2));
    }
#endif

    if(!g_pMasterMarker)
        Msg("[GetNavArea_masterMarker] Wraning m_masterMarker is nullptr\n");

    return reinterpret_cast<unsigned int*>(g_pMasterMarker);
}

void *HL2::GetNavArea_openList()
{
    if(g_pOpenList)
    {
        return g_pOpenList;
    }

#ifdef _DEBUG
    {
        void *addr = nullptr;
        g_pGameConf->GetMemSig("CNavArea_m_openList", &addr);

        Msg("CNavArea_m_openList 0x%x(0x%x)\n", addr, *reinterpret_cast<void**>(addr));
    }
#endif

    if(g_pGameConf->GetAddress("CNavArea_openList", &g_pOpenList) && g_pOpenList != nullptr)
    {
        return g_pOpenList;
    }
    return nullptr;
}

void *HL2::GetNavArea_openListTail()
{
    if(g_pOpenListTail) return g_pOpenListTail;

    if( g_pGameConf->GetAddress("CNavArea_openListTail", &g_pOpenListTail) && g_pOpenListTail != nullptr)
    {
        return g_pOpenListTail;
    }
    return nullptr;
}

void HL2::ClientPutInServerOverride( ClientPutInServerOverrideFn fn )
{
    if(g_pClientPutInServerOverride)
    {
        (*(void**)g_pClientPutInServerOverride) = (void*)fn;
        return;
    }

    if(g_pGameConf->GetAddress("pClientPutInServerOverride", &g_pClientPutInServerOverride) && g_pClientPutInServerOverride != nullptr)
    {
        (*(void**)g_pClientPutInServerOverride) = (void*)fn;
        return;
    }

    return;
}

void HL2::Set_BasePlayer_PlayerEdict( edict_t *pEdict )
{
    if(g_pBasePlayer_PlayerEdict)
    {
        *(void**)g_pBasePlayer_PlayerEdict = pEdict;
        return;
    }

    if(g_pGameConf->GetAddress("BasePlayer__PlayerEdict", &g_pBasePlayer_PlayerEdict) && g_pBasePlayer_PlayerEdict != nullptr)
    {
        *(void**)g_pBasePlayer_PlayerEdict = pEdict;
        return;
    }

    return;
}

void HL2::SetManualSpawn(bool IsManual)
{
    if(g_pManualSpawn)
    {
        (*reinterpret_cast<int*>(g_pManualSpawn)) = IsManual ? 1 : 0;
        return;
    }

    if(g_pGameConf->GetAddress("ManualSpawn", &g_pManualSpawn) && g_pManualSpawn != nullptr)
    {
        (*reinterpret_cast<int*>(g_pManualSpawn)) = IsManual ? 1 : 0;
        return;
    }
}

void HL2::SelectedZombieSpawn(void *ptr)
{
    if(g_pSelectedZombieSpawn)
    {
        *(void**)g_pSelectedZombieSpawn = ptr;
        return;
    }

    if(g_pGameConf->GetAddress("SelectedZombieSpawn", &g_pSelectedZombieSpawn) && g_pSelectedZombieSpawn != nullptr)
    {
        *(void**)g_pSelectedZombieSpawn = ptr;
        return;
    }
    return;
}

void *HL2::GetGameStats()
{
    if(g_pGameStats)
    {
        return g_pGameStats;
    }

    if(g_pGameConf->GetAddress("GameStats", &g_pGameStats) && g_pGameStats != nullptr)
    {
        return g_pGameStats;
    }
    return nullptr;
}

void *HL2::CEventQueue()
{
    if(g_pEventQueue) return g_pEventQueue;

    if(g_pGameConf->GetAddress("CEventQueue_ptr", &g_pEventQueue) && g_pEventQueue != nullptr) return g_pEventQueue;

    return nullptr;
}

unsigned int *HL2::GetHidingSpot_nextID()
{
    if(g_pNextID)
    {
        return reinterpret_cast<unsigned int*>(g_pNextID);
    }
    
    if(g_pGameConf->GetAddress("HidingSpot__nextID", &g_pNextID) && g_pNextID != nullptr)
    {
        return reinterpret_cast<unsigned int*>(g_pNextID);
    }

    return nullptr;
}

unsigned int *HL2::GetHidingSpot_masterMarker()
{
    if(g_pHidingSpotmasterMarker)
    {
        return reinterpret_cast<unsigned int*>(g_pHidingSpotmasterMarker);
    }
    
    if(g_pGameConf->GetAddress("HidingSpot__masterMarker", &g_pHidingSpotmasterMarker) && g_pHidingSpotmasterMarker != nullptr)
    {
        return reinterpret_cast<unsigned int*>(g_pHidingSpotmasterMarker);
    }

    return nullptr;
}

int serchMarker = -1;
unsigned int *HL2::GetNavMesh_GetNerestNavArea_SearchMarker()
{
    if(g_pSerchMarker)
    {
        return reinterpret_cast<unsigned int*>(g_pSerchMarker);
    }

    if(!g_pSerchMarker && serchMarker == -1)
    {
        void *addrInBase = (void*)g_SMAPI->GetServerFactory(false);
        if(addrInBase)
        {
            Dl_info info;
            if(dladdr(addrInBase, &info) != 0)
            {
                void *handle = dlopen(info.dli_fname, RTLD_NOW);
                if(handle)
                {
                    g_pSerchMarker = memutils->ResolveSymbol(handle, "_ZZNK8CNavMesh17GetNearestNavAreaERK6VectorbfbbE12searchMarker");
                    dlclose(handle);
                    if(g_pSerchMarker)
                    {
                        return reinterpret_cast<unsigned int*>(g_pSerchMarker);
                    }
                }
            }
        }
    }

    if(serchMarker == -1)
	{
		serchMarker = RandomInt(0, 1024*1024);
	}
    
    return (unsigned int*)&serchMarker;
}

INavMesh *HL2::GetTheNavMesh()
{
    void *addr = nullptr;
    if(g_pGameConf->GetAddress("Nav_Mesh", &addr) && addr != nullptr)
    {
        return *reinterpret_cast<INavMesh**>(addr);
    }

    return nullptr;
}

void *HL2::GetDirtyKDTree()
{
    if(!g_pDirtyKDTree)
    {
        g_pGameConf->GetAddress("ptr_DirtyKDTree", &g_pDirtyKDTree);
    }

    return g_pDirtyKDTree;
}

void *HL2::GetVoteController()
{
    return FindEntityByClassName(nullptr, "vote_controller");
}

ITeam *HL2::GetGlobalTeam(int iIndex)
{
    if(!g_pTeamGlobal)
    {
        g_pGameConf->GetAddress("GlobalTeam", &g_pTeamGlobal);
    }

    CUtlVector<ITeam*> &pTeamGlobal = reinterpret_cast<CUtlVector<ITeam *>&>(*(DWORD*)g_pTeamGlobal);
    if(iIndex < 0 || iIndex >= pTeamGlobal.Size())
    {
        return nullptr;
    }
    return pTeamGlobal[iIndex];
}

void *HL2::GetDirector()
{
    if(g_pDirector == nullptr)
    {
        g_pGameConf->GetAddress("CDirector", &g_pDirector);
    }

    return g_pDirector;
}

void *HL2::GetAIConceptTable()
{
    void *addr = nullptr;
    if(g_pGameConf->GetAddress("AI_ConceptTable", &addr) && addr != nullptr)
    {
        return addr;
    }
    return nullptr;
}

void *HL2::GetResponseQueueManager()
{
    void *addr = nullptr;
    if(g_pGameConf->GetAddress("C_ResponseQueueManager", &addr) && addr != nullptr)
    {
        return addr;
    }
    return nullptr;
}

void HL2::ClearCollisionHash()
{
    g_pCollisionPaintHash = nullptr;
    g_pPhysicsProps = nullptr;
}

void HL2::OnMobRash()
{
    if(SetupOnMobRash())
    {
        s_OnMobRash->Execute(ArgcBuffer<void *>(g_pDirector), nullptr);
    }
}

IBaseEntity* HL2::FindEntityByClassName(IBaseEntity* pEnt, const char *nameCalss)
{
    if(pEnt == nullptr)
    {
        pEnt = (IBaseEntity *)servertools->FirstEntity();
    }
    else
    {
        pEnt = (IBaseEntity *)servertools->NextEntity(pEnt);
    }
    if(pEnt == nullptr)
    {
        return nullptr;
    }

    while(pEnt != nullptr)
    {
        if(pEnt->ClassMatches(nameCalss))
        {
            return pEnt;
        }
        pEnt = (IBaseEntity *)servertools->NextEntity(pEnt);
    }

    return nullptr;
}

int HL2::GetHasAnySurvivorLeftSafeArea()
{
    if(SetupHasAnySurvivorLeftSafeArea())
    {
        int ret = -1;
        s_HasAnySurvivorLeftSafeArea->Execute(ArgcBuffer<void *>(g_pDirector), &ret);
        return ret;
    }
    return -1;
}

bool HL2::UTIL_ContainsDataTable(SendTable *pTable, const char *name)
{
    const char *pname = pTable->GetName();
    int props = pTable->GetNumProps();
    SendProp *prop;
    SendTable *table;

    if (pname && strcmp(name, pname) == 0)
        return true;

    for (int i=0; i<props; i++)
    {
        prop = pTable->GetProp(i);

        if ((table = prop->GetDataTable()) != NULL)
        {
            pname = table->GetName();
            if (pname && strcmp(name, pname) == 0)
            {
                return true;
            }

            if (UTIL_ContainsDataTable(table, name))
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Converts an entity index to a CBaseEntity and/or an edict_t pointer.
 * @param num The entity index to convert.
 * @param pEntData If not NULL, the address of a CBaseEntity pointer to store the result.
 * @param pEdictData If not NULL, the address of an edict_t pointer to store the result.
 * @return True if the conversion was successful, false otherwise.
 *
 * This function will return false if the entity index is invalid or if the entity is not a CBaseEntity.
 * Additionally, if the index is a player index, it will return false if the player is not connected.
 * If pEntData is not NULL, the address of a CBaseEntity pointer to store the result is passed.
 * If pEdictData is not NULL, the address of an edict_t pointer to store the result is passed.
 * If an edict_t is requested but the entity is not an edict_t, the function will return false.
 */
bool HL2::IndexToAThings(cell_t num, CBaseEntity **pEntData, edict_t **pEdictData)
{
    CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(num);

    if(!pEntity)
        return false;

    int index = gamehelpers->ReferenceToIndex(num);
    if(index > 0 && index <= playerhelpers->GetMaxClients())
    {
        IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(index);
        if(!pPlayer || !pPlayer->IsConnected())
        {
            return false;
        }
    }

    if(pEntData)
    {
        *pEntData = pEntity;
    }

    if(pEdictData)
    {
        edict_t *pEdict =  BaseEntityToEdict(pEntity);
        if(!pEdict || pEdict->IsFree())
        {
            pEdict = NULL;
        }

        *pEdictData = pEdict;
    }
    return true;
}

/*int __cdecl CSurvivorRescue::IsLiveSurvivorInside(void)*/
int HL2::IsLiveSurInside(CBaseEntity *gRescue)
{
    if(SetupIsLiveSurvivorInside())
    {
        int ret = -1;
        s_IsLiveSurvivorInside->Execute(ArgcBuffer<void *>(gRescue), &ret);
        return ret;
    }
    return -1;
}

/**
 * Applies damage to all entities in a sphere of a given radius.
 *
 * @param pAttack The entity responsible for the damage.
 * @param vecSrcIn The center of the sphere.
 * @param fRadius The radius of the sphere.
 * @param fDamag The amount of damage to apply.
 */
void HL2::DamageRadius(CBaseEntity *pAttack, const Vector &vecSrc, float fRadius, float fDamag)
{
    RadiusDamage(CTakeDamageInfoHack(pAttack, pAttack, fDamag, DMG_BULLET, vecSrc), vecSrc, fRadius, 0, nullptr);
}

/**
 * Applies damage to all entities in a sphere of a given radius.
 *
 * @param info The damage information structure to use for the damage.
 * @param vecSrcIn The center of the sphere.
 * @param flRadius The radius of the sphere.
 * @param iClassIgnore If non-zero, the type of entity to ignore.
 * @param bIgnoreWorld If true, ignores the world.
 */
void HL2::RadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld)
{
    CBaseEntity *pEntity = nullptr;
    trace_t     tr;
    float       flAdjustedDamage, falloff, damagePercentage;
    Vector      vecSpot;
    Vector      vecToTarget;
    Vector      vecEndPos;

    vecEndPos.Init();
    Vector vecSrc = vecSrcIn;
    damagePercentage = 1.0;

    if(flRadius)
    {
        falloff = info.GetDamage() / flRadius;
    }
    else
    {
        falloff = 1.0;
    }

    int bInWater = (g_pTarce->GetPointContents(vecSrc) & MASK_WATER) ? true : false;
    vecSrc.z += 1;

    for(CEntitySphereQuery_ sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != nullptr; sphere.NextEntity())
    {
        ITerrorPlayer *iEntity = (ITerrorPlayer *)pEntity;
        if(iEntity->m_takedamage == DAMAGE_NO)
            continue;

        if(iClassIgnore != 0 && iEntity->Classify() == iClassIgnore)
            continue;

        if(!bIgnoreWorld)
        {
            if(bInWater && iEntity->GetWaterLevel() == 0)
            {
                continue;
            }
            if(!bInWater && iEntity->GetWaterLevel() == 3)
            {
                continue;
            }
        }

        vecSpot = iEntity->BodyTarget(vecSrc);
        bool bHit = false;

        if(bIgnoreWorld)
        {
            vecEndPos = vecSpot;
            bHit = true;
        }
        else
        {
            damagePercentage = GetAmountOfEntityVisible(vecSrc, iEntity);
            if(damagePercentage > 0.0)
            {
                vecEndPos = vecSpot;
                bHit = true;
            }
        }

        if(bHit)
        {
            vecToTarget = ( vecEndPos - vecSrc );

            flAdjustedDamage = vecToTarget.Length() * falloff;
            flAdjustedDamage = info.GetDamage() - flAdjustedDamage;
            flAdjustedDamage = flAdjustedDamage * damagePercentage;
        
            if ( flAdjustedDamage > 0 )
            {
                CTakeDamageInfo adjustedInfo = info;

                IBaseCombatWeapon *pWeapon = reinterpret_cast<IBaseCombatCharacter*>(info.GetAttacker())->GetActiveWeapon();
                if(pWeapon)
                {
                    if( g_Sample.my_bStrcmp("weapon_hunting_rifle",  pWeapon->GetName()) ||
                        g_Sample.my_bStrcmp("weapon_rifle",          pWeapon->GetName()))
                    {
                        if(iEntity->ClassMatches("witch"))
                        {
                            adjustedInfo.SetDamageType(DMG_BLAST);
                        }
                        else if(iEntity->ClassMatches("player"))
                        {
                            switch (iEntity->GetClass())
                            {
                                case ZombieClassSmoker:
                                case ZombieClassBommer:
                                case ZombieClassHunter:
                                    adjustedInfo.SetDamageType(DMG_BLAST);
                                    break;
                            }
                        }
                    }
                }

                adjustedInfo.SetDamage( flAdjustedDamage );

                Vector dir = vecToTarget;
                VectorNormalize( dir );

                if ( adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin )
                {
                    CalculateExplosiveDamageForce( &adjustedInfo, dir, vecSrc, 1.5 );
                }
                else
                {
                    float flForce = adjustedInfo.GetDamageForce().Length() * falloff;
                    adjustedInfo.SetDamageForce( dir * flForce );
                    adjustedInfo.SetDamagePosition( vecSrc );
                }

                Vector vecTarget;
                vecTarget = iEntity->BodyTarget(vecSrc, false);

                CTraceFilterSimples mFilter(nullptr, COLLISION_GROUP_NONE);
                util_TraceLine(vecSrc, vecTarget, MASK_SHOT, &mFilter, &tr);

                if (tr.fraction != 1.0)
                {
                    ClearMultiDamages();
                    iEntity->DispatchTraceAttack(adjustedInfo, dir, &tr);
                    ApplyMultiDamages();
                }
                else
                {
                    iEntity->TakeDamage(adjustedInfo);
                }

                iEntity->TraceAttackToTriggers(adjustedInfo, vecSrc, vecEndPos, dir);
            }
        }
    }
}

float HL2::GetAmountOfEntityVisible(Vector &vecSrc, IBaseEntity *pEntity)
{
    float retval = 0.0;

    const float damagePercentageChest       = 0.40;
    const float damagePercentageHead        = 0.20;
    const float damagePercentageFeet        = 0.20;
    const float damagePercentageRightSide   = 0.10;
    const float damagePercentageLeftSide    = 0.10;

    if (!(pEntity->IsPlayer()))
    {
        Vector vecTarget;
        vecTarget = pEntity->BodyTarget(vecSrc, false);

        return GetExplosionDamageAdjustment(vecSrc, vecTarget, pEntity);
    }

    float chestHeightFromFeet;
    float armDistanceFromChest = 16.f;

    Vector vecFeet = pEntity->GetAbsOrigin();

    Vector vecChest = pEntity->BodyTarget(vecSrc, false);
    chestHeightFromFeet = vecChest.z - vecFeet.z;

    Vector vecHead = pEntity->GetAbsOrigin();
    vecHead.z += 72.f;

    Vector vecRightFacing;
    AngleVectors(pEntity->GetAbsAngles(), NULL, &vecRightFacing, NULL);

    vecRightFacing.NormalizeInPlace();
    vecRightFacing = vecRightFacing * armDistanceFromChest;

    Vector vecLeftSide = pEntity->GetAbsOrigin();
    vecLeftSide.x -= vecRightFacing.x;
    vecLeftSide.y -= vecRightFacing.y;
    vecLeftSide.z += chestHeightFromFeet;

    Vector vecRightSide = pEntity->GetAbsOrigin();
    vecRightSide.x += vecRightFacing.x;
    vecRightSide.y += vecRightFacing.y;
    vecRightSide.z += chestHeightFromFeet;

    float damageAdjustment = GetExplosionDamageAdjustment(vecSrc, vecChest, pEntity);
    retval += (damagePercentageChest * damageAdjustment);

    damageAdjustment = GetExplosionDamageAdjustment(vecSrc, vecHead, pEntity);
    retval += (damagePercentageHead * damageAdjustment);

    damageAdjustment = GetExplosionDamageAdjustment(vecSrc, vecFeet, pEntity);
    retval += (damagePercentageFeet * damageAdjustment);

    damageAdjustment = GetExplosionDamageAdjustment(vecSrc, vecLeftSide, pEntity);
    retval += (damagePercentageLeftSide * damageAdjustment);

    damageAdjustment = GetExplosionDamageAdjustment(vecSrc, vecRightSide, pEntity);
    retval += (damagePercentageRightSide * damageAdjustment);

    return retval;
}

float HL2::GetExplosionDamageAdjustment(Vector &vecSrc, Vector &vecEnd, IBaseEntity *pEntToIgnore)
{
    float retval = 0.0;
    trace_t tr;

    CTraceFilterSimples traceFilter(pEntToIgnore->GetNetworkable()->GetEntityHandle(), COLLISION_GROUP_NONE);
    util_TraceLine(vecSrc, vecEnd, MASK_SHOT, &traceFilter, &tr);
    if (tr.fraction == 1.0)
    {
        retval = 1.0;
    }
    else if (!(tr.m_pEnt == this->GetWorldEnt()) && 
    (tr.m_pEnt != NULL) && 
    ((IBaseEntity *)tr.m_pEnt != pEntToIgnore) && 
    (reinterpret_cast<IBaseEntity *>(tr.m_pEnt)->GetOwnerEntity() != pEntToIgnore))
    {
        CBaseEntity *blockingEntity = tr.m_pEnt;
        CTraceFilterSimples traceFilter(NULL, COLLISION_GROUP_NONE);
        util_TraceLine(vecSrc, vecEnd, MASK_SHOT, &traceFilter, &tr);

        if (tr.fraction == 1.0)
        {
            if ((blockingEntity != NULL) && (pEntToIgnore->VPhysicsGetObject() != NULL))
            {
                int nMaterialIndex = pEntToIgnore->VPhysicsGetObject()->GetMaterialIndex();

                float flDensity;
                float flThickness;
                float flFriction;
                float flElasticity;

                this->GetSurfaceProps()->GetPhysicsProperties( nMaterialIndex, &flDensity, &flThickness, &flFriction, &flElasticity );

                const float DENSITY_ABSORB_ALL_DAMAGE = 3000.0;
                float scale = flDensity / DENSITY_ABSORB_ALL_DAMAGE;
                if ((scale >= 0.0) && (scale < 1.0))
                {
                    retval = 1.0 - scale;
                }
                else if (scale < 0.0)
                {
                    retval = 1.0;
                }
            }
            else
            {
                retval = 0.75;
            }
        }
    }

    return retval;
}

void HL2::CalculateExplosiveDamageForce(CTakeDamageInfo *info, const Vector &vecDir, const Vector &vecForceOrigin, float flScale)
{
    auto ImpScale = [](float flTargetMass, float flDesiredSpeed)
    {
        return (flTargetMass * flDesiredSpeed);
    };

	info->SetDamagePosition( vecForceOrigin );
	float flClampForce = ImpScale( 75, 400 );
	float flForceScale = info->GetBaseDamage() * ImpScale( 75, 4 );

	if( flForceScale > flClampForce )
		flForceScale = flClampForce;

	flForceScale *= ::RandomFloat( 0.85, 1.15 );

	Vector vecForce = vecDir;
	VectorNormalize( vecForce );
	vecForce *= flForceScale;
	vecForce *= g_pConVar->GetConVarFloat("phys_pushscale");
	vecForce *= flScale;
	info->SetDamageForce( vecForce );
}

void HL2::PlayerVomitUpon(CBaseEntity *pEntity, CBaseEntity *aEntity, cell_t params)
{
	if(SetupVomitUpon())
	{
        ArgcBuffer<CBaseEntity *, CBaseEntity *, cell_t> pParam(pEntity, aEntity, params);
		s_VomitUpon->Execute(pParam, nullptr);
	}
}

int64_t HL2::GetReserveLobby()
{
    if(g_pServer == nullptr)
    {
        g_pSM->LogError(myself, "[UkrCoop] g_pServer is NULL");
        return -1;
    }

    if(this->SetupReserveLobby())
    {
        int64_t cookie = 0;
        s_Reserver->Execute(ArgcBuffer<void *>(g_pServer), &cookie);
        return cookie;
    }
    return -1;
}

//CBaseServer::SetReservationCookie(uint64_t reservationCookie, const char* formatString, va_list ap)
void HL2::UnReserveLobby()
{
    if(g_pServer == nullptr)
    {
        g_pSM->LogError(myself, "[UkrCoop] g_pServer is NULL");
        return;
    }

    if(this->SetupUnReserveLobby())
    {
        const uint64_t cookieUnreserve = 0;
        ArgcBuffer<IServer*, uint64_t, char const*, void *> pParam(g_pServer, cookieUnreserve, "Manually unreserved by UkrCoop extenshin", nullptr);
        s_UnReserver->Execute(pParam, nullptr);
    }
}

void HL2::SetOriginal(CBaseEntity *pEnt, Vector *vecSet, bool bFireTriggers)
{
    if(SetupSetOrigin())
    {
        ArgcBuffer<CBaseEntity *, Vector*, bool> vparam(pEnt, vecSet, bFireTriggers);
        s_SetOrigin->Execute(vparam, nullptr);
    }
}

IPhysicsObject *HL2::PhysModelCreate(IBaseEntity *pEnt, int modelIndex, const Vector *origin, const QAngle *angles, solid_t *pSolid)
{
    if(SetupPhysModelCreate())
    {
        void *pResul = nullptr;
        VCaller::ArgcBuffer<IBaseEntity*, int, const Vector*, const QAngle*, solid_t*> vparam(pEnt, modelIndex, origin, angles, pSolid);
        s_PhysModelCreate->Execute(vparam, &pResul);
        return (IPhysicsObject*)pResul;
    }
    return nullptr;
}

IPhysicsObject *HL2::PhysModelCreateBox(IBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, bool isStatic)
{
    if(SetupPhysModelCreateBox())
    {
        void *pResul = nullptr;
        VCaller::ArgcBuffer<IBaseEntity*, const Vector *, const Vector *, const Vector*, bool> pParam(pEntity, &mins, &maxs, &origin, isStatic);
        s_PhysModelCreateBox->Execute(pParam, &pResul);
        return (IPhysicsObject*)pResul;
    }

    return nullptr;
}

IPhysicsObject *HL2::PhysModelCreateOBB(IBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, const QAngle &angle, bool isStatic)
{
    if(SetupPhysModelCreateOBB())
    {
        void* pResult = nullptr;
        VCaller::ArgcBuffer<void*, const void*, const void*, const void*, const void*, bool> pParam(pEntity, &mins, &maxs, &origin, &angle, isStatic);
        s_PhysModelCreateOBB->Execute(pParam, &pResult);
        return (IPhysicsObject*)pResult;
    }
    return nullptr;
}

bool HL2::IsLineOfSightBetweenTwoEntitiesClear(IBaseEntity *pSrcEntity, EEntityOffsetMode_t nSrcOffsetMode, IBaseEntity *pDestEntity, EEntityOffsetMode_t nDestOffsetMode, IBaseEntity *pSkipEntity, int nCollisionGroup, unsigned int nTraceMask, ShouldHitFunc_t pTraceFilterCallback, float flMinimumUpdateInterval)
{
    bool bResult = false;
    if(SetupIsLineOfSightBetweenTwoEntitiesClear())
    {
        VCaller::ArgcBuffer<IBaseEntity *, EEntityOffsetMode_t, IBaseEntity *, EEntityOffsetMode_t, IBaseEntity *, int, unsigned int, ShouldHitFunc_t, float> vparam(pSrcEntity, nSrcOffsetMode, pDestEntity, nDestOffsetMode, pSkipEntity, nCollisionGroup, nTraceMask, pTraceFilterCallback, flMinimumUpdateInterval);
        s_IsLineOfSightBetweenTwoEntitiesClear->Execute(vparam, &bResult);
    }
    return bResult;
}

char *HL2::GetTransitionedLandmarkName()
{
    char *pName = nullptr;
    if(SetupGetTransitionedLandmarkName())
    {
        s_GetTransitionedLandmarkName->Execute(nullptr, &pName);
    }

    return pName;
}

Activity HL2::GetIndexForName(const char *pszActivityName)
{
    if(SetupGetIndexForName())
    {
        int nResult = -1;
        ArgcBuffer<const char*> param(pszActivityName);
        s_ActivityList_IndexForName->Execute(param, &nResult);
        return static_cast<Activity>(nResult);
    }

    return ACT_INVALID;
}

Activity HL2::RegisterPrivateActivity(const char *pszActivityName)
{
    if(SetupRegisterPrivateActivity())
    {
        int nResult = -1;
        ArgcBuffer<const char*> param(pszActivityName);
        s_RegisterPrivateActivity->Execute(param, &nResult);
        return static_cast<Activity>(nResult);
    }

    return ACT_INVALID;
}

void HL2::CEventQueueAdd(const char *target, const char *targetInput, variant_t Value, float fireDelay, IBaseEntity *pActivator, IBaseEntity *pCaller, int outputID)
{
    if(SetupEventQueueAdd())
    {
        void *pThis = CEventQueue();
        if(!pThis)
        {
            Msg("CEventQueue is null\n");
            return;
        }

        VCaller::ArgcBuffer<void *, const char*, const char*, variant_t, float, void*, void*, int> pParam(pThis, target, targetInput, Value, fireDelay, pActivator, pCaller, outputID);
        s_CEventQueue_AddEvent->Execute(pParam, nullptr);
    }
}

bool HL2::AddStepDiscontinuity(CBaseEntity *pClient, float flTime, Vector *vecOrigin, QAngle *vecAngles)
{
    bool resul = false;

    if(SetupAddStepDiscontinuity())
    {
        ArgcBuffer<CBaseEntity*, float, Vector*, QAngle*> param(pClient, flTime, vecOrigin, vecAngles);
        s_AddStepDiscon->Execute(param, &resul);
    }

    return resul;
}

IBaseEntity *HL2::FindEntityByName(IBaseEntity *pStartEntity, const char *szName, IBaseEntity *pSearchEntity, IBaseEntity *pActivator, IBaseEntity *pCaller, IEntityFindFilter *pFilter)
{
    IBaseEntity *pFindEntity = nullptr;
    if(SetupFindEntityByName())
    {
        ArgcBuffer<void *, IBaseEntity*, const char*, IBaseEntity*, IBaseEntity*, IBaseEntity*, IEntityFindFilter*> param(g_pEntityList, pStartEntity, szName, pSearchEntity, pActivator, pCaller, pFilter);
        s_FindEntityByName->Execute(param, &pFindEntity);
    }

    return pFindEntity;
}

void HL2::PhysicsTouchTriggers(const CBaseEntity *pThisPtr, const Vector *pPrevAbsOrigin)
{
    if(SetupPhysicsTouchTrigger())
    {
        ArgcBuffer<const CBaseEntity *, const Vector *> param(pThisPtr, pPrevAbsOrigin);
        s_PhysicsTouchTriger->Execute(param, nullptr);
    }
}

CBaseEntity *HL2::NPCPhysicsCreateSolver(CBaseEntity *pNPC, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationDuration)
{
    CBaseEntity *pResult = nullptr;

    if(SetupNPCPhysicsCreateSolver())
    {
        ArgcBuffer<CBaseEntity *, CBaseEntity *, bool, float> param(pNPC, pPhysicsObject, disableCollisions, separationDuration);
        s_NPCPhysics_CreateSolver->Execute(param, &pResult);
    }

    return pResult;
}

void HL2::LockStudioHdr(IBaseAnimating *pThisPtr)
{
    if(SetupLockStudioHdr())
    {
        ArgcBuffer<IBaseAnimating*> param(pThisPtr);
        s_LockStudioHdr->Execute(param, nullptr);
    }
    return;
}

int HL2::LookupPoseParam(CBaseEntity *pThisPtr, void *modePtr, const char *szName)
{
    int iResult = -2;

    if(SetupLookupPoseParam())
    {
        ArgcBuffer<CBaseEntity*, void *, const char *> param(pThisPtr, modePtr, szName);
        s_LookupPoseParam->Execute(param, &iResult);
    }

    return iResult;
}

bool HL2::HasPoseParam(CBaseEntity *pThisPtr, int iSeqence, int iParam)
{
    bool bResult = false;
    if(SetupHasPoseParam())
    {
        ArgcBuffer<CBaseEntity*, int, int> param(pThisPtr, iSeqence, iParam);
        s_HasPoseParam->Execute(param, &bResult);
    }

    return bResult;
}

int HL2::SetSequence(CBaseEntity *pThisPtr, int nSequence)
{
    int result = -1;
    if(SetupSetSequence())
    {
        ArgcBuffer<CBaseEntity *, int> param(pThisPtr, nSequence);
        s_SetSequence->Execute(param, &result);
    }

    return result;
}

float HL2::SetPoseParam(CBaseEntity *pThisPtr, void *pModel, int iParam, float flValue)
{
    float flResult = -1.f;

    if(SetupSetPoseParam())
    {
        ArgcBuffer<CBaseEntity *, void *, int, float> param(pThisPtr, pModel, iParam, flValue);
        s_SetPoseParam->Execute(param, &flResult);
    }

    return flResult;
}

void HL2::UTIL_Remove(void *pRemovePtr)
{
    if(SetupRemove())
    {
        ArgcBuffer<void *> param(pRemovePtr);
        s_UTIL_Remove->Execute(param, nullptr);
    }
}

void *HL2::GetAmmoDef()
{
    void *ptr = nullptr;
    if(SetupGetAmmoDef())
    {
        s_GetAmmoDef->Execute(nullptr, &ptr);
    }
    return ptr;
}

bool HL2::IsPhysIsInCallback()
{
    bool ret = false;
    if(SetuppHysIsInCallback())
    {
        s_PhysIsInCallback->Execute(nullptr, &ret);
    }
    
    return ret;
}

void HL2::PhysCallBackDamage(CBaseEntity *pEnt, const CTakeDamageInfo &info)
{
    if(SetupPhysCallBackDamage())
    {
        ArgcBuffer<CBaseEntity *, const CTakeDamageInfo *> param(pEnt, &info);
        s_PhysCallbackDamage->Execute(param, nullptr);
    }
}

void HL2::ApplyMultiDamages(void)
{
    if(SetupApplyMultiDamag())
    {
        s_ApplyMultiDamag->Execute(nullptr, nullptr);
    }
}

void HL2::ClearMultiDamages()
{
    if(SetupClearMultiDamag())
    {
        s_ClearMultiDamag->Execute(nullptr, nullptr);
    }
}

void *HL2::GetWorldEnt()
{
    void *pWorld = nullptr;
    if(SetupGetWorldEntity())
    {
        s_GetWorldEntity->Execute(nullptr, &pWorld);
    }

    return pWorld;
}

void HL2::OnGSClientApprove(void *pThisPtr, void *pGSClientApprove)
{
    if(SetupOnGSCLientApprove())
    {
        ArgcBuffer<void*, void*> pParam(pThisPtr, pGSClientApprove);
        s_OnGSClientApprove->Execute(pParam, nullptr);
    }
}

void HL2::PrecacheGibsForModel(int iModel)
{
    if(SetupPrecacheGibsForModel())
    {
        ArgcBuffer<int> pParam(iModel);
        s_PrecacheGibsForModel->Execute(pParam, nullptr);
    }
}

void *HL2::CreateDataObjects(CBaseEntity *pEnt, int type)
{
    void *result = nullptr;
    if(SetupCreateDataObjects())
    {
        ArgcBuffer<CBaseEntity*, int> pParam(pEnt, type);
        s_CreateDataObj->Execute(pParam, &result);
    }
    return result;
}

void HL2::DestroyDataObjects(CBaseEntity *pEnt, int type)
{
    if(SetupDestroyDataObjects())
    {
        ArgcBuffer<CBaseEntity *, int> pParam(pEnt, type);
        s_DestroyDataObj->Execute(pParam, nullptr);
    }
}

void *HL2::GetDataObject(CBaseEntity *pEnt, int type)
{
    void *result = nullptr;

    if(SetupGetDataObject())
    {
        ArgcBuffer<CBaseEntity *, int> pParam(pEnt, type);
        s_GetDataObject->Execute(pParam, &result);
    }

    return result;
}

void HL2::PhysicsRemoveGround(void *other, void *link)
{
    if(SetupPhysicsRemoveGround())
    {
        ArgcBuffer<void *, void *> pParam(other, link);
        s_PhysicsRemoveGround->Execute(pParam, nullptr);
    }
}

void HL2::SimThink_EntityChanged(CBaseEntity *pEnt)
{
    if(SetupSimThinkEntityChanged())
    {
        ArgcBuffer<CBaseEntity *> pParam(pEnt);
        s_SimThinkEntityChanged->Execute(pParam, nullptr);
    }
}

void HL2::EntityTouchAdd(CBaseEntity *pThis)
{
    if(SetupEntityTouchAdd())
    {
        ArgcBuffer<CBaseEntity*> pParam(pThis);
        s_EntityTouchAdd->Execute(pParam, nullptr);
    }
}

int HL2::EventListIndexForName(const char *pszName)
{
    int res = -1;
    if(SetupEventListIndexForName())
    {
        ArgcBuffer<const char*> pParam(pszName);
        s_EventListIndexForName->Execute(pParam, &res);
    }
    return res;
}

int HL2::EventListRegisterPrivateEvent(const char *szName)
{
    int result = -1;
    if(SetupEventListRegisterPrivateEvent())
    {
        ArgcBuffer<const char *> pParam(szName);
        s_EventListRegisterPrivateEvent->Execute(pParam, &result);
    }
    return result;
}

int HL2::EventListGetEventType(int event_index)
{
    int result = -1;
    if(SetupEventListGetEventType())
    {
        ArgcBuffer<int> pParam(event_index);
        s_EventListGetEventType->Execute(pParam, &result);
    }
    
    return result;
}

void *HL2::AddEntityToGroundList(IBaseEntity *pThis, IBaseEntity *pEnt)
{
    void *pResult = nullptr;
    if(SetupAddEntityToGroundList())
    {
        ArgcBuffer<IBaseEntity*, IBaseEntity*> pParam(pThis, pEnt);
        s_AddEntityToGroundList->Execute(pParam, &pResult);
    }

    return pResult;
}

void HL2::ReportEntityFlagsChanged(CBaseEntity *pEnt, unsigned int flagsOld, unsigned int flagsNew)
{
    if(SetupReportEntityFlagsChanged())
    {
        ArgcBuffer<void*, void*, unsigned int, unsigned int> pParam(g_pEntityList, pEnt, flagsOld, flagsNew);
        s_ReportEntityFlagsChanged->Execute(pParam, nullptr);
    }
}

void HL2::ReportPositionChanged(CBaseEntity *pEnt)
{
    if(SetupReportPositionChanged())
    {
        ArgcBuffer<void *> pParam(pEnt);
        s_ReportPositionChanged->Execute(pParam, nullptr);
    }
}

void HL2::TransitionPlayerCount(int *v1, int *v2, int team)
{
    if(SetupTransitionPlayerCount())
    {
        ArgcBuffer<int *, int *, int> pParam(v1, v2, team);
        s_TransitionPlayerCount->Execute(pParam, nullptr);
    }
}

void* HL2::GetTheNextBots()
{
    void *ret = nullptr;
    if(SetupGetTheNextBots())
    {
        s_TheNextBot->Execute(nullptr, &ret);
    }
    return ret;
}

int HL2::Select_Weighted_Sequence(void *pModel, int activity, int curSeq)
{
    int result = -1;
    if(SetupSelect_Weighted_Sequence())
    {
        VCaller::ArgcBuffer<void*, int, int> pParam(pModel, activity, curSeq);
        s_Select_Weighted_Sequence->Execute(pParam, &result);
    }
    return result;
}

void HL2::CBaseEntity_EmitSound(IRecipientFilter &filter, int iEntIndex, const EmitSound_t &params)
{
    if(SetupCBaseEntityEmitSound1())
    {
        VCaller::ArgcBuffer<IRecipientFilter*, int, const EmitSound_t*> pParam(&filter, iEntIndex, &params);
        s_CBaseEntity9EmitSound1->Execute(pParam, nullptr);
    }
}

void HL2::CBaseEntity_EmitSound(IRecipientFilter &filter, int iEntIndex, const EmitSound_t &params, HSOUNDSCRIPTHANDLE &handle)
{
    if(SetupCBaseEntityEmitSound2())
    {
        VCaller::ArgcBuffer<IRecipientFilter*, int, const EmitSound_t*, HSOUNDSCRIPTHANDLE> pParam(&filter, iEntIndex, &params, handle);
        s_CBaseEntity9EmitSound2->Execute(pParam, nullptr);
    }
}

int HL2::CSoundEnt_InsertSound(int iType, const Vector &vecOrigion, int iVolume, float flDyration, CBaseEntity *pOwner, int soundChannelIndex, CBaseEntity *pSoundTarget)
{
    int result = -1;
    if(SetupCSoundEntInsertSound())
    {
        VCaller::ArgcBuffer<int, const Vector*, int, float, CBaseEntity*, int, CBaseEntity*> pParam(iType, &vecOrigion, iVolume, flDyration, pOwner, soundChannelIndex, pSoundTarget);
        s_CSoundEntInsertSound->Execute(pParam, &result);
    }
    return result;
}

void HL2::CResponseQueue_Add(const CAI_Concept &concepts, const void *context, float time, const CFollowupTargetSpec_t &targetspec, void *pIssuer)
{
    if(SetupCResponseQueueAdd())
    {
        CResponseQueueManager *pManager = (CResponseQueueManager*)GetResponseQueueManager();
        if(pManager)
        {
            VCaller::ArgcBuffer<const void *, const void *, const void *, float, const void*, void*> pParam(pManager->GetQueue(), &concepts, context, time, &targetspec, pIssuer);
            s_CResponseQueueAdd->Execute(pParam, nullptr);
        }
    }
}

bool HL2::IsBreakableEntity(CBaseEntity *pentity, bool val1, bool val2)
{
    bool bResult = false;
    if(SetupIsBreakableEntity())
    {
        VCaller::ArgcBuffer<void*, bool, bool> pParam(pentity, val1, val2);
        s_IsBreakableEntity->Execute(pParam, &bResult);
    }
    return bResult;
}

void HL2::Infected_SetDamagedBodyGroupVariant(void *me, const char *szVal1, const char *szVal2)
{
    if(SetupInfectedSetDamagedBodyGroupVariant())
    {
        VCaller::ArgcBuffer<void*, const char*, const char*> pParam(me, szVal1, szVal2);
        s_SetDamagedBodyGroupVariant->Execute(pParam, nullptr);
    }
}

#include "MemberFunctionWrapper.h"
static MemberFunctionWrapper<string_t, const char*> func_AllocPooledString;
string_t HL2::AllocPooledString(const char *name)
{
    if(func_AllocPooledString == nullptr)
    {
        if(!g_pGameConf->GetMemSig("AllocPooledString", func_AllocPooledString))
        {
            return NULL_STRING;
        }
    }

    return func_AllocPooledString(name);
}

void HL2::CCSPlayer_State_Transition(void *pPlayer, int newState)
{
    if(SetupState_Transition())
    {
        VCaller::ArgcBuffer<void*, int> pParam(pPlayer, newState);
        s_State_Transition->Execute(pParam, nullptr);
    }
}

bool HL2::Director_IsVisibleToTeam(const Vector &vec, int val1, int val2, float val3, INavArea *val4, const CBaseEntity *val5)
{
    bool isResult = false;
    if(SetupDirectorIsVisibleToTeam())
    {
        auto pDirector = GetDirector();
        if(!pDirector)
        {
            Msg("Director is nullptr\n");
            return false;
        }

        VCaller::ArgcBuffer<void*, const void*, int, int, float, void*, const void*> pParam(pDirector, &vec, val1, val2, val3, val4, val5);
        s_DirectorIsVisibleToTeam->Execute(pParam, &isResult);
    }

    return isResult;
}

bool HL2::Director_IsVisibleToTeam(const IBaseEntity *pEntity, int team, int val1, float val2, INavArea *area)
{
    if(pEntity->GetTeamNumber() == team)
        return true;

    INavArea* pArea = nullptr;
    if(!area)
    {
        auto pCahacterPtr = (IBaseCombatCharacter*)const_cast<IBaseEntity*>(pEntity)->MyCombatCharacterPointer();
        if(pCahacterPtr)
        {
            pArea = (INavArea*)pCahacterPtr->GetLastKnownArea();
        }
    }
    else
    {
        pArea = area;
    }

    Vector vec_WSC = pEntity->WorldSpaceCenter();
    if(Director_IsVisibleToTeam(vec_WSC, team, val1, val2, pArea, (CBaseEntity*)pEntity))
    {
        return true;
    }

    Vector vecAbsOrigion = pEntity->GetAbsOrigin();
    if(Director_IsVisibleToTeam(vecAbsOrigion, team, val1, val2, pArea, (CBaseEntity*)pEntity))
    {
        return true;
    }

    return Director_IsVisibleToTeam(const_cast<IBaseEntity*>(pEntity)->EyePosition(), team, val1, val2, pArea, (CBaseEntity*)pEntity);
}

bool HL2::Director_IsTransitioned()
{
    if(SetupDirector_IsTransitioned())
    {
        auto pDirector = GetDirector();
        if(pDirector)
        {
            bool bResult = false;
            VCaller::ArgcBuffer<void*> pParam(pDirector);
            s_Director_IsTransitioned->Execute(pParam, &bResult);
            return bResult;
        }
    }

    return false;
}

bool HL2::IsMotionControlledXY(IBaseEntity *pThis, Activity activity)
{
    if(SetupIsMotionControlledXY())
    {
        bool bResult = false;
        VCaller::ArgcBuffer<void*, int> param(pThis, activity);
        s_IsMotionControlledXY->Execute(param, &bResult);
        return bResult;
    }
    return false;
}

bool HL2::IsMotionControlledZ(IBaseEntity *pThis, Activity activity)
{
    if(SetupIsMotionControlledZ())
    {
        bool bResult = false;
        VCaller::ArgcBuffer<void*, int> param(pThis, activity);
        s_IsMotionControlledZ->Execute(param, &bResult);
        return bResult;
    }

    return false;
}

int HL2::UTIL_EntityInSphere(const Vector &center, float radius, CFlaggedEntitiesEnum_ *pEnum)
{
    g_pPartition->EnumerateElementsInSphere(PARTITION_ENGINE_NON_STATIC_EDICTS, center, radius, false, pEnum);
    return pEnum->GetCount();
}

int HL2::UTIL_EntityesInSphere(CBaseEntity **pList, int listMax, const Vector &center, float radius, int flagMask)
{
    CFlaggedEntitiesEnum_ sphereEnum(pList, listMax, flagMask);
    return g_HL2->UTIL_EntityInSphere(center, radius, &sphereEnum);
}

float HL2::GetSequenceMoveYam(CBaseEntity *pClient, int nSequence)
{
    float val = 0.f;
    if(SetupGetSeqenceMoveYam())
    {
        ArgcBuffer<CBaseEntity*, int> param(pClient, nSequence);
        s_GetSequenceMoveYam->Execute(param, &val);
    }
    return val;
}

//_DWORD __cdecl CTerrorPlayer::TakeOverBot(bool)
void HL2::takeOversBot(CBaseEntity *pEntity, bool switchs)
{
    if(this->SetupTakeOversBot())
    {        
        s_TakeOver->Execute(ArgcBuffer<CBaseEntity *, bool>(pEntity, switchs), NULL);
   }
}

//_DWORD __cdecl SurvivorBot::SetHumanSpectator(CTerrorPlayer *)
void HL2::setHumansSpec(CBaseEntity *bEntity, CBaseEntity *pEntity)
{
    if(this->SetupSetHumansSpec())
    {
        s_HumenSpec->Execute(ArgcBuffer<CBaseEntity *, CBaseEntity *>(bEntity, pEntity), NULL);
    }
}

void HL2::PlayerStargget(CBaseEntity *pEntity, CBaseEntity *tEntity, Vector *tVector)
{
    if(this->SetupStargget())
    {
        s_Stargged->Execute(ArgcBuffer<CBaseEntity *, CBaseEntity *, Vector *>(pEntity, tEntity, tVector), NULL);
    }
}

IBaseEntity *HL2::FindEntityGeneric(IBaseEntity *pStartEntity, const char *szName, IBaseEntity *pSearchingEntity, IBaseEntity *pActivator, IBaseEntity *pCaller)
{
    IBaseEntity*pEntity = nullptr;
    pEntity = FindEntityByName(pStartEntity, szName, pSearchingEntity, pActivator, pCaller);
    if(!pEntity)
        pEntity = FindEntityByClassName(pStartEntity, szName);

    return nullptr;
}

void *HL2::GetFileWeaponInfoFromHandlet(unsigned short handle)
{
    if(SetupGetFileWeaponInfoFromHandlet())
    {
        void *pResult;
        ArgcBuffer<unsigned short> param(handle);

        s_GetFileWeaponInfoFromHandlet->Execute(param, &pResult);
        return pResult;
    }

    return nullptr;
}

IMoveHelperServer *HL2::MoveHelperServerv()
{
    if(SetupMoveHelperServerv())
    {
        void* pResult;
        s_MoveHelperServerv->Execute(nullptr, &pResult);
        return (IMoveHelperServer*)pResult;
    }

    return nullptr;
}

IBaseEntity* HL2::FindEntityByClassNameNearest(const char*szName, const Vector& vecSrc, float flRadius)
{
    IBaseEntity *pEntity = nullptr;
    float flMaxDist2 = flRadius * flRadius;
    if(flMaxDist2 == 0)
    {
        flMaxDist2 = ( 1.732050807569 * (2*(16384)) ) * ( 1.732050807569 * (2*(16384)) );
    }

    IBaseEntity *pSearch = nullptr;
    while((pSearch = FindEntityByClassName(pSearch, szName)) != nullptr)
    {
        if(!pSearch->edict())
            continue;

        float flDist2 = (pSearch->GetAbsOrigin() - vecSrc).LengthSqr();
        if(flMaxDist2 > flDist2)
        {
            pEntity = pSearch;
            flMaxDist2 = flDist2;
        }
    }

    return pEntity;
}

bool HL2::Translate(char *buffer, size_t maxlength, const char *format, unsigned int numparams, size_t *pOutLength, ...)
{
	va_list ap;
	unsigned int i;
	const char *fail_phrase;
	void *params[MAX_TRANSLATE_PARAMS];

	if (numparams > MAX_TRANSLATE_PARAMS){
		return false;
	}

	va_start(ap, pOutLength);
	for (i = 0; i < numparams; i++){
		params[i] = va_arg(ap, void *);
	}
	va_end(ap);

	if (!ipharases->FormatString(buffer, maxlength, format, params, numparams, pOutLength, &fail_phrase))
	{
		if(fail_phrase != NULL)
		{
			g_pSM->LogError(myself, "[UkrCoop] Could not find core phrase: %s", fail_phrase);
		} else {
			g_pSM->LogError(myself, "[UkrCoop] Unknown fatal error while translating a core phrase.");
		}
		return false;
	}
	return true;
}

void HL2::PrintToConsole(int client, const char* msg, ...)
{
	char buffer[1024];
	va_list ap;

	va_start(ap, msg);
	size_t len = vsnprintf(buffer, sizeof(buffer), msg, ap);
	va_end(ap);

	if(len >= sizeof(buffer) - 1)
	{
		buffer[sizeof(buffer) - 2] = '\n';
		buffer[sizeof(buffer) - 1] = '\0';
	} else {
		buffer[len++] = '\n';
		buffer[len] = '\0';
	}

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
	if(pPlayer)
    {
		if(!pPlayer->IsConnected() && !pPlayer->IsInGame())
		{
			return;
		}
	}
	engine->ClientPrintf(pPlayer->GetEdict(), buffer);
}

void HL2::PrintToConsole(edict_t *m_pEndict, const char* msg, ...)
{
	char buffer[1024];
	va_list ap;

	va_start(ap, msg);
	size_t len = vsnprintf(buffer, sizeof(buffer), msg, ap);
	va_end(ap);

	if(len >= sizeof(buffer) - 1)
	{
		buffer[sizeof(buffer) - 2] = '\n';
		buffer[sizeof(buffer) - 1] = '\0';
	} else {
		buffer[len++] = '\n';
		buffer[len] = '\0';
	}
	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(m_pEndict);
	if(pPlayer)
    {
		if(!pPlayer->IsConnected() && !pPlayer->IsInGame())
		{
			return;
		}
	}
	engine->ClientPrintf(m_pEndict, buffer);
}

#define FIND_PROP_DATA(td, ret) \
    datamap_t *pMap; \
    if((pMap = GetVirtualClass<ITerrorPlayer>(pEntity)->GetDataDescMap()) == NULL) \
    { \
        return ret; \
    } \
    sm_datatable_info_t info; \
    if(!gamehelpers->FindDataMapInfo(pMap, prop, &info)) \
    { \
        return ret; \
    } \
    td = info.prop;

#define CHECK_SET_PROP_DATA_OFFSET(ret) \
    if(element < 0 || element >= td->fieldSize) \
    { \
        return ret; \
    } \
    offset = info.actual_offset + (element * (td->fieldSizeInBytes / td->fieldSize));

#define FIND_PROP_SEND(type, ret) \
    sm_sendprop_info_t info; \
    SendProp *pProp; \
    IServerUnknown *pUnk = (IServerUnknown *)pEntity; \
    IServerNetworkable *pNet = pUnk->GetNetworkable(); \
    if(!pNet) \
        return ret; \
         \
    if(!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), prop, &info)) \
    { \
        return ret; \
    } \
    offset = info.actual_offset; \
    pProp = info.prop; \
    bit_count = pProp->m_nBits; \
     \
    switch(pProp->GetType())\
    { \
        case type: \
        { \
            if(element != 0) \
            { \
                return ret; \
            } \
            break; \
        } \
        case DPT_DataTable: \
        { \
            FIND_PROP_SEND_IN_SENDTABLE(info, pProp, element, type, ret); \
             \
            offset += pProp->GetOffset(); \
            bit_count = pProp->m_nBits; \
            break; \
        } \
        default: \
        { \
            return ret; \
        } \
    }

#define FIND_PROP_SEND_IN_SENDTABLE(info, pProp, element, type, ret) \
    SendTable *pTable = pProp->GetDataTable(); \
    if(!pTable) \
    { \
        return ret; \
    } \
    int elementCount = pTable->GetNumProps(); \
    if(element < 0 || element >= elementCount) \
    { \
        return ret; \
    } \
    pProp = pTable->GetProp(element); \
    if(pProp->GetType() != type) \
    { \
        return ret; \
    }

int HL2::GetEntProp(cell_t entity, Prop_Types type, const char *prop, int size, int element)
{
    CBaseEntity *pEntity;
    int offset;
    edict_t *pEdict;
    int bit_count;
    bool is_unsigned = false;

    if(!IndexToAThings(entity, &pEntity, &pEdict))
    {
        return 0;
    }

    switch (type)
    {
        case Prop_Datas:
        {
            typedescription_t *td;

            FIND_PROP_DATA(td, 0);

            if((bit_count = MatchFildAsInteger(td->fieldType)) == 0)
            {
                return 0;
            }

            CHECK_SET_PROP_DATA_OFFSET(0);
            break;
        }
        case Prop_Sends:
        {
            FIND_PROP_SEND(DPT_Int, 0);
            is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);
            break;
        }
        default:
        {
            return 0;
        }
    }
    if(bit_count < 1)
    {
        bit_count = size * 8;
    }

    if(bit_count >= 17)
    {
        return *(int32_t *)((uint8_t *)pEntity + offset);
    }
    else if(bit_count >= 9)
    {
        if(is_unsigned)
        {
            return *(uint16_t *)((uint8_t *)pEntity + offset);
        }
        else
        {
            return *(int16_t *)((uint8_t *)pEntity + offset);
        }
    }
    else if(bit_count >= 2)
    {
        if(is_unsigned)
        {
            return *(uint8_t *)((uint8_t *)pEntity + offset);
        }
        else
        {
            return *(int8_t *)((uint8_t *)pEntity + offset);
        }
    }
    else
    {
        return *(bool *)((uint8_t *)pEntity + offset) ? 1 : 0;
    }
    return 0;
}

bool HL2::SetEntProp(cell_t entity, Prop_Types type, const char *prop, int value, int size, int element)
{
    CBaseEntity *pEntity;
    int offset;
    edict_t *pEdict;
    int bit_count;

    if(!IndexToAThings(entity, &pEntity, &pEdict))
    {
        return false;
    }

    switch(type)
    {
        case Prop_Datas:
        {
            typedescription_t *td;
            FIND_PROP_DATA(td, false);

            if((bit_count = MatchFildAsInteger(td->fieldType)) == 0)
            {
                return false;
            }

            CHECK_SET_PROP_DATA_OFFSET(false);
            break;
        }
        case Prop_Sends:
        {
            FIND_PROP_SEND(DPT_Int, false);
            break;
        }
        default:
        {
            return false;
        }
    }
    if(bit_count < 1)
    {
        bit_count = (size * 8);
    }

    if(bit_count >= 17)
    {
        *(int32_t *)((uint8_t *)pEntity + offset) = value;
    }
    else if(bit_count >= 9)
    {
        *(int16_t *)((uint8_t *)pEntity + offset) = (int16_t)value;
    }
    else if(bit_count >= 2)
    {
        *(int8_t *)((uint8_t *)pEntity + offset) = (int8_t)value;
    }
    else
    {
        *(bool *)((uint8_t *)pEntity + offset) = value ? true : false;
    }

    if(type == Prop_Sends && (pEdict != NULL))
    {
        gamehelpers->SetEdictStateChanged(pEdict, offset);
    }
    return true;
}

const bool HL2::IsNPC(const CBaseEntity *pEnt) const
{
    if(GetVirtualClass<ITerrorPlayer>((CBaseEntity*)pEnt)->IsNPC())
    {
        return true;
    }

    if( reinterpret_cast<IBaseEntity *>((CBaseEntity *)pEnt)->ClassMatches("witch") || 
        reinterpret_cast<IBaseEntity *>((CBaseEntity *)pEnt)->ClassMatches("infected"))
    {
        return true;
    }
    return false;
}

Vector* HL2::GetEntPropVector(cell_t entity, Prop_Types type, const char *prop, int size, int element)
{
	CBaseEntity *pEntity;
	int offset;
	int bit_count;
	edict_t *pEdict;
	
	if (!IndexToAThings(entity, &pEntity, &pEdict))
	{
		return nullptr;
	}

	switch (type)
	{
	case Prop_Datas:
		{
			typedescription_t *td;
			FIND_PROP_DATA(td, nullptr);

			if (td->fieldType != FIELD_VECTOR && td->fieldType != FIELD_POSITION_VECTOR)
			{
				return nullptr;
			}

			CHECK_SET_PROP_DATA_OFFSET(nullptr);

			break;
		}
	case Prop_Sends:
		{
			FIND_PROP_SEND(DPT_Vector, nullptr);
			break;
		}
	default:
		{
			return nullptr;
		}
	}

	return (Vector *)((uint8_t *)pEntity + offset);
}

CTriggerTraceEnums::CTriggerTraceEnums(Ray_t *pRay, const CTakeDamageInfo &info, const Vector &dir, int contentsMask) : 
m_VecDir(dir), m_ContentsMask(contentsMask), m_pRay(pRay), m_info( info )
{
}

bool CTriggerTraceEnums::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    CBaseEntity *pEnt = g_HL2->GetBaseEntity( pHandleEntity->GetRefEHandle() );

    ICollideable *pCollide = reinterpret_cast<IServerUnknown *>(pEnt)->GetCollideable();
    if ( IsSolid(pCollide->GetSolid(), pCollide->GetSolidFlags()) )
        return true;

    g_pTarce->ClipRayToEntity( *m_pRay, m_ContentsMask, pHandleEntity, &tr );
    if (tr.fraction < 1.0f)
    {
        reinterpret_cast<IBaseEntity *>(pEnt)->DispatchTraceAttack(m_info, m_VecDir, &tr);
        g_CallHelper->ApplyMultiDamages();
    }

    return true;
}

CEntitySphereQuery_::CEntitySphereQuery_(const Vector &center, float radius, int flagMask)
{
    m_listIndex = 0;
    m_listCount = g_HL2->UTIL_EntityesInSphere(m_pList, ARRAYSIZE(m_pList), center, radius, flagMask);
}

CBaseEntity *CEntitySphereQuery_::GetCurrentEntity()
{
    if(m_listIndex < m_listCount)
    {
        return m_pList[m_listIndex];
    }
    return nullptr;
}

CFlaggedEntitiesEnum_::CFlaggedEntitiesEnum_(CBaseEntity **pList, int listMax, int flagMask)
{
    m_pList = pList;
    m_listMax = listMax;
    m_flagMask = flagMask;
    m_count = 0;
}

IterationRetval_t CFlaggedEntitiesEnum_::EnumElement(IHandleEntity *pHandleEntity)
{
    CBaseEntity *pEntity = g_HL2->GetBaseEntity(pHandleEntity->GetRefEHandle());
    if(pEntity)
    {
        if(m_flagMask && !(Prop_get_fFlags(pEntity) & m_flagMask))
            return ITERATION_CONTINUE;

        if(!AddToList(pEntity))
            return ITERATION_STOP;
    }
    return ITERATION_CONTINUE;
}

bool CFlaggedEntitiesEnum_::AddToList(CBaseEntity *pEntity)
{
    if ( m_count >= m_listMax )
    {
        return false;
    }
    m_pList[m_count] = pEntity;
    m_count++;
    return true;
}

#ifdef _NET_PROP_DUMP_
const char *GetDTTypeName(int type)
{
	switch (type)
	{
	case DPT_Int:
		{
			return "integer";
		}
	case DPT_Float:
		{
			return "float";
		}
	case DPT_Vector:
		{
			return "vector";
		}
	case DPT_String:
		{
			return "string";
		}
	case DPT_Array:
		{
			return "array";
		}
	case DPT_DataTable:
		{
			return "datatable";
		}
	default:
		{
			return NULL;
		}
	}

	return NULL;
}

char *UTIL_SendFlagsToString(int flags, int type)
{
	static char str[1024];
	str[0] = 0;

	if (flags & SPROP_UNSIGNED)
	{
		strcat(str, "Unsigned|");
	}
	if (flags & SPROP_COORD)
	{
		strcat(str, "Coord|");
	}
	if (flags & SPROP_NOSCALE)
	{
		strcat(str, "NoScale|");
	}
	if (flags & SPROP_ROUNDDOWN)
	{
		strcat(str, "RoundDown|");
	}
	if (flags & SPROP_ROUNDUP)
	{
		strcat(str, "RoundUp|");
	}
	if (flags & SPROP_NORMAL)
	{
		if (type == DPT_Int)
		{
			strcat(str, "VarInt|");
		}
		else
		{
			strcat(str, "Normal|");
		}
	}
	if (flags & SPROP_EXCLUDE)
	{
		strcat(str, "Exclude|");
	}
	if (flags & SPROP_XYZE)
	{
		strcat(str, "XYZE|");
	}
	if (flags & SPROP_INSIDEARRAY)
	{
		strcat(str, "InsideArray|");
	}
	if (flags & SPROP_PROXY_ALWAYS_YES)
	{
		strcat(str, "AlwaysProxy|");
	}
	if (flags & SPROP_CHANGES_OFTEN)
	{
		strcat(str, "ChangesOften|");
	}
	if (flags & SPROP_IS_A_VECTOR_ELEM)
	{
		strcat(str, "VectorElem|");
	}
	if (flags & SPROP_COLLAPSIBLE)
	{
		strcat(str, "Collapsible|");
	}
	if (flags & SPROP_COORD_MP)
	{
		strcat(str, "CoordMP|");
	}
	if (flags & SPROP_COORD_MP_LOWPRECISION)
	{
		strcat(str, "CoordMPLowPrec|");
	}
	if (flags & SPROP_COORD_MP_INTEGRAL)
	{
		strcat(str, "CoordMpIntegral|");
	}

	int len = strlen(str) - 1;
	if (len > 0)
	{
		str[len] = 0;
	}

	return str;
}

void UTIL_DrawSendTable(FILE *fp, SendTable *pTable, int level)
{
	SendProp *pProp;
	const char *type;

	for (int i = 0; i < pTable->GetNumProps(); i++)
	{
		pProp = pTable->GetProp(i);
		if (pProp->GetDataTable())
		{
            for(int i = 0; i < level; i++)
            {
                fprintf(fp, "\t");
            }
			fprintf(fp, "Table: %s (offset %d) (type %s){\n", pProp->GetName(), pProp->GetOffset(), pProp->GetDataTable()->GetName());
			UTIL_DrawSendTable(fp, pProp->GetDataTable(), level + 1);
            for(int i = 0; i < level; i++)
            {
                fprintf(fp, "\t");
            }
            fprintf(fp, "}\n");
		}
		else
		{
			type = GetDTTypeName(pProp->GetType());

			if (type != NULL)
			{
                for(int i = 0; i < level; i++)
                {
                    fprintf(fp, "\t");
                }
				fprintf(fp, "Member: %-30s (offset %-5d) (type %-11s) (bits %-3d) (%s)\n", pProp->GetName(), pProp->GetOffset(), type, pProp->m_nBits, UTIL_SendFlagsToString(pProp->GetFlags(), pProp->GetType()));
			}
			else
			{
                for(int i = 0; i < level; i++)
                {
                    fprintf(fp, "\t");
                }
				fprintf(fp, "Member: %-30s (offset %-5d) (type %-11d) (bits %-3d) (%s)\n", pProp->GetName(), pProp->GetOffset(), pProp->GetType(), pProp->m_nBits, UTIL_SendFlagsToString(pProp->GetFlags(), pProp->GetType()));
			}
		}
	}
}

CON_COMMAND(sm_ukr_dump_netprops, "Dumps the networkable property table as a text file")
{
	if (args.ArgC() < 2)
	{
		META_CONPRINT("Usage: sm_dump_netprops <file>\n");
		return;
	}

	const char *file = args.Arg(1);
	if (!file || file[0] == '\0')
	{
		META_CONPRINT("Usage: sm_dump_netprops <file>\n");
		return;
	}

	char path[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_Game, path, sizeof(path), "%s", file);

	FILE *fp = NULL;
	if ((fp = fopen(path, "wt")) == NULL)
	{
		META_CONPRINTF("Could not open file \"%s\"\n", path);
		return;
	}

	fprintf(fp, "// Dump of all network properties for \"%s\" follows\n//\n\n", g_pSM->GetGameFolderName());

	ServerClass *pBase = gamedll->GetAllServerClasses();
	while (pBase != NULL)
	{
		fprintf(fp, "\n%s (type %s){\n", pBase->GetName(), pBase->m_pTable->GetName());
		UTIL_DrawSendTable(fp, pBase->m_pTable, 1);
        fprintf(fp, "}\n");
		pBase = pBase->m_pNext;
	}

	fclose(fp);

    META_CONPRINTF("File dump is create: %s\n", path);
}

#endif
