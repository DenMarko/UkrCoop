#ifndef _INCLUDE_HL2_H_
#define _INCLUDE_HL2_H_

#include "extension.h"
#include "log_messege.h"
#include "CTrace.h"
#include <dt_send.h>
#include <dt_recv.h>
#define GAME_DLL 1

#include <isaverestore.h>

#include <ehandle.h>
#include <server_class.h>

#include <shareddefs.h>
#include <takedamageinfo.h>
#include <variant_t.h>
#include <vphysics/object_hash.h>
#include <const.h>
#include "sdk/ai_speechconcept.h"
#include <imovehelper.h>

class VfuncEmptyClass {};
class IBaseEntity;
class INavMesh;

struct solid_t
{
	int		index;
	char	name[512];
	char	parent[512];
	char	surfaceprop[512];
	Vector	massCenterOverride;
	objectparams_t params;
};

enum
{
	SOUND_NONE				= 0,
	SOUND_COMBAT			= 0x00000001,
	SOUND_WORLD				= 0x00000002,
	SOUND_PLAYER			= 0x00000004,
	SOUND_DANGER			= 0x00000008,
	SOUND_BULLET_IMPACT		= 0x00000010,
	SOUND_CARCASS			= 0x00000020,
	SOUND_MEAT				= 0x00000040,
	SOUND_GARBAGE			= 0x00000080,
	SOUND_THUMPER			= 0x00000100, // keeps certain creatures at bay
	SOUND_BUGBAIT			= 0x00000200, // gets the antlion's attention
	SOUND_PHYSICS_DANGER	= 0x00000400,
	SOUND_DANGER_SNIPERONLY	= 0x00000800, // only scares the sniper NPC.
	SOUND_MOVE_AWAY			= 0x00001000,
	SOUND_PLAYER_VEHICLE	= 0x00002000,
	SOUND_READINESS_LOW		= 0x00004000, // Changes listener's readiness (Player Companion only)
	SOUND_READINESS_MEDIUM	= 0x00008000,
	SOUND_READINESS_HIGH	= 0x00010000,

	// Contexts begin here.
	SOUND_CONTEXT_FROM_SNIPER		= 0x00100000, // additional context for SOUND_DANGER
	SOUND_CONTEXT_GUNFIRE			= 0x00200000, // Added to SOUND_COMBAT
	SOUND_CONTEXT_MORTAR			= 0x00400000, // Explosion going to happen here.
	SOUND_CONTEXT_COMBINE_ONLY		= 0x00800000, // Only combine can hear sounds marked this way
	SOUND_CONTEXT_REACT_TO_SOURCE	= 0x01000000, // React to sound source's origin, not sound's location
	SOUND_CONTEXT_EXPLOSION			= 0x02000000, // Context added to SOUND_COMBAT, usually.
	SOUND_CONTEXT_EXCLUDE_COMBINE	= 0x04000000, // Combine do NOT hear this
	SOUND_CONTEXT_DANGER_APPROACH   = 0x08000000, // Treat as a normal danger sound if you see the source, otherwise turn to face source.
	SOUND_CONTEXT_ALLIES_ONLY		= 0x10000000, // Only player allies can hear this sound
	SOUND_CONTEXT_PLAYER_VEHICLE	= 0x20000000, // HACK: need this because we're not treating the SOUND_xxx values as true bit values! See switch in OnListened.

	ALL_CONTEXTS			= 0xFFF00000,

	ALL_SCENTS				= SOUND_CARCASS | SOUND_MEAT | SOUND_GARBAGE,

	ALL_SOUNDS				= 0x000FFFFF & ~ALL_SCENTS,

};

typedef void *(*ClientPutInServerOverrideFn)(edict_t*, const char*);

abstract_class IEntityFindFilter
{
public:
	virtual bool ShouldFindEntity( CBaseEntity *pEntity ) = 0;
	virtual CBaseEntity *GetFilterResult( void ) = 0;
};

class CTakeDamageInfoHack : public CTakeDamageInfo
{
public:
    CTakeDamageInfoHack();
	CTakeDamageInfoHack( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType, const Vector& vecDamagPos = vec3_origin, const Vector& vecDamagForce = vec3_origin);
	CTakeDamageInfoHack( IBaseEntity *pInflictor, IBaseEntity *pAttacker, float flDamage, int bitsDamageType, const Vector& vecDamagPos = vec3_origin, const Vector& vecDamagForce = vec3_origin);

    void Init(CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType, const Vector& vecDamagPos, const Vector& vecDamagForce);

    inline int GetAttacker() const { return m_hAttacker.IsValid() ? m_hAttacker.GetEntryIndex() : -1; }
	inline int GetInflictor() const { return m_hInflictor.IsValid() ? m_hInflictor.GetEntryIndex() : -1; }

    CTakeDamageInfoHack &operator = (const CTakeDamageInfoHack &other)
    {
        m_hInflictor = other.m_hInflictor;
        m_hAttacker = other.m_hAttacker;
        m_flDamage = other.m_flDamage;
        m_flBaseDamage = other.m_flBaseDamage;
        m_bitsDamageType = other.m_bitsDamageType;
        m_flMaxDamage = other.m_flMaxDamage;
        m_vecDamageForce = other.m_vecDamageForce;
        m_vecDamagePosition = other.m_vecDamagePosition;
        m_vecReportedPosition = other.m_vecReportedPosition;
        m_iAmmoType = other.m_iAmmoType;
        m_iDamageCustom = other.m_iDamageCustom;
        
        return *this;
    }

    CTakeDamageInfoHack &operator = (const CTakeDamageInfo &other)
    {
        m_hInflictor = other.GetInflictor();
        m_hAttacker = other.GetAttacker();
        m_flDamage = other.GetDamage();
        m_flBaseDamage = other.GetBaseDamage();
        m_bitsDamageType = other.GetDamageType();
        m_flMaxDamage = other.GetMaxDamage();
        m_vecDamageForce = other.GetDamageForce();
        m_vecDamagePosition = other.GetDamagePosition();
        m_vecReportedPosition = other.GetReportedPosition();
        m_iAmmoType = other.GetAmmoType();
        m_iDamageCustom = other.GetDamageCustom();

        return *this;
    }
};

typedef enum {
	HINTTEXT = 1,
	CONSOLE,
	CHAT,
	CENTER
}DEST;

enum Prop_Types
{
    Prop_Sends = 0,
    Prop_Datas
};

namespace VCaller
{
    template <typename T, typename...Rest>
    class ArgcBuffer {
    public:
        ArgcBuffer(const T& t, const Rest&... rest) {
            unsigned char *ptr = buff;
            buildbuffer(&ptr, t, rest...);
        }

        operator void*() { return buff; }
        operator unsigned char*() { return buff; }

        constexpr int size() const {
            return sizeof(buff);
        }

    private:
        template <typename K>
        constexpr static int sizetypes() {
            return sizeof(K);
        }
        template <typename K, typename K2, typename... Kn>
        constexpr static int sizetypes() {
            return sizeof(K) + sizetypes<K2, Kn...>();
        }

        template <typename K>
        void buildbuffer(unsigned char **ptr, K& k) {
            memcpy(*ptr, &k, sizeof(k));
            *ptr += sizeof(K);
        }

        template <typename K, typename... Kn>
        void buildbuffer(unsigned char **ptr, K& k, Kn&... kn) {
            buildbuffer(ptr, k);
            if (sizeof...(kn)!=0)
                buildbuffer(ptr, kn...);
        }

    private:
        unsigned char buff[sizetypes<T, Rest...>()];
    };

    class CCallHelper
    {
    public:
        CCallHelper()
        {
            call = nullptr;
            supported = false;
            setup = false;
        }
        
        ~CCallHelper()
        {}

        void Shutdown()
        {
            if (call)
            {
                call->Destroy();
                call = nullptr;
                supported = false;
            }
        }

        CCallHelper operator = (ICallWrapper *other)
        {
            this->call = other;
            return *this;
        }

        bool operator == (const ICallWrapper *other) const
        {
            return (this->call == other);
        }
        bool operator != (const ICallWrapper *other) const
        {
            return (this->call != other);
        }

        ICallWrapper *operator ->()
        {
            return this->call;
        }
        const ICallWrapper *operator ->() const
        {
            return this->call;
        }

        bool IsSupported() const
        {
            return supported;
        }
        bool IsSetup() const
        {
            return setup;
        }
        void MakeSuported()
        {
            supported = true;
        }
        void MakeSetup()
        {
            setup = true;
        }

    private:
        ICallWrapper *call;
        bool supported;
        bool setup;
    };
}
class CTriggerTraceEnums : public IEntityEnumerator
{
public:
    CTriggerTraceEnums( Ray_t *pRay, const CTakeDamageInfo &info, const Vector &dir, int contentsMask );
    virtual bool EnumEntity( IHandleEntity *pHandleEntity );
private:
    Vector m_VecDir;
    int m_ContentsMask;
    Ray_t *m_pRay;
    CTakeDamageInfo m_info;
};

#define MAX_SPHERE_QUERY	512
class CEntitySphereQuery_
{
public:
    CEntitySphereQuery_( const Vector &center, float radius, int flagMask = 0 );    
    CBaseEntity *GetCurrentEntity();

    inline void NextEntity() { m_listIndex++; }

private:
    int			m_listIndex;
    int			m_listCount;
    CBaseEntity *m_pList[MAX_SPHERE_QUERY];
};

class CFlaggedEntitiesEnum_ : public IPartitionEnumerator
{
public:
    CFlaggedEntitiesEnum_( CBaseEntity **pList, int listMax, int flagMask );
    virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity );
    int GetCount() { return m_count; }
    bool AddToList( CBaseEntity *pEntity );
    
private:
    CBaseEntity		**m_pList;
    int				m_listMax;
    int				m_flagMask;
    int				m_count;
};

enum DeferredResponseTarget_t
{
	kDRT_ANY,
	kDRT_ALL,
	kDRT_SPECIFIC,

	kDRT_MAX,
};

enum EQueryType_t
{
	EQUERY_INVALID = 0,									// an invalid or unused entry
	EQUERY_TRACELINE,
	EQUERY_ENTITY_LOS_CHECK,

};

enum EEntityOffsetMode_t
{
	EOFFSET_MODE_WORLDSPACE_CENTER,
	EOFFSET_MODE_EYEPOSITION,
	EOFFSET_MODE_NONE,										// nop
};

struct CFollowupTargetSpec_t
{
    DeferredResponseTarget_t m_iTargetType;
    EHANDLE	m_hHandle;
};

class IMoveHelperServer : public IMoveHelper
{
public:
    virtual void SetHost( class IBasePlayer* host ) = 0;
};

class ITerrorGameRules;
class IBaseAnimating;
class ITeam;
class INavArea;

abstract_class ICallHellpers
{
public:
    virtual void takeOversBot(CBaseEntity *pEntity, bool switchs) = 0;
    virtual void setHumansSpec(CBaseEntity *bEntity, CBaseEntity *pEntity) = 0;
    virtual void RadiusDamage(const CTakeDamageInfo& info, const Vector &vecSrc, float flRadius, int iClassIgnore, bool bIgnoreWorld) = 0;
    virtual int IsLiveSurInside(CBaseEntity *gRescue) = 0;
    virtual float GetSequenceMoveYam(CBaseEntity*, int) = 0;
    virtual bool AddStepDiscontinuity(CBaseEntity *, float, Vector*, QAngle*) = 0;
    virtual void PhysicsTouchTriggers(const CBaseEntity *pThisPtr, const Vector *pPrevAbsOrigin) = 0;
    virtual CBaseEntity *NPCPhysicsCreateSolver(CBaseEntity *pNPC, CBaseEntity* pPhysicsObject, bool disableCollisions, float separationDuration) = 0;
    virtual void LockStudioHdr(IBaseAnimating *pThisPtr) = 0;
    virtual bool HasPoseParam(CBaseEntity *, int iSeqence, int iParam) = 0;
    virtual void UTIL_Remove(void *) = 0;
    virtual void *GetAmmoDef(void) = 0;
    virtual bool IsPhysIsInCallback() = 0;
    virtual void PhysCallBackDamage(CBaseEntity *pEnt, const CTakeDamageInfo &info) = 0;
    virtual void ApplyMultiDamages(void) = 0;
    virtual void ClearMultiDamages(void) = 0;
    virtual void *GetWorldEnt(void) = 0;
    virtual void OnGSClientApprove(void *pThisPtr, void* pGSClientApprove) = 0;
    virtual void PrecacheGibsForModel(int iModel) = 0;
    virtual void *CreateDataObjects(CBaseEntity *pEnt, int type) = 0;
    virtual void DestroyDataObjects(CBaseEntity *pEnt, int type) = 0;
    virtual void *GetDataObject(CBaseEntity* pEnt, int type) = 0;
    virtual void PhysicsRemoveGround(void *other, void *link) = 0;
    virtual void SimThink_EntityChanged(CBaseEntity *pEnt) = 0;
    virtual void EntityTouchAdd(CBaseEntity *) = 0;
    virtual int EventListIndexForName(const char*) = 0;
    virtual int EventListRegisterPrivateEvent(const char*) = 0;
    virtual int EventListGetEventType(int) = 0;
    virtual void *AddEntityToGroundList(IBaseEntity* , IBaseEntity*) = 0;
    virtual void ReportEntityFlagsChanged(CBaseEntity*, unsigned int flagsOld, unsigned int flagsNew) = 0;
    virtual void ReportPositionChanged(CBaseEntity* pEnt) = 0;
    virtual void TransitionPlayerCount(int *v1, int *v2, int team) = 0;
    virtual void *GetTheNextBots() = 0;
    virtual int Select_Weighted_Sequence(void* pModel, int, int) = 0;
    virtual void CBaseEntity_EmitSound(IRecipientFilter& filter, int iEntIndex, const EmitSound_t& params) = 0;
    virtual void CBaseEntity_EmitSound(IRecipientFilter& filter, int iEntIndex, const EmitSound_t& params, HSOUNDSCRIPTHANDLE& handle) = 0;
    virtual int CSoundEnt_InsertSound(int iType, const Vector& vecOrigion, int iVolume, float flDyration, CBaseEntity* pOwner = nullptr, int soundChannelIndex = 0, CBaseEntity* pSoundTarget = NULL) = 0;
    virtual void CResponseQueue_Add(const CAI_Concept& concepts, const void* context, float time, const CFollowupTargetSpec_t& targetspec, void *pIssuer) = 0;
    virtual bool IsBreakableEntity(CBaseEntity*, bool, bool) = 0;
    virtual string_t AllocPooledString(const char* name) = 0;
    virtual void CCSPlayer_State_Transition(void*, int) = 0;
    virtual bool Director_IsVisibleToTeam(const Vector& vec, int team, int val1 = 0, float val2 = 0.f, INavArea* area = nullptr, const CBaseEntity* pEntity = nullptr) = 0;
    virtual bool Director_IsVisibleToTeam(const IBaseEntity* pEntity, int team, int val1 = 0, float val2 = 0.f, INavArea* area = nullptr) = 0;
    virtual bool Director_IsTransitioned() = 0;
    virtual int GetHasAnySurvivorLeftSafeArea() = 0;
    virtual void OnSpawnITMob(int mobSize) = 0;
    virtual void OnMobRash(void) = 0;
    virtual IBaseEntity* FindEntityByClassName(IBaseEntity* pEnt, const char *nameCalss) = 0;
    virtual IBaseEntity* FindEntityByName(IBaseEntity* pStartEntity, const char* szName, IBaseEntity* pSearchEntity = nullptr, IBaseEntity *pActivator = nullptr, IBaseEntity *pCaller = nullptr, IEntityFindFilter *pFilter = nullptr) = 0;
    virtual IBaseEntity* FindEntityGeneric( IBaseEntity *pStartEntity, const char *szName, IBaseEntity *pSearchingEntity = nullptr, IBaseEntity *pActivator = nullptr, IBaseEntity *pCaller = nullptr ) = 0;
    virtual IBaseEntity* FindEntityByClassNameNearest(const char*zsName, const Vector& vecSrc, float flRadius) = 0;
    virtual void UnReserveLobby() = 0;
    virtual int64_t GetReserveLobby() = 0;
    virtual void PlayerVomitUpon(CBaseEntity *pEntity, CBaseEntity *aEntity, cell_t params) = 0;
    virtual void *GetFileWeaponInfoFromHandlet(unsigned short handle) = 0;
    virtual IMoveHelperServer *MoveHelperServerv() = 0;
    virtual IPhysicsObject *PhysModelCreate(IBaseEntity* pEnt, int modelIndex, const Vector *origin, const QAngle *angles, solid_t *pSolid = nullptr) = 0;
    virtual IPhysicsObject *PhysModelCreateBox(IBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, bool isStatic) = 0;
    virtual IPhysicsObject *PhysModelCreateOBB(IBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, const QAngle &angle, bool isStatic) = 0;
    virtual bool IsLineOfSightBetweenTwoEntitiesClear(IBaseEntity*pSrcEntity, EEntityOffsetMode_t nSrcOffsetMode, IBaseEntity *pDestEntity, EEntityOffsetMode_t nDestOffsetMode, IBaseEntity *pSkipEntity, int nCollisionGroup, unsigned int nTraceMask, ShouldHitFunc_t pTraceFilterCallback, float flMinimumUpdateInterval = 0.2 ) = 0;
    virtual char* GetTransitionedLandmarkName() = 0;
    virtual bool IsMotionControlledXY( IBaseEntity* pThis, Activity activity ) = 0;
    virtual bool IsMotionControlledZ( IBaseEntity* pThis, Activity activity ) = 0;
    virtual Activity GetIndexForName(const char *pszActivityName) = 0;
    virtual Activity RegisterPrivateActivity(const char* pszActivityName) = 0;
    virtual void CEventQueueAdd(const char* target, const char* targetInput, variant_t Value, float fireDelay, IBaseEntity* pActivator, IBaseEntity* pCaller, int outputID) = 0;
};

class HL2 : public ICallHellpers
{
public:
    CBaseEntity* GetBaseEntity(CBaseHandle handle);
    
    void *g_pMasterMarker;
    void *g_pNextID;
    void *g_pHidingSpotmasterMarker;
    void *g_pOpenList;
    void *g_pOpenListTail;
    void *g_pClientPutInServerOverride;
    void *g_pBasePlayer_PlayerEdict;
    void *g_pSelectedZombieSpawn;
    void *g_pGameStats;
    void *g_pEventQueue;
    void *g_pManualSpawn;
    void *g_pSerchMarker;
private:

    void *g_pDirector;
    void *g_pZombiMeneger;
    void *g_pCollisionPaintHash;
    void *g_pPhysicsProps;
    void *g_pTeamGlobal;
    void *g_pPhysicsCollision;
    void *g_pDirtyKDTree;

    VCaller::CCallHelper s_TakeOver;
    VCaller::CCallHelper s_HumenSpec;
    VCaller::CCallHelper s_UnReserver;
    VCaller::CCallHelper s_Reserver;
    VCaller::CCallHelper s_VomitUpon;
    VCaller::CCallHelper s_IsLiveSurvivorInside;
    VCaller::CCallHelper s_HasAnySurvivorLeftSafeArea;
    VCaller::CCallHelper s_OnMobRash;
    VCaller::CCallHelper s_SpawnITMob;
    VCaller::CCallHelper s_GetSequenceMoveYam;
    VCaller::CCallHelper s_AddStepDiscon;
    VCaller::CCallHelper s_FindEntityByName; //_ZN17CGlobalEntityList16FindEntityByNameEP11CBaseEntityPKcS1_S1_S1_P17IEntityFindFilter
    VCaller::CCallHelper s_PhysicsTouchTriger; //_ZN11CBaseEntity20PhysicsTouchTriggersEPK6Vector
    VCaller::CCallHelper s_NPCPhysics_CreateSolver; //_Z23NPCPhysics_CreateSolverP11CAI_BaseNPCP11CBaseEntitybf
    VCaller::CCallHelper s_LockStudioHdr; //_ZN14CBaseAnimating13LockStudioHdrEv
    VCaller::CCallHelper s_HasPoseParam; //BaseAnimation_HasPoseParam
    VCaller::CCallHelper s_UTIL_Remove; //UTIL_Remove
    VCaller::CCallHelper s_GetAmmoDef;
    VCaller::CCallHelper s_PhysIsInCallback;
    VCaller::CCallHelper s_PhysCallbackDamage;
    VCaller::CCallHelper s_ApplyMultiDamag;
    VCaller::CCallHelper s_ClearMultiDamag;
    VCaller::CCallHelper s_GetWorldEntity;
    VCaller::CCallHelper s_OnGSClientApprove;
    VCaller::CCallHelper s_PrecacheGibsForModel;
    VCaller::CCallHelper s_CreateDataObj;
    VCaller::CCallHelper s_DestroyDataObj;
    VCaller::CCallHelper s_SimThinkEntityChanged;
    VCaller::CCallHelper s_EntityTouchAdd;
    VCaller::CCallHelper s_EventListIndexForName;
    VCaller::CCallHelper s_EventListRegisterPrivateEvent;
    VCaller::CCallHelper s_EventListGetEventType;
    VCaller::CCallHelper s_GetDataObject;
    VCaller::CCallHelper s_AddEntityToGroundList;
    VCaller::CCallHelper s_PhysicsRemoveGround;
    VCaller::CCallHelper s_ReportEntityFlagsChanged;
    VCaller::CCallHelper s_ReportPositionChanged;
    VCaller::CCallHelper s_TransitionPlayerCount;
    VCaller::CCallHelper s_TheNextBot;
    VCaller::CCallHelper s_Select_Weighted_Sequence;
    VCaller::CCallHelper s_CBaseEntity9EmitSound1;
    VCaller::CCallHelper s_CBaseEntity9EmitSound2;
    VCaller::CCallHelper s_CSoundEntInsertSound;
    VCaller::CCallHelper s_CResponseQueueAdd;
    VCaller::CCallHelper s_IsBreakableEntity;
    VCaller::CCallHelper s_DirectorIsVisibleToTeam;
    VCaller::CCallHelper s_State_Transition;
    VCaller::CCallHelper s_GetFileWeaponInfoFromHandlet;
    VCaller::CCallHelper s_MoveHelperServerv;
    VCaller::CCallHelper s_PhysModelCreate;
    VCaller::CCallHelper s_PhysModelCreateBox;
    VCaller::CCallHelper s_PhysModelCreateOBB;
    VCaller::CCallHelper s_IsLineOfSightBetweenTwoEntitiesClear;
    VCaller::CCallHelper s_Director_IsTransitioned;
    VCaller::CCallHelper s_GetTransitionedLandmarkName;
    VCaller::CCallHelper s_IsMotionControlledXY;
    VCaller::CCallHelper s_IsMotionControlledZ;
    VCaller::CCallHelper s_ActivityList_IndexForName;
    VCaller::CCallHelper s_RegisterPrivateActivity;
    VCaller::CCallHelper s_CEventQueue_AddEvent;

public:
    HL2();
    ~HL2();

    virtual void takeOversBot(CBaseEntity *pEntity, bool switchs);
    virtual void setHumansSpec(CBaseEntity *bEntity, CBaseEntity *pEntity);

    IPhysicsObjectPairHash *GetCollisionHash();
    IPhysicsSurfaceProps *GetSurfaceProps();
    IPhysicsCollision *GetPhysicsCollision();

    unsigned int *GetNavArea_masterMarker();
    unsigned int *GetHidingSpot_nextID();
    unsigned int *GetHidingSpot_masterMarker();
    unsigned int *GetNavMesh_GetNerestNavArea_SearchMarker();

    void *GetNavArea_openList();
    void *GetNavArea_openListTail();
    
    void ClientPutInServerOverride( ClientPutInServerOverrideFn fn );
    void Set_BasePlayer_PlayerEdict( edict_t *pEdict);
    void SetManualSpawn(bool );
    void SelectedZombieSpawn(void*);
    void *GetGameStats();
    void *CEventQueue();

    INavMesh *GetTheNavMesh();

    void *GetDirtyKDTree();
    void *GetVoteController();
    ITeam *GetGlobalTeam(int iIndex);
    void *GetDirector();

    void *GetAIConceptTable();
    void *GetResponseQueueManager();

    void ClearCollisionHash();
    ITerrorGameRules *GetGameRules()
    {
        return(ITerrorGameRules *)g_pSDKTools->GetGameRules();
    }

    void DamageRadius(CBaseEntity *pAttack, const Vector& vecSrc, float fRadius, float fDamag);
    inline void RadiusDamage(const CTakeDamageInfo& info, const Vector& vecSrc, float flRadius, int iClassIgnore, CBaseEntity* pEntityIgnore)
    {
        RadiusDamage(info, vecSrc, flRadius, iClassIgnore, false);
    }
    virtual void RadiusDamage(const CTakeDamageInfo& info, const Vector &vecSrc, float flRadius, int iClassIgnore, bool bIgnoreWorld);
    float GetAmountOfEntityVisible(Vector& vecSrc, IBaseEntity* pEntity);
    float GetExplosionDamageAdjustment(Vector& vecSrc, Vector &vecEnd, IBaseEntity *pEntToIgnore);
    void CalculateExplosiveDamageForce( CTakeDamageInfo *info, const Vector &vecDir, const Vector &vecForceOrigin, float flScale );

    virtual int IsLiveSurInside(CBaseEntity *gRescue);
    virtual float GetSequenceMoveYam(CBaseEntity*, int);
    virtual bool AddStepDiscontinuity(CBaseEntity *, float, Vector*, QAngle*);
    virtual IBaseEntity *FindEntityByName(IBaseEntity* pStartEntity, const char* szName, IBaseEntity* pSearchEntity = nullptr, IBaseEntity *pActivator = nullptr, IBaseEntity *pCaller = nullptr, IEntityFindFilter *pFilter = nullptr);
    virtual void PhysicsTouchTriggers(const CBaseEntity *pThisPtr, const Vector *pPrevAbsOrigin);
    virtual CBaseEntity *NPCPhysicsCreateSolver(CBaseEntity *pNPC, CBaseEntity* pPhysicsObject, bool disableCollisions, float separationDuration);
    virtual void LockStudioHdr(IBaseAnimating *pThisPtr);
    virtual bool HasPoseParam(CBaseEntity *, int iSeqence, int iParam);
    virtual void UTIL_Remove(void *);
    virtual void *GetAmmoDef();
    virtual bool IsPhysIsInCallback();
    virtual void PhysCallBackDamage(CBaseEntity *pEnt, const CTakeDamageInfo &info);
    virtual void ApplyMultiDamages(void);
    virtual void ClearMultiDamages(void);
    virtual void *GetWorldEnt();
    virtual void OnGSClientApprove(void *pThisPtr, void* pGSClientApprove);
    virtual void PrecacheGibsForModel(int iModel);
    virtual void *CreateDataObjects(CBaseEntity *pEnt, int type);
    virtual void DestroyDataObjects(CBaseEntity *pEnt, int type);
    virtual void *GetDataObject(CBaseEntity* pEnt, int type);
    virtual void PhysicsRemoveGround(void *other, void *link);
    virtual void SimThink_EntityChanged(CBaseEntity *pEnt);
    virtual void EntityTouchAdd(CBaseEntity *);
    virtual int EventListIndexForName(const char*);
    virtual int EventListRegisterPrivateEvent(const char*);
    virtual int EventListGetEventType(int);
    virtual void *AddEntityToGroundList(IBaseEntity* , IBaseEntity*);
    virtual void ReportEntityFlagsChanged(CBaseEntity*, unsigned int flagsOld, unsigned int flagsNew);
    virtual void ReportPositionChanged(CBaseEntity* pEnt);
    virtual void TransitionPlayerCount(int *v1, int *v2, int team);
    virtual void *GetTheNextBots();
    virtual int Select_Weighted_Sequence(void* pModel, int, int);
    virtual void CBaseEntity_EmitSound(IRecipientFilter& filter, int iEntIndex, const EmitSound_t& params);
    virtual void CBaseEntity_EmitSound(IRecipientFilter& filter, int iEntIndex, const EmitSound_t& params, HSOUNDSCRIPTHANDLE& handle);
    virtual int CSoundEnt_InsertSound(int iType, const Vector& vecOrigion, int iVolume, float flDyration, CBaseEntity* pOwner = nullptr, int soundChannelIndex = 0, CBaseEntity* pSoundTarget = NULL);
    virtual void CResponseQueue_Add(const CAI_Concept& concepts, const void* context, float time, const CFollowupTargetSpec_t& targetspec, void *pIssuer);
    virtual bool IsBreakableEntity(CBaseEntity*, bool, bool);
    virtual string_t AllocPooledString(const char* name);
    virtual void CCSPlayer_State_Transition(void*, int);
    
    virtual bool Director_IsVisibleToTeam(const Vector& vec, int team, int val1 = 0, float val2 = 0.f, INavArea* area = nullptr, const CBaseEntity* pEntity = nullptr);
    virtual bool Director_IsVisibleToTeam(const IBaseEntity* pEntity, int team, int val1, float val2, INavArea* area);
    virtual bool Director_IsTransitioned();

    virtual int GetHasAnySurvivorLeftSafeArea();
    virtual bool IsMotionControlledXY( IBaseEntity* pThis, Activity activity );
    virtual bool IsMotionControlledZ( IBaseEntity* pThis, Activity activity );

    virtual void OnSpawnITMob(int mobSize);
    virtual void OnMobRash();
    virtual IBaseEntity* FindEntityByClassName(IBaseEntity* pEnt, const char *nameCalss);
    virtual IBaseEntity* FindEntityByClassNameNearest(const char*zsName, const Vector& vecSrc, float flRadius);

    virtual void UnReserveLobby();
    virtual int64_t GetReserveLobby();

    virtual void PlayerVomitUpon(CBaseEntity *pEntity, CBaseEntity *aEntity, cell_t params);

    virtual IBaseEntity *FindEntityGeneric( IBaseEntity *pStartEntity, const char *szName, IBaseEntity *pSearchingEntity = nullptr, IBaseEntity *pActivator = nullptr, IBaseEntity *pCaller = nullptr );
    virtual void *GetFileWeaponInfoFromHandlet(unsigned short handle);
    virtual IMoveHelperServer *MoveHelperServerv();
    virtual IPhysicsObject *PhysModelCreate(IBaseEntity* pEnt, int modelIndex, const Vector *origin, const QAngle *angles, solid_t *pSolid = nullptr);
    virtual IPhysicsObject *PhysModelCreateBox(IBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, bool isStatic);
    virtual IPhysicsObject *PhysModelCreateOBB(IBaseEntity *pEntity, const Vector &mins, const Vector &maxs, const Vector &origin, const QAngle &angle, bool isStatic);
    virtual bool IsLineOfSightBetweenTwoEntitiesClear(IBaseEntity *pSrcEntity, EEntityOffsetMode_t nSrcOffsetMode, IBaseEntity *pDestEntity, EEntityOffsetMode_t nDestOffsetMode, IBaseEntity *pSkipEntity, int nCollisionGroup, unsigned int nTraceMask, ShouldHitFunc_t pTraceFilterCallback, float flMinimumUpdateInterval = 0.2 );
    virtual char* GetTransitionedLandmarkName();

    virtual Activity GetIndexForName(const char *pszActivityName);
    virtual Activity RegisterPrivateActivity(const char* pszActivityName);
    virtual void CEventQueueAdd(const char* target, const char* targetInput, variant_t Value, float fireDelay, IBaseEntity* pActivator, IBaseEntity* pCaller, int outputID);

	bool TextMsg(int client, DEST dest, const char *msg);
	bool Translate(char *buffer, size_t maxlength, const char *format, unsigned int numparams, size_t *pOutLength, ...);
	void PrintToConsole(edict_t *m_pEdict, const char* msg, ...);
	void PrintToConsole(int client, const char* msg, ...);
    
	int GetEntProp(cell_t entity, Prop_Types type, const char *prop, int size = 4, int element = 0);
    Vector* GetEntPropVector(cell_t entity, Prop_Types type, const char *prop, int size = 4, int element = 0);
	bool SetEntProp(cell_t entity, Prop_Types type, const char *prop, int value, int size = 4, int element = 0);
    const bool IsNPC(const CBaseEntity *pEnt) const;

public:
    edict_t *BaseEntityToEdict(const CBaseEntity *pEntity)
    {
        IServerUnknown *pUnk = (IServerUnknown *)pEntity;
        IServerNetworkable *pNet = pUnk->GetNetworkable();
        if(!pNet)
            return NULL;

        return pNet->GetEdict();
    }
    
    bool UTIL_ContainsDataTable(SendTable *pTable, const char *name);
    bool IndexToAThings(cell_t num, CBaseEntity **pEntData, edict_t **pEdictData);

    int UTIL_EntityInSphere(const Vector& center, float radius, CFlaggedEntitiesEnum_* pEnum);
    int UTIL_EntityesInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius, int flagMask);

private:
    bool SetupIsLiveSurvivorInside();
    bool SetupTakeOversBot();
    bool SetupSetHumansSpec();
    bool SetupUnReserveLobby();
    bool SetupReserveLobby();
    bool SetupVomitUpon();
    bool SetupHasAnySurvivorLeftSafeArea();
    bool SetupOnMobRash();
    bool SetupSpawnITMob();
    bool SetupGetSeqenceMoveYam();
    bool SetupAddStepDiscontinuity();
    bool SetupFindEntityByName();
    bool SetupPhysicsTouchTrigger();
    bool SetupNPCPhysicsCreateSolver();
    bool SetupLockStudioHdr();
    bool SetupHasPoseParam();
    bool SetupRemove();
    bool SetupGetAmmoDef();
    bool SetuppHysIsInCallback();
    bool SetupPhysCallBackDamage();
    bool SetupApplyMultiDamag();
    bool SetupClearMultiDamag();
    bool SetupGetWorldEntity();
    bool SetupOnGSCLientApprove();
    bool SetupPrecacheGibsForModel();
    bool SetupCreateDataObjects();
    bool SetupDestroyDataObjects();
    bool SetupSimThinkEntityChanged();
    bool SetupEntityTouchAdd();
    bool SetupEventListIndexForName();
    bool SetupEventListRegisterPrivateEvent();
    bool SetupEventListGetEventType();
    bool SetupGetDataObject();
    bool SetupAddEntityToGroundList();
    bool SetupPhysicsRemoveGround();
    bool SetupReportEntityFlagsChanged();
    bool SetupReportPositionChanged();
    bool SetupTransitionPlayerCount();
    bool SetupGetTheNextBots();
    bool SetupSelect_Weighted_Sequence();
    bool SetupCBaseEntityEmitSound1();
    bool SetupCBaseEntityEmitSound2();
    bool SetupCSoundEntInsertSound();
    bool SetupCResponseQueueAdd();
    bool SetupIsBreakableEntity();
    bool SetupDirectorIsVisibleToTeam();
    bool SetupState_Transition();
    bool SetupGetFileWeaponInfoFromHandlet();
    bool SetupMoveHelperServerv();
    bool SetupPhysModelCreate();
    bool SetupPhysModelCreateBox();
    bool SetupPhysModelCreateOBB();
    bool SetupIsLineOfSightBetweenTwoEntitiesClear();
    bool SetupDirector_IsTransitioned();
    bool SetupGetTransitionedLandmarkName();
    bool SetupIsMotionControlledXY();
    bool SetupIsMotionControlledZ();
    bool SetupGetIndexForName();
    bool SetupRegisterPrivateActivity();
    bool SetupEventQueueAdd();

    int MatchFildAsInteger(int field_type)
    {
        switch(field_type)
        {
            case FIELD_TICK:
            case FIELD_MODELINDEX:
            case FIELD_MATERIALINDEX:
            case FIELD_INTEGER:
            case FIELD_COLOR32:
                return 32;
            case FIELD_SHORT:
                return 16;
            case FIELD_CHARACTER:
                return 8;
            case FIELD_BOOLEAN:
                return 1;
            default:
                return 0;
        }
        return 0;
    }
};

extern HL2*             g_HL2;
extern ICallHellpers*   g_CallHelper;

#endif
