#include "HL2.h"

inline void SetParam(PassInfo *pParam, unsigned int size, unsigned int flags = PASSFLAG_BYVAL, PassType type = PassType_Basic)
{
    pParam->flags = flags; pParam->size = size, pParam->type = type;
}

#define STRINGIFY(string) #string

#define IS_SETUP(CallHelper) \
    if(CallHelper.IsSetup()) \
    {   \
        return CallHelper.IsSupported(); \
    }

#define HELPER_VALID(CallHelper) \
        if(CallHelper != nullptr) \
        { \
            CallHelper.MakeSuported(); \
        }

#define BEGIN_SETUP(CallHelper, signature) \
    IS_SETUP(CallHelper) \
    void *addr = nullptr; \
    if(!g_pGameConf->GetMemSig(STRINGIFY(signature), &addr) && addr == nullptr) \
    { \
        m_sLog->LogToFileEx(false, "addres is null for signature %s", #signature); \
    } \
    else \
    {
    

#define END_SETUP(CallHelper, CallConv, result, param, count) \
        CallHelper = g_pBinTools->CreateCall(addr, CallConv, result, param, count); \
        HELPER_VALID(CallHelper) \
    } \
    CallHelper.MakeSetup(); \
    return CallHelper.IsSupported(); \


bool HL2::SetupSpawnITMob()
{
    IS_SETUP(s_SpawnITMob)

    void *addr = nullptr;
    if(g_pGameConf->GetMemSig("ZombieManager_SpawnITMob", &addr) && addr != nullptr)
    {
        PassInfo param;
        SetParam(&param, sizeof(int));

        s_SpawnITMob = g_pBinTools->CreateCall(addr, CallConv_ThisCall, nullptr, &param, 1);
        if(s_SpawnITMob != nullptr)
        {
            if(g_pZombiMeneger == nullptr)
            {
                g_pGameConf->GetAddress("ZombieManager", &g_pZombiMeneger);
            }
            s_SpawnITMob.MakeSuported();
        }
    }

    s_SpawnITMob.MakeSetup();
    return s_SpawnITMob.IsSupported();
}

bool HL2::SetupOnMobRash()
{
    IS_SETUP(s_OnMobRash)

    void *addr = nullptr;
    if(g_pGameConf->GetMemSig("CDirector_OnMobRushStart", &addr) && addr != nullptr)
    {
        s_OnMobRash = g_pBinTools->CreateCall(addr, CallConv_ThisCall, nullptr, nullptr, 0);
        if(s_OnMobRash != nullptr)
        {
            if(g_pDirector == nullptr)
            {
                g_pGameConf->GetAddress("CDirector", &g_pDirector);
            }
            if(g_pDirector != nullptr)
            {
                s_OnMobRash.MakeSuported();
            }
        }
    }
    s_OnMobRash.MakeSetup();
    return s_OnMobRash.IsSupported();
}

bool HL2::SetupHasAnySurvivorLeftSafeArea()
{
    IS_SETUP(s_HasAnySurvivorLeftSafeArea)

    void *addr = nullptr;
    if(g_pGameConf->GetMemSig("CDirector_HasAnySurvivorLeftSafeArea", &addr) && addr != nullptr)
    {
        PassInfo ret;
        SetParam(&ret, sizeof(int));

        s_HasAnySurvivorLeftSafeArea = g_pBinTools->CreateCall(addr, CallConv_ThisCall, &ret, nullptr, 0);
        if(s_HasAnySurvivorLeftSafeArea != nullptr)
        {
            if(g_pDirector == nullptr)
            {
                g_pGameConf->GetAddress("CDirector", &g_pDirector);
            }
            if(g_pDirector != nullptr)
            {
                s_HasAnySurvivorLeftSafeArea.MakeSuported();
            }
        }
    }
    s_HasAnySurvivorLeftSafeArea.MakeSetup();
    return s_HasAnySurvivorLeftSafeArea.IsSupported();
}

bool HL2::SetupIsLiveSurvivorInside()
{
    BEGIN_SETUP(s_IsLiveSurvivorInside, CSurvivorRescue::IsLiveSurvivorInside)
        PassInfo ret;
        SetParam(&ret, sizeof(int));
    END_SETUP(s_IsLiveSurvivorInside, CallConv_ThisCall, &ret, nullptr, 0)
}
bool HL2::SetupVomitUpon()
{
    BEGIN_SETUP(s_VomitUpon, CTerrorPlayer_OnVomitedUpon)
		PassInfo info[2];
        SetParam(&info[0], sizeof(void *));
        SetParam(&info[1], sizeof(int32_t));
    END_SETUP(s_VomitUpon, CallConv_ThisCall, nullptr, info, 2)
}

bool HL2::SetupReserveLobby()
{
    BEGIN_SETUP(s_Reserver, GetReservationCookie)
        PassInfo ret;
        SetParam(&ret, sizeof(int));
    END_SETUP(s_Reserver, CallConv_ThisCall, &ret, nullptr, 0)
}

bool HL2::SetupUnReserveLobby()
{
    BEGIN_SETUP(s_UnReserver, SetReservationCookie)
        PassInfo s_info[3];
        SetParam(&s_info[0], sizeof(uint64_t));
        SetParam(&s_info[1], sizeof(const char *));
        SetParam(&s_info[2], sizeof(void *));
    END_SETUP(s_UnReserver, CallConv_ThisCall, NULL, s_info, 3)
}

bool HL2::SetupSetOrigin()
{
    BEGIN_SETUP(s_SetOrigin, UTIL_SetOrigin)
        PassInfo s_info[3];
        SetParam(&s_info[0], sizeof(void *));
        SetParam(&s_info[1], sizeof(Vector *));
        SetParam(&s_info[2], sizeof(bool));
    END_SETUP(s_SetOrigin, CallConv_Cdecl, nullptr, s_info, 3)
}

bool HL2::SetupGetSeqenceMoveYam()
{
    BEGIN_SETUP(s_GetSequenceMoveYam, GetSequenceMoveYam)
        PassInfo param, ret;
        SetParam(&param, sizeof(int));
        SetParam(&ret, sizeof(float), PASSFLAG_BYVAL, PassType_Float);
    END_SETUP(s_GetSequenceMoveYam, CallConv_ThisCall, &ret, &param, 1)
}

bool HL2::SetupAddStepDiscontinuity()
{
    BEGIN_SETUP(s_AddStepDiscon, AddStepDiscon)
        PassInfo param[3], result;
        SetParam(&param[0], sizeof(float), PASSFLAG_BYVAL, PassType_Float);
        SetParam(&param[1], sizeof(void *));
        SetParam(&param[2], sizeof(void *));
        SetParam(&result, sizeof(bool));
    END_SETUP(s_AddStepDiscon, CallConv_ThisCall, &result, param, 3)
}

bool HL2::SetupRemove()
{
    BEGIN_SETUP(s_UTIL_Remove, UTIL_Remove)
        PassInfo param;
        SetParam(&param, sizeof(void *));
    END_SETUP(s_UTIL_Remove, CallConv_Cdecl, nullptr, &param, 1)
}

bool HL2::SetupPrecacheGibsForModel()
{
    BEGIN_SETUP(s_PrecacheGibsForModel, PreGibsForModel)
        PassInfo param;
        SetParam(&param, sizeof(int));
    END_SETUP(s_PrecacheGibsForModel, CallConv_Cdecl, nullptr, &param, 1)
}

bool HL2::SetupCreateDataObjects()
{
    BEGIN_SETUP(s_CreateDataObj, CBaseEntity_CreateDataObject)
        PassInfo param, result;
        SetParam(&param, sizeof(int));
        SetParam(&result, sizeof(void *));
    END_SETUP(s_CreateDataObj, CallConv_ThisCall, &result, &param, 1)
}

bool HL2::SetupDestroyDataObjects()
{
    BEGIN_SETUP(s_DestroyDataObj, CBaseEntity_DestroyDataObject)
        PassInfo param;
        SetParam(&param, sizeof(int));
    END_SETUP(s_DestroyDataObj, CallConv_ThisCall, nullptr, &param, 1)
}

bool HL2::SetupSimThinkEntityChanged()
{
    BEGIN_SETUP(s_SimThinkEntityChanged, SimThinkEntityChanged)
        PassInfo param;
        SetParam(&param, sizeof(void *));
    END_SETUP(s_SimThinkEntityChanged, CallConv_Cdecl, nullptr, &param, 1)
}

bool HL2::SetupEntityTouchAdd()
{
    BEGIN_SETUP(s_EntityTouchAdd, EntityTouchAdd)
        PassInfo param;
        SetParam(&param, sizeof(void *));
    END_SETUP(s_EntityTouchAdd, CallConv_Cdecl, nullptr, &param, 1)
}

bool HL2::SetupEventListIndexForName()
{
    BEGIN_SETUP(s_EventListIndexForName, EventListIndexForName)
        PassInfo param, result;
        SetParam(&param, sizeof(const char *));
        SetParam(&result, sizeof(int));
    END_SETUP(s_EventListIndexForName, CallConv_Cdecl, &result, &param, 1)
}

bool HL2::SetupEventListRegisterPrivateEvent()
{
    BEGIN_SETUP(s_EventListRegisterPrivateEvent, EventListRegisterPrivateEvent)
        PassInfo result, param;
        SetParam(&param, sizeof(const char *));
        SetParam(&result, sizeof(int));
    END_SETUP(s_EventListRegisterPrivateEvent, CallConv_Cdecl, &result, &param, 1)
}

bool HL2::SetupEventListGetEventType()
{
    BEGIN_SETUP(s_EventListGetEventType, EventListGetEventType)
        PassInfo result, param;
        SetParam(&param, sizeof(int));
        SetParam(&result, sizeof(int));
    END_SETUP(s_EventListGetEventType, CallConv_Cdecl, &result, &param, 1)
}

bool HL2::SetupGetDataObject()
{
    BEGIN_SETUP(s_GetDataObject, CBaseEntity_GetDataObject)
        PassInfo param, result;
        SetParam(&param, sizeof(int));
        SetParam(&result, sizeof(void *));
    END_SETUP(s_GetDataObject, CallConv_ThisCall, &result, &param, 1)
}

bool HL2::SetupAddEntityToGroundList()
{
    BEGIN_SETUP(s_AddEntityToGroundList, CBaseEntity_AddEntityToGroundList)
        PassInfo param, result;
        SetParam(&param, sizeof(void *));
        SetParam(&result, sizeof(void *));
    END_SETUP(s_AddEntityToGroundList, CallConv_ThisCall, &result, &param, 1)
}

bool HL2::SetupPhysicsRemoveGround()
{
    BEGIN_SETUP(s_PhysicsRemoveGround, CBaseEntity_PhysicsRemoveGround)
        PassInfo param[2];
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(void *));
    END_SETUP(s_PhysicsRemoveGround, CallConv_Cdecl, nullptr, param, 2)
}

bool HL2::SetupReportEntityFlagsChanged()
{
    BEGIN_SETUP(s_ReportEntityFlagsChanged, CGlobalEntityList_ReportEntityFlagsChanged)
        PassInfo param[3];
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(unsigned int));
        SetParam(&param[2], sizeof(unsigned int));
    END_SETUP(s_ReportEntityFlagsChanged, CallConv_ThisCall, nullptr, param, 3)
}

bool HL2::SetupReportPositionChanged()
{
    BEGIN_SETUP(s_ReportPositionChanged, ReportPositionChanged)
        PassInfo param;
        SetParam(&param, sizeof(void *));
    END_SETUP(s_ReportPositionChanged, CallConv_Cdecl, nullptr, &param, 1)
}

bool HL2::SetupTransitionPlayerCount()
{
    BEGIN_SETUP(s_TransitionPlayerCount, TransitionedPlayerCount)
        PassInfo param[3];
        SetParam(&param[0], sizeof(int *));
        SetParam(&param[1], sizeof(int *));
        SetParam(&param[2], sizeof(int));
    END_SETUP(s_TransitionPlayerCount, CallConv_Cdecl, nullptr, param, 3)
}

bool HL2::SetupGetTheNextBots()
{
    BEGIN_SETUP(s_TheNextBot, GetTheNextBots)
        PassInfo result;
        SetParam(&result, sizeof(void *));
    END_SETUP(s_TheNextBot, CallConv_Cdecl, &result, nullptr, 0)
}

bool HL2::SetupSelect_Weighted_Sequence()
{
    BEGIN_SETUP(s_Select_Weighted_Sequence, Select_Weighted_Sequence)
        PassInfo param[3], result;
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(int));
        SetParam(&result, sizeof(int));
    END_SETUP(s_Select_Weighted_Sequence, CallConv_Cdecl, param, &result, 3)
}

bool HL2::SetupCBaseEntityEmitSound1()
{
    BEGIN_SETUP(s_CBaseEntity9EmitSound1, CBaseEntity_EmitSound_1)
        PassInfo param[3];
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(void*));
    END_SETUP(s_CBaseEntity9EmitSound1, CallConv_Cdecl, nullptr, param, 3)
}

bool HL2::SetupCBaseEntityEmitSound2()
{
    BEGIN_SETUP(s_CBaseEntity9EmitSound2, CBaseEntity_EmitSound_2)
        PassInfo param[4];
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(void*));
        SetParam(&param[3], sizeof(short));
    END_SETUP(s_CBaseEntity9EmitSound2, CallConv_Cdecl, nullptr, param, 4)
}

bool HL2::SetupCSoundEntInsertSound()
{
    BEGIN_SETUP(s_CSoundEntInsertSound, CSoundEnt_InsertSound)
        PassInfo param[7], result;
        SetParam(&result, sizeof(int));
        SetParam(&param[0], sizeof(int));
        SetParam(&param[1], sizeof(Vector *));
        SetParam(&param[2], sizeof(int));
        SetParam(&param[3], sizeof(float));
        SetParam(&param[4], sizeof(void *));
        SetParam(&param[5], sizeof(int));
        SetParam(&param[6], sizeof(void *));
    END_SETUP(s_CSoundEntInsertSound, CallConv_Cdecl, &result, param, 7)
}

bool HL2::SetupCResponseQueueAdd()
{
    BEGIN_SETUP(s_CResponseQueueAdd, CResponseQueue_Add)
        PassInfo param[5];
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(void*));
        SetParam(&param[2], sizeof(float));
        SetParam(&param[3], sizeof(void*));
        SetParam(&param[4], sizeof(void*));
    END_SETUP(s_CResponseQueueAdd, CallConv_ThisCall, nullptr, param, 5)
}

bool HL2::SetupIsBreakableEntity()
{
    BEGIN_SETUP(s_IsBreakableEntity, IsBreakableEntity)
        PassInfo param[3], result;
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(bool));
        SetParam(&param[2], sizeof(bool));
        SetParam(&result, sizeof(bool));
    END_SETUP(s_IsBreakableEntity, CallConv_Cdecl, &result, param, 3)
}

bool HL2::SetupDirectorIsVisibleToTeam()
{
    BEGIN_SETUP(s_DirectorIsVisibleToTeam, Director_IsVisibleToTeam)
        PassInfo param[6], result;
        SetParam(&result, sizeof(bool));
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(int));
        SetParam(&param[3], sizeof(float));
        SetParam(&param[4], sizeof(void*));
        SetParam(&param[5], sizeof(void*));
    END_SETUP(s_DirectorIsVisibleToTeam, CallConv_ThisCall, &result, param, 6)
}

bool HL2::SetupInfectedSetDamagedBodyGroupVariant()
{
    BEGIN_SETUP(s_SetDamagedBodyGroupVariant, Infected_SetDamagedBodyGroupVariant)
        PassInfo param[2];
        SetParam(&param[0], sizeof(const char*));
        SetParam(&param[1], sizeof(const char*));
    END_SETUP(s_SetDamagedBodyGroupVariant, CallConv_ThisCall, nullptr, param, 2)
}

bool HL2::SetupState_Transition()
{
    BEGIN_SETUP(s_State_Transition, CCSPlayer_State_Transition)
        PassInfo param;
        SetParam(&param, sizeof(int));
    END_SETUP(s_State_Transition, CallConv_ThisCall, nullptr, &param, 1)
}

bool HL2::SetupGetFileWeaponInfoFromHandlet()
{
    BEGIN_SETUP(s_GetFileWeaponInfoFromHandlet, GetFileWeaponInfoFromHandlet)
        PassInfo param, ret;
        SetParam(&param, sizeof(unsigned short));
        SetParam(&ret, sizeof(void *));
    END_SETUP(s_GetFileWeaponInfoFromHandlet, CallConv_Cdecl, &ret, &param, 1)
}

bool HL2::SetupMoveHelperServerv()
{
    BEGIN_SETUP(s_MoveHelperServerv, MoveHelperServerv)
        PassInfo ret;
        SetParam(&ret, sizeof(void*));
    END_SETUP(s_MoveHelperServerv, CallConv_Cdecl, &ret, nullptr, 0)
}

bool HL2::SetupPhysModelCreate()
{
    BEGIN_SETUP(s_PhysModelCreate, PhysModelCreate)
        PassInfo param[5], ret;
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(void*));
        SetParam(&param[3], sizeof(void*));
        SetParam(&param[4], sizeof(void*));
        SetParam(&ret, sizeof(void*));
    END_SETUP(s_PhysModelCreate, CallConv_Cdecl, &ret, param, 5)
}

bool HL2::SetupPhysModelCreateBox()
{
    BEGIN_SETUP(s_PhysModelCreateBox, PhysModelCreateBox)
        PassInfo param[5], ret;
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(void*));
        SetParam(&param[2], sizeof(void*));
        SetParam(&param[3], sizeof(void*));
        SetParam(&param[4], sizeof(bool));
        SetParam(&ret, sizeof(void*));
    END_SETUP(s_PhysModelCreateBox, CallConv_Cdecl, &ret, param, 5)
}

bool HL2::SetupPhysModelCreateOBB()
{
    BEGIN_SETUP(s_PhysModelCreateOBB, PhysModelCreateOBB)
        PassInfo param[6], ret;
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(void*));
        SetParam(&param[2], sizeof(void*));
        SetParam(&param[3], sizeof(void*));
        SetParam(&param[4], sizeof(void*));
        SetParam(&param[5], sizeof(bool));
        SetParam(&ret, sizeof(void*));
    END_SETUP(s_PhysModelCreateOBB, CallConv_Cdecl, &ret, param, 6)
}

bool HL2::SetupIsLineOfSightBetweenTwoEntitiesClear()
{
    BEGIN_SETUP(s_IsLineOfSightBetweenTwoEntitiesClear, IsLineOfSightBetweenTwoEntitiesClear)
        PassInfo param[9], ret;
        SetParam(&param[0], sizeof(void*));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(void*));
        SetParam(&param[3], sizeof(int));
        SetParam(&param[4], sizeof(void*));
        SetParam(&param[5], sizeof(int));
        SetParam(&param[6], sizeof(unsigned int));
        SetParam(&param[7], sizeof(ShouldHitFunc_t));
        SetParam(&param[8], sizeof(float));
        SetParam(&ret, sizeof(bool));
    END_SETUP(s_IsLineOfSightBetweenTwoEntitiesClear, CallConv_Cdecl, &ret, param, 9)
}

bool HL2::SetupDirector_IsTransitioned()
{
    BEGIN_SETUP(s_Director_IsTransitioned, Director_IsTransitionedEv)
        PassInfo ret;
        SetParam(&ret, sizeof(bool));
    END_SETUP(s_Director_IsTransitioned, CallConv_ThisCall, &ret, nullptr, 0)
}

bool HL2::SetupGetTransitionedLandmarkName()
{
    BEGIN_SETUP(s_GetTransitionedLandmarkName, GetTransitionedLandmarkName)
        PassInfo ret;
        SetParam(&ret, sizeof(char*));
    END_SETUP(s_GetTransitionedLandmarkName, CallConv_ThisCall, &ret, nullptr, 0)
}

bool HL2::SetupIsMotionControlledXY()
{
    BEGIN_SETUP(s_IsMotionControlledXY, IsMotionControlledXY)
        PassInfo ret, param;
        SetParam(&param, sizeof(int));
        SetParam(&ret, sizeof(bool));
    END_SETUP(s_IsMotionControlledXY, CallConv_ThisCall, &ret, &param, 1)
}

bool HL2::SetupIsMotionControlledZ()
{
    BEGIN_SETUP(s_IsMotionControlledZ, IsMotionControlledZ)
        PassInfo ret, param;
        SetParam(&param, sizeof(int));
        SetParam(&ret, sizeof(bool));
    END_SETUP(s_IsMotionControlledZ, CallConv_ThisCall, &ret, &param, 1)
}

bool HL2::SetupGetIndexForName()
{
    BEGIN_SETUP(s_ActivityList_IndexForName, ActivityList_IndexForName);
        PassInfo ret, param;
        SetParam(&param, sizeof(char*));
        SetParam(&ret, sizeof(int));
    END_SETUP(s_ActivityList_IndexForName, CallConv_Cdecl, &ret, &param, 1)
}

bool HL2::SetupRegisterPrivateActivity()
{
    BEGIN_SETUP(s_RegisterPrivateActivity, ActivityList_RegisterPrivateActivity)
        PassInfo ret, param;
        SetParam(&param, sizeof(char*));
        SetParam(&ret, sizeof(int));
    END_SETUP(s_RegisterPrivateActivity, CallConv_Cdecl, &ret, &param, 1)
}

bool HL2::SetupEventQueueAdd()
{
    BEGIN_SETUP(s_CEventQueue_AddEvent, EventQueueAddEvent)
        PassInfo param[7];
        SetParam(&param[0], sizeof(char*));
        SetParam(&param[1], sizeof(char*));
        SetParam(&param[2], sizeof(variant_t));
        SetParam(&param[3], sizeof(float));
        SetParam(&param[4], sizeof(void*));
        SetParam(&param[5], sizeof(void*));
        SetParam(&param[6], sizeof(int));
    END_SETUP(s_CEventQueue_AddEvent, CallConv_ThisCall, nullptr, param, 7)
}

bool HL2::SetupOnGSCLientApprove()
{
    BEGIN_SETUP(s_OnGSClientApprove, CSteam3Server_OnGSClientApprove)
        PassInfo param;
        SetParam(&param, sizeof(void*));
    END_SETUP(s_OnGSClientApprove, CallConv_ThisCall, nullptr, &param, 1)
}

bool HL2::SetupGetAmmoDef()
{
    BEGIN_SETUP(s_GetAmmoDef, GetAmmoDef)
        PassInfo result;
        SetParam(&result, sizeof(void *));
    END_SETUP(s_GetAmmoDef, CallConv_Cdecl, &result, nullptr, 0)
}

bool HL2::SetuppHysIsInCallback()
{
    BEGIN_SETUP(s_PhysIsInCallback, IsPhysIsInCallback)
        PassInfo result;
        SetParam(&result, sizeof(bool));
    END_SETUP(s_PhysIsInCallback, CallConv_Cdecl, &result, nullptr, 0)
}

bool HL2::SetupPhysCallBackDamage()
{
    BEGIN_SETUP(s_PhysCallbackDamage, PhysCallbackDamage)
        PassInfo param[2];
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(void *));
    END_SETUP(s_PhysCallbackDamage, CallConv_Cdecl, nullptr, param, 2)
}

bool HL2::SetupApplyMultiDamag()
{
    BEGIN_SETUP(s_ApplyMultiDamag, ApplyMultiDamag)
    END_SETUP(s_ApplyMultiDamag, CallConv_Cdecl, nullptr, nullptr, 0)
}

bool HL2::SetupClearMultiDamag()
{
    BEGIN_SETUP(s_ClearMultiDamag, ClearMultiDamag)
    END_SETUP(s_ClearMultiDamag, CallConv_Cdecl, nullptr, nullptr, 0)
}

bool HL2::SetupGetWorldEntity()
{
    BEGIN_SETUP(s_GetWorldEntity, GetWorldEnt)
        PassInfo ret;
        SetParam(&ret, sizeof(void *));
    END_SETUP(s_GetWorldEntity, CallConv_Cdecl, &ret, nullptr, 0)
}

bool HL2::SetupSetPoseParam()
{
    BEGIN_SETUP(s_SetPoseParam, BaseAnimation_SetPoseParam)
        PassInfo param[3], result;
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(int));
        SetParam(&param[2], sizeof(float), PASSFLAG_BYVAL, PassType_Float);
        SetParam(&result, sizeof(float), PASSFLAG_BYVAL, PassType_Float);
    END_SETUP(s_SetPoseParam, CallConv_ThisCall, &result, param, 3)
}

bool HL2::SetupSetSequence()
{
    BEGIN_SETUP(s_SetSequence, BaseAnimation_SetSequence)
        PassInfo param, result;
        SetParam(&param, sizeof(int));
        SetParam(&result, sizeof(int));
    END_SETUP(s_SetSequence, CallConv_ThisCall, &result, &param, 1)
}

bool HL2::SetupHasPoseParam()
{
    BEGIN_SETUP(s_HasPoseParam, BaseAnimation_HasPoseParam)
        PassInfo param[2], result;
        SetParam(&param[0], sizeof(int));
        SetParam(&param[1], sizeof(int));
        SetParam(&result,  sizeof(bool));
    END_SETUP(s_HasPoseParam, CallConv_ThisCall, &result, param, 2)
}

bool HL2::SetupLookupPoseParam()
{
    BEGIN_SETUP(s_LookupPoseParam, BaseAnimation_LookupPoseParameter)
        PassInfo param[2], result;
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(const char *));
        SetParam(&result, sizeof(int));
    END_SETUP(s_LookupPoseParam, CallConv_ThisCall, &result, param, 2)
}

bool HL2::SetupLockStudioHdr()
{
    BEGIN_SETUP(s_LockStudioHdr, BaseAnimation_LockStudioHdr)
    END_SETUP(s_LockStudioHdr, CallConv_ThisCall, nullptr, nullptr, 0)
}

bool HL2::SetupNPCPhysicsCreateSolver()
{
    BEGIN_SETUP(s_NPCPhysics_CreateSolver, NPCPhysics_CreateSolver)
        PassInfo param[4], result;
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(void *));
        SetParam(&param[2], sizeof(bool));
        SetParam(&param[3], sizeof(float), PASSFLAG_BYVAL, PassType_Float);
        SetParam(&result, sizeof(void *));
    END_SETUP(s_NPCPhysics_CreateSolver, CallConv_Cdecl, &result, param, 4)
}

bool HL2::SetupPhysicsTouchTrigger()
{
    BEGIN_SETUP(s_PhysicsTouchTriger, baseEntity_PhysicsTouchtrigger)
        PassInfo param;
        SetParam(&param, sizeof(void *));
    END_SETUP(s_PhysicsTouchTriger, CallConv_ThisCall, nullptr, &param, 1)
}

bool HL2::SetupFindEntityByName()
{
    BEGIN_SETUP(s_FindEntityByName, FindEntByName)
        PassInfo param[6], result;
        SetParam(&param[0], sizeof(void *));
        SetParam(&param[1], sizeof(void *));
        SetParam(&param[2], sizeof(void *));
        SetParam(&param[3], sizeof(void *));
        SetParam(&param[4], sizeof(void *));
        SetParam(&param[5], sizeof(void *));
        SetParam(&result, sizeof(void *));
    END_SETUP(s_FindEntityByName, CallConv_ThisCall, &result, param, 6)
}

bool HL2::SetupTakeOversBot()
{
    BEGIN_SETUP(s_TakeOver, TakeOverBot)
        PassInfo s_info;
        SetParam(&s_info, sizeof(bool));
    END_SETUP(s_TakeOver, CallConv_ThisCall, NULL, &s_info, 1)
}


bool HL2::SetupSetHumansSpec()
{
    BEGIN_SETUP(s_HumenSpec, SetHumanSpec)
        PassInfo s_info;
        SetParam(&s_info, sizeof(void *));
    END_SETUP(s_HumenSpec, CallConv_ThisCall, NULL, &s_info, 1)
}

bool HL2::SetupStargget()
{
    BEGIN_SETUP(s_Stargged, CTerrorPlayer_OnStaggered)
        PassInfo s_info[2];
        SetParam(&s_info[0], sizeof(void *));
        SetParam(&s_info[1], sizeof(void *));
    END_SETUP(s_Stargged, CallConv_ThisCall, NULL, s_info, 2)
}
