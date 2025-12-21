#include "extension.h"
#include "chat_log.h"
#include "log_messege.h"
#include "HL2.h"
#include "detours.h"
#include "../../sourcemod/core/logic/CellArray.h"
#include "LuaBridge/LuaBridge.h"
#include "inetchannelinfo.h"
#include <datamap.h>
#include "CTrace.h"
#include "CLuaPropProxy.h"
#include "CTempEntity.h"
#include "CCmdRunHook.h"
#include "TickRegen.h"
#include "ConVar_l4d.h"
#include "CUserMessage.h"
#include "LuaListthink.h"
#include <filesystem.h>
#include <ispatialpartition.h>
#include "MyNatives.h"
#include "CActionHook.h"
#include "CAmmoPack.h"

#include "Interface/ISurvivorBot.h"
#include "Interface/ITeam.h"
#include "Interface/IGameRules.h"

Sample				g_Sample;
SMEXT_LINK(&g_Sample);

ConVar				ukr_cvar_debug(		"ukr_coop_debug", 		"0", 		FCVAR_NONE, 	"Enable debug log", 				true, 0.f, 		true, 1.0f);
ConVar				ukr_next_bot_debug(	"ukr_next_bot_debug",	"1",		FCVAR_NONE,		"Enable debug logging the nextbot",	true, 0.f,		true, 1.f);
ConVar				ukr_my_witch_action("ukr_my_witch_action",	"1",		FCVAR_NONE,		"Enable to MyWitchAction",			true, 0.f,		true, 1.f);
ConVar				ukr_my_tank_action(	"ukr_my_tank_action",	"1",		FCVAR_NONE,		"Enable to MyTankAction",			true, 0.f,		true, 1.f);
ConVar				ukr_issue_vote_menu("ukr_issue_vote_menu", 	"1", 		FCVAR_NONE, 	"Enable vote menu", 				true, 0.f, 		true, 1.f);
ConVar				l4dSurvivorLimit(	"ukr_survivor_limit", 	"10.0", 	FCVAR_NONE, 	"Survivor Limit", 					true, 4.f, 		true, 10.f);
ConVar				l4dPlayerZombi(		"ukr_player_zombi", 	"20.0", 	FCVAR_NONE, 	"Player zombi Limit", 				true, 4.f, 		true, 20.f);

CDetour			    *WitchAttackCreate 					= nullptr;
// CDetour             *WitchAttackgetVictim 				= nullptr;
CDetour				*g_pCCommandBuffer_InsertCommand	= nullptr;
CDetour				*g_BoneIndexByName 					= nullptr;
CDetour				*g_OnGSClientDeny					= nullptr;
CDetour				*g_pOnCreateStandartEntity			= nullptr;

ConVar_l4d			*g_pConVar 							= nullptr;
HL2					*g_HL2 								= nullptr;
TickRegen   		*g_pRegen 							= nullptr;
LM					*m_sLog 							= nullptr;
chat_log			*m_sChatLog 						= nullptr;
ISoundEmitterSystemBase *soundemitterbase				= nullptr;
UkrCoop_Interface	*g_pSDKUkrCoop 						= nullptr;
IServerPluginHelpers *pluginhelpers 					= nullptr;
IServerGameClients	*serverClients 						= nullptr;
IPhraseCollection	*ipharases 							= nullptr;
IServerTools		*servertools 						= nullptr;
ICvar				*icvar 								= nullptr;
IBinTools			*g_pBinTools 						= nullptr;
IGameConfig			*g_pGameConf 						= nullptr;
IGeoIP              *g_pGeoIP 							= nullptr;
IGameEventManager2  *gameevents 						= nullptr;
IServer				*g_pServer 							= nullptr;
ISDKTools			*g_pSDKTools 						= nullptr;
ISDKHooks			*g_pSDKHooks 						= nullptr;
ITopMenuManager		*g_pTopMenu							= nullptr;
CGlobalVars			*g_pGlobals 						= nullptr;
IEngineTrace		*g_pTarce 							= nullptr;
ISpatialPartition	*g_pPartition 						= nullptr;
CBaseEntityList 	*g_pEntityList 						= nullptr;
INetworkStringTableContainer *g_pNetStringTableContainer= nullptr;
IVModelInfo			*g_pModelInfo						= nullptr;
IMDLCache			*g_pMDLCache						= nullptr;
CSharedEdictChangeInfo*g_pSharedChangeInfo				= nullptr;
CEventListenerLua	*g_pEventList						= nullptr;

#ifdef IS_VOTE_ENABLE
IForward			*g_pOnRegisterVote					= nullptr;
#endif

int g_WitchACharasterOffset = -1;

bool bNextFrame = false;

class SDKUkrCoop : public UkrCoop_Interface
{
public:
	virtual const char* GetInterfaceName()
	{
		return SMINTERFACE_UKRCOOP_NAME;
	}

	virtual unsigned int GetInterfaceVersion()
	{
		return SMINTERFACE_UKRCOOP_VERSION;
	}
public:
	virtual void UCLogMessage(const char *msg, ...)
	{
		char buffer[2048];
		va_list ap;

		va_start(ap, msg);
		vsnprintf(buffer, sizeof(buffer), msg, ap);
		m_sLog->LogToFileEx(false, "%s", buffer);
		va_end(ap);
	}
} g_SDKUkrCoop_API;

struct sLuaEventRef
{
	sLuaEventRef(luabridge::LuaRef& mRef) : pRef(mRef) {}
	~sLuaEventRef() {}
	luabridge::LuaRef pRef;
};

struct cEventList
{
	cEventList() { }
	~cEventList()
	{
		ListEvent.clear();
	}
	SourceHook::String eventName;
	SourceHook::List<sLuaEventRef> ListEvent;
};

class CEventListenerLua : public CGameEventListeners
{
public:
	CEventListenerLua() : CGameEventListeners() {}
	~CEventListenerLua() { luaEventList.clear(); }

	void AddEventListLua(const char *sEventName, luabridge::LuaRef sCallBack);
	virtual void FireGameEvent(IGameEvent *event);
	
private:
	SourceHook::List<sLuaEventRef> *GetListEvent(const char *sNameEvent);
	SourceHook::List<cEventList>	luaEventList;
};

IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
	return engine->GetChangeAccessor( (const edict_t *)this );
}

const IChangeInfoAccessor *CBaseEdict::GetChangeAccessor() const
{
	return engine->GetChangeAccessor( (const edict_t *)this );
}

bool CGameTrace::DidHitWorld() const
{
	return m_pEnt == g_CallHelper->GetWorldEnt();
}

bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != NULL && !DidHitWorld();
}

int CGameTrace::GetEntityIndex() const
{
	if ( m_pEnt )
		return reinterpret_cast<IBaseEntity*>(m_pEnt)->entindex();
	else
		return -1;
}


SourceHook::List<LuaListThink> ListCallThink;

SH_DECL_HOOK2(		IGameEventManager2,		FireEvent,					SH_NOATTRIB, 0,		bool, IGameEvent *, bool);

SH_DECL_HOOK5(		IServerGameClients,		ClientConnect,				SH_NOATTRIB, 0,		bool, edict_t *, const char *, const char *, char *, int);
SH_DECL_HOOK1_void(	IServerGameClients,		ClientDisconnect,			SH_NOATTRIB, 0,		edict_t *);
SH_DECL_HOOK2_void(	IServerGameClients,		ClientPutInServer,			SH_NOATTRIB, 0,		edict_t *, const char *);
SH_DECL_HOOK1_void(	IServerGameClients,		SetCommandClient,			SH_NOATTRIB, false, int);
SH_DECL_HOOK2_void( IServerGameClients,		ClientCommand, 				SH_NOATTRIB, 0, 	edict_t *, const CCommand &);

SH_DECL_HOOK6(		IServerGameDLL,			LevelInit,					SH_NOATTRIB, false, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK1_void(	IServerGameDLL,			GameFrame,					SH_NOATTRIB, false, bool);
SH_DECL_HOOK0_void(	IServerGameDLL,			LevelShutdown,				SH_NOATTRIB, false);

SH_DECL_HOOK1_void(	ConCommand,				Dispatch,					SH_NOATTRIB, false, const CCommand &);

SH_DECL_HOOK3_void(	ICvar,					CallGlobalChangeCallbacks,	SH_NOATTRIB, false, ConVar *, const char *, float);

SH_DECL_MANUALHOOK0_void(Think, 0, 0, 0);

int LuaSetThink(CBaseEntity* pThisPtr, luabridge::LuaRef call, bool post)
{
	if(call.isFunction())
	{
		LuaListThink called(pThisPtr);
		SourceHook::List<LuaListThink>::iterator refList = ListCallThink.begin();
		while(refList != ListCallThink.end())
		{
			if(*refList == called)
			{
				if(post)
				{
					if(refList->GetHookPostId() == -1)
					{
						refList->SetLuaRefPost(call);
						refList->SetHookPostId(SH_ADD_MANUALVPHOOK(Think, pThisPtr, SH_MEMBER(&g_Sample, &Sample::EntityThinkPost), post));
					}
					else
					{
						refList->SetLuaRefPost(call);
					}
					return refList->GetHookPostId();
				}
				else
				{
					if(refList->GetHookPreId() != -1)
					{
						refList->SetLuaRefPre(call);
						refList->SetHookPreId(SH_ADD_MANUALVPHOOK(Think, pThisPtr, SH_MEMBER(&g_Sample, &Sample::EntityThinkPre), post));
					}
					else
					{
						refList->SetLuaRefPre(call);
					}
					return refList->GetHookPreId();
				}
			}
			refList++;
		}

		if(post)
		{
			called.SetLuaRefPost(call);
			called.SetHookPostId(SH_ADD_MANUALVPHOOK(Think, pThisPtr, SH_MEMBER(&g_Sample, &Sample::EntityThinkPost), post));
		}
		else
		{
			called.SetLuaRefPre(call);
			called.SetHookPreId(SH_ADD_MANUALVPHOOK(Think, pThisPtr, SH_MEMBER(&g_Sample, &Sample::EntityThinkPre), post));
		}
		ListCallThink.push_back(called);
		return post ? (ListCallThink.back()).GetHookPostId() : (ListCallThink.back()).GetHookPreId();
	}
	return 0;
}

struct GSClientApprove_t
{
	CSteamID m_SteamID;
};

struct GSClientDeny_t
{
	CSteamID m_SteamID;
	EDenyReason m_eDenyReason;
	char m_pchOptionalText[128];
};

/*void CSteam3Server::OnGSClientDeny(GSClientDeny_t *)*/
DETOUR_DECL_MEMBER1(OnGSClientDeny, void, GSClientDeny_t *, pGSClientDeny)
{
	m_sLog->LogToFileEx(false, "OnGSClientDeny: SteamId [U:%u:%u], DenyReason [%d], OptionalText [%s]", 
		pGSClientDeny->m_SteamID.GetEUniverse(), 
		pGSClientDeny->m_SteamID.GetAccountID(), 
		pGSClientDeny->m_eDenyReason, 
		pGSClientDeny->m_pchOptionalText);

	switch (pGSClientDeny->m_eDenyReason)
	{
		case EDenyReason::k_EDenyCheater:
		{
			lua_State *g_pState = nullptr;
			if((g_pState = g_Sample.GetLuaState()) != nullptr)
			{
				static luabridge::LuaRef OnClientDeny = luabridge::getGlobal(g_pState, "OnClientDeny");
				luabridge::LuaRef g_mResult = OnClientDeny(pGSClientDeny->m_SteamID);

				if(g_mResult.isBool() && (bool)g_mResult == true)
				{
					GSClientApprove_t mGSClientA;
					mGSClientA.m_SteamID = pGSClientDeny->m_SteamID;
					g_CallHelper->OnGSClientApprove(this, &mGSClientA);
					m_sLog->LogToFileEx(false, "OnGSClientDeny: The connection is allowed!");
					return;
				}
			}
			break;
		}
	}
	return DETOUR_MEMBER_CALL1(OnGSClientDeny, pGSClientDeny);
}

// void OnNextFrame(void *data)
// {
// 	CellArray* g_pArray = reinterpret_cast<CellArray *>(data);

// 	int ilen = g_pArray->size();
// 	for(int i = 0; i < ilen; i++)
// 	{
// 		g_Sample.InsertCommands((const char*)g_pArray->at(i));
// 		engine->ServerExecute();
// 	}
// 	g_pArray->clear();
// 	delete g_pArray;
// }

DETOUR_DECL_MEMBER3(CCommandBuffer__InsertCommand, bool, const char*, pArgs, int, nCommandSize, int, nTick)
{
	auto result = DETOUR_MEMBER_CALL1(CCommandBuffer__InsertCommand, pArgs, nCommandSize, nTick);

	if(result)
		return result;

	auto OnNextFrame = [](void *data) {
		CellArray* g_pArray = reinterpret_cast<CellArray *>(data);

		int ilen = g_pArray->size();
		for(int i = 0; i < ilen; i++)
		{
			g_Sample.InsertCommands((const char*)g_pArray->at(i));
			engine->ServerExecute();
		}
		g_pArray->clear();
		delete g_pArray;
	};

	CellArray *g_pArray = new CellArray((ARGS_BUFFER_LENGTH + 3) / 4);
	cell_t *blk = g_pArray->push();
	if(blk)
	{
		g_Sample.strncopy((char*)blk, pArgs, g_pArray->blocksize() * sizeof(cell_t));
		g_pSM->AddFrameAction(OnNextFrame, g_pArray);
		return true;
	} else {
		delete g_pArray;
	}

	return result;
}

DETOUR_DECL_MEMBER1(WitchAttack__WitchAttack, void*, CBaseEntity*, pEntity)
{
	void *result = DETOUR_MEMBER_CALL1(WitchAttack__WitchAttack, pEntity);
	DWORD &iCharId = access_member<DWORD>(this, g_WitchACharasterOffset * 4UL);

	iCharId = 4;
	return result;
}

DETOUR_DECL_STATIC2(BoneIndexByName, int, const CStudioHdr*, pStudioHdr, const char*, pBoneName)
{
	if(pStudioHdr)
	{
		return DETOUR_STATIC_CALL(BoneIndexByName, pStudioHdr, pBoneName);
	}

	m_sLog->LogToFileEx(false, "pBoneName => %-30s pStudioHdr => null", pBoneName);
	return -1;
}

DETOUR_DECL_MEMBER0(CTerrorGameRules_CreateStandartEntitys, void)
{
	DETOUR_MEMBER_CALL2(CTerrorGameRules_CreateStandartEntitys);

#ifdef IS_VOTE_ENABLE
	g_pOnRegisterVote->Execute(NULL);
#endif
}

class Timers : public ITimedEvent
{
	ResultType OnTimer(ITimer* pTimer, void *pData)
	{
		int client = (int)pData;

		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
		if(!pPlayer){
			return Pl_Stop;
		} else if(!pPlayer->IsConnected()){
			return Pl_Stop;
		} else if(!pPlayer->IsInGame()){
			return Pl_Continue;
		}

		char msg[255];
		g_HL2->Translate(msg, sizeof(msg), "%T", 2, NULL, "Visit_grup", &client);
		g_HL2->TextMsg(client, CHAT, msg);

		return Pl_Stop;
	}
	void OnTimerEnd(ITimer* pTimer, void* pData){}
} m_Timers;

class survivor_limit_Chaged : public IConVarChangeListener
{
public:
	void OnConVarChanged(ConVar *pCVar, const char *oldValue, const char* newValue) {
		pCVar->SetValue(l4dSurvivorLimit.GetFloat());
	}
} pHookChagedSurvivorLimit;

class max_player_zombies_Chaged : public IConVarChangeListener
{
public:
	void OnConVarChanged(ConVar *pCVar, const char *oldValue, const char* newValue) {
		pCVar->SetValue(l4dPlayerZombi.GetFloat());
	}
} pHookChagedMaxPlayerZombies;

void CEventListenerLua::AddEventListLua(const char *sEventName, luabridge::LuaRef sCallBack)
{
	if(sCallBack.isFunction())
	{
		auto eList = GetListEvent(sEventName);
		if(eList != nullptr) 
		{
			eList->push_back(sLuaEventRef(sCallBack));
		} else {
			cEventList pInfo;
			pInfo.eventName.assign(sEventName);
			pInfo.ListEvent.push_back(sLuaEventRef(sCallBack));
			luaEventList.push_back(pInfo);

			ListenForGameEvent(sEventName);
		}
	}
}

void CEventListenerLua::FireGameEvent(IGameEvent *event)
{
	auto gList = GetListEvent(event->GetName());
	if(gList != nullptr)
	{
		for(auto eLies : *gList)
		{
			if(eLies.pRef.isFunction())
			{
				eLies.pRef(event);
			}
		}
	}
}

SourceHook::List<sLuaEventRef> *CEventListenerLua::GetListEvent(const char *sNameEvent)
{
	for(auto& e_list : luaEventList)
	{
		if(g_Sample.my_bStrcmp(e_list.eventName.c_str(), sNameEvent))
		{
			return &e_list.ListEvent;
		}
	}
	return nullptr;
}

bool		Sample::DispatchKeyValues(CBaseEntity *pEntity, const char *szField, const char *szValue) 			{ return servertools->SetKeyValue(pEntity, szField, szValue); }
bool		Sample::DispatchKeyValuesVector(CBaseEntity *pEntity, const char *szField, const Vector &vecValue) 	{ return servertools->SetKeyValue(pEntity, szField, vecValue); }
bool		Sample::DispatchKeyValuesFloat(CBaseEntity *pEntity, const char *szField, float flValue) 			{ return servertools->SetKeyValue(pEntity, szField, flValue); }
CBaseEntity*Sample::CreateEntityByName(const char *szClassName) 												{ return reinterpret_cast<CBaseEntity *>(servertools->CreateEntityByName(szClassName)); }
void		Sample::DispatchSpawn(CBaseEntity *pEntity) 														{ servertools->DispatchSpawn(pEntity); }
CSteamID	Sample::GetUserSteamId(int val1, int val2) 															{ return CSteamID(val1 | (val2 << 1), 1, k_EUniversePublic, k_EAccountTypeIndividual); }

const char *Sample::GetTeamName(const int client) const
{
	IBaseEntity *pEntity = GetVirtualClass<IBaseEntity>(client);
	if(pEntity)
	{
		ITeam *pTeam = pEntity->GetTeam();
		if(pTeam)
		{
			return pTeam->GetName();
		}
	}
	return "Team_Invalid";
}

IBaseEntity *ConvertToSimpleProp(IBaseEntity* pEnt)
{
	IBaseEntity* pRetVal = nullptr;
	int modelindex = pEnt->GetModelIndex();
	const model_t* model = g_pModelInfo->GetModel(modelindex);
	if(model && g_pModelInfo->GetModelType(model) == 1)
	{
		pRetVal = (IBaseEntity*)servertools->CreateEntityByName("simple_physics_brush");
	}
	else
	{
		pRetVal = (IBaseEntity*)servertools->CreateEntityByName("simple_physics_prop");
	}

	pRetVal->KeyValue("model", STRING(pEnt->GetModelName()));
	pRetVal->SetAbsOrigin(pEnt->GetAbsOrigin());
	pRetVal->SetAbsAngles(pEnt->GetAbsAngles());
	pRetVal->Spawn();
	pRetVal->VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);

	return pRetVal;
}

bool UTIL_TransferPoseParameters( IBaseEntity *pSourceEntity, IBaseEntity *pDestEntity )
{
	IBaseAnimating *pSourceBaseAnimating = dynamic_cast<IBaseAnimating*>( pSourceEntity );
	IBaseAnimating *pDestBaseAnimating = dynamic_cast<IBaseAnimating*>( pDestEntity );

	if ( !pSourceBaseAnimating || !pDestBaseAnimating )
		return false;

	for ( int iPose = 0; iPose < MAXSTUDIOPOSEPARAM; ++iPose )
	{
		pDestBaseAnimating->SetPoseParameter( iPose, pSourceBaseAnimating->GetPoseParameter( iPose ) );
	}
	
	return true;
}

void UnlinkChild( IBaseEntity *pParent, IBaseEntity *pChild )
{
	IBaseEntity *pList;
	IHANDLES *pPrev;

	pList = pParent->m_hMoveChild;
	pPrev = &pParent->m_hMoveChild;
	while ( pList )
	{
		IBaseEntity *pNext = pList->m_hMovePeer;
		if ( pList == pChild )
		{
			// patch up the list
			pPrev->Set( pNext );

			// Clear hierarchy bits for this guy
			pList->m_hMoveParent.Set( NULL );
			pList->m_hMovePeer.Set( NULL );
			pList->NetworkProp()->SetNetworkParent( CBaseHandle() );
			pList->DispatchUpdateTransmitState();	
			pList->OnEntityEvent( 2, NULL );
			
			pParent->RecalcHasPlayerChildBit();
			return;
		}
		else
		{
			pPrev = &pList->m_hMovePeer;
			pList = pNext;
		}
	}
}

void LinkChild( IBaseEntity *pParent, IBaseEntity *pChild )
{
	IHANDLES hParent;
	hParent.Set( pParent );
	pChild->m_hMovePeer.Set( pParent->FirstMoveChild() );
	pParent->m_hMoveChild.Set( pChild );
	pChild->m_hMoveParent = hParent;
	pChild->NetworkProp()->SetNetworkParent( hParent );
	pChild->DispatchUpdateTransmitState();
	pChild->OnEntityEvent( 2, NULL );
	pParent->RecalcHasPlayerChildBit();
}

void TransferChildren( IBaseEntity *pOldParent, IBaseEntity *pNewParent )
{
	IBaseEntity *pChild = pOldParent->FirstMoveChild();
	while ( pChild )
	{
		Vector vecAbsOrigin = pChild->GetAbsOrigin();
		QAngle angAbsRotation = pChild->GetAbsAngles();
		Vector vecAbsVelocity = pChild->GetAbsVelocity();

		UnlinkChild( pOldParent, pChild );
		LinkChild( pNewParent, pChild );

		pChild->m_vecAbsOrigin.Init( FLT_MAX, FLT_MAX, FLT_MAX );
		pChild->m_angAbsRotation.Init( FLT_MAX, FLT_MAX, FLT_MAX );
		pChild->m_vecAbsVelocity.Init( FLT_MAX, FLT_MAX, FLT_MAX );

		pChild->SetAbsOrigin(vecAbsOrigin);
		pChild->SetAbsAngles(angAbsRotation);
		pChild->SetAbsVelocity(vecAbsVelocity);

		pChild  = pOldParent->FirstMoveChild();
	}
}

bool Disolved(CBaseEntity* pThisPtr, const char* pMaterialName, bool bNPCOnly, int nDissolveType, Vector vDissolveOrigin, int nMagnitude)
{
	IBaseAnimating* pAnim = (IBaseAnimating*)(reinterpret_cast<IBaseEntity*>(pThisPtr)->GetBaseAnimating());
	if(pAnim && !pAnim->IsDissolving())
	{
		IBaseEntity *pDissolvingObj = ConvertToSimpleProp(pAnim);
		if(pDissolvingObj)
		{
			pDissolvingObj->SetName(pAnim->GetEntityName());
			UTIL_TransferPoseParameters(pAnim, pDissolvingObj);
			TransferChildren(pAnim, pDissolvingObj);
			pDissolvingObj->SetCollisionGroup(COLLISION_GROUP_INTERACTIVE_DEBRIS);
			pAnim->AddSolidFlags(FSOLID_NOT_SOLID);
			pAnim->AddEffects(EF_NODRAW);

			IPhysicsObject* pPhys = pDissolvingObj->VPhysicsGetObject();
			if(pPhys)
			{
				pPhys->EnableGravity(false);

				Vector vOldVel;
				AngularImpulse vOldAng;
				pAnim->GetVelocity( &vOldVel, &vOldAng );

				Vector vVel = vOldVel;
				AngularImpulse vAng = vOldAng;

				vVel.z += 5.f;
				vAng.z += 20.0f;

				pPhys->SetVelocity( &vVel, &vAng );
			}

			pAnim->AddFlag(FL_DISSOLVING);
			g_CallHelper->UTIL_Remove(pAnim);
		}

		if(nDissolveType < ENTITY_DISSOLVE_NORMAL 
		|| nDissolveType > ENTITY_DISSOLVE_BITS)
			nDissolveType = ENTITY_DISSOLVE_NORMAL;

		if(pDissolvingObj && pDissolvingObj->GetBaseAnimating()) 
		{ 
			reinterpret_cast<IBaseAnimating*>(pDissolvingObj->GetBaseAnimating())->Dissolve(pMaterialName, g_pGlobals->curtime, bNPCOnly, nDissolveType, vDissolveOrigin, nMagnitude); 
			return true; 
		} 
	}

	return false; 
}

bool Sample::SDK_OnMetamodLoad(SourceMM::ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_ANY(GetServerFactory, gamedll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, serverClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, pluginhelpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, soundemitterbase, ISoundEmitterSystemBase, SOUNDEMITTERSYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pTarce, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_ANY(GetEngineFactory, g_pPartition, ISpatialPartition, INTERFACEVERSION_SPATIALPARTITION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetStringTableContainer, INetworkStringTableContainer, INTERFACENAME_NETWORKSTRINGTABLESERVER);
	GET_V_IFACE_ANY(GetEngineFactory, g_pModelInfo, IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pMDLCache, IMDLCache, MDLCACHE_INTERFACE_VERSION);

	g_pCVar = icvar;
	ConVar_Register(0, this);

	g_pGlobals = ismm->GetCGlobals();
	g_pSharedChangeInfo = engine->GetSharedEdictChangeInfo();

	new ConVar("ukr_coop_version", SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY, "UKRCOOP Version");

	m_sLog = new LM();
	g_HL2 = new HL2();
	g_pRegen = new TickRegen();
	g_pConVar = new ConVar_l4d();
	m_sChatLog = new chat_log();
	g_pEventList = new CEventListenerLua();

	luabridge::getGlobalNamespace(L_sctipt)
		.addFunction("AddFrag", std::function<int(int, int)>([](int f1, int f2) { return (f1 | f2); }))
		.addFunction("RemoveFrag", std::function<int(int, int)>([](int f1, int f2) { return (f1 & ~f2); }))
		.addFunction("Msg", std::function<void(const char *)>([](const char *str){ Msg("%s\n", str); }))
		.addFunction("IsFlagSet", std::function<bool(const int, const int)>([](const int flags, const int flagSet) { return (flags & flagSet) == flagSet; }))
		.addFunction("CreateAmmoBox", std::function<int(Vector, bool)>([](Vector vecOrigion, bool IsSoutgun) { return OnCreate(vecOrigion, IsSoutgun); }))
		.beginClass<QAngle>("QAngle")
			.addConstructor<void(*)(void)>()
			.addConstructor<void(*)(float , float, float)>()
			.addProperty("x", &QAngle::x)
			.addProperty("y", &QAngle::y)
			.addProperty("z", &QAngle::z)
			.addFunction("IsValid", &QAngle::IsValid)
			.addFunction("Init", &QAngle::Init)
			.addFunction("IsIdentical", &IsIdentical)
			.addFunction("IsNotIdentical", IsNotIdentical)
			.addFunction("AngleVectors", std::function<void(const QAngle*, Vector*, Vector*, Vector*)>([](const QAngle *pThisPtr, Vector* forvard, Vector* right, Vector* up){ AngleVectors((*pThisPtr), forvard, right, up); }))
		.endClass()
		.beginClass<Vector>("Vector")
			.addConstructor<void(*)(void)>()
			.addConstructor<void(*)(float, float, float)>()
			.addProperty("x", &Vector::x)
			.addProperty("y", &Vector::y)
			.addProperty("z", &Vector::z)
			.addFunction("IsValid", &Vector::IsValid)
			.addFunction("Init", &Vector::Init)
			.addFunction("DistTo", &Vector::DistTo)
			.addFunction("DistToSqr", &Vector::DistToSqr)
			.addFunction("Cross", &Vector::Cross)
			.addFunction("Normalize", &Vector::NormalizeInPlace)
			.addFunction("Copy", std::function<void(Vector*, Vector*)>([](Vector* pThis, Vector *vec) { if(vec) { pThis->x = vec->x; pThis->y = vec->y; pThis->z = vec->z; } }))
			.addFunction("VectorAngles", std::function<void(const Vector*, QAngle*)>([](const Vector *vec, QAngle *ang) { VectorAngles((*vec), (*ang)); }))
			.addFunction("VectorScale", std::function<void(const Vector*, float)>([](const Vector* pThisPtr, float scale){ VectorScale((*pThisPtr), scale, (*((Vector*)pThisPtr))); }))
			.addFunction("VectorAdd", std::function<void(const Vector*, const Vector*, Vector*)>([](const Vector* pThis, const Vector *Vec2, Vector* res){ VectorAdd((*pThis), (*Vec2), (*res)); }))
			.addFunction("VectorSubtract", std::function<void(const Vector*, const Vector*, Vector*)>([](const Vector* pThis, const Vector* vec2, Vector* res) { VectorSubtract((*pThis), (*vec2), (*res)); }))
			.addFunction("IsIdentical", &IsIdentical)
			.addFunction("IsNotIdentical", &IsNotIdentical)
		.endClass()
		.beginClass<IPlayerInfo>("PlayerInfo")
			.addFunction("GetName", &IPlayerInfo::GetName)
			.addFunction("GetUserID", &IPlayerInfo::GetUserID)
			.addFunction("GetNetworkIDString", &IPlayerInfo::GetNetworkIDString)
			.addFunction("GetTeamIndex", &IPlayerInfo::GetTeamIndex)
			.addFunction("ChangeTeam", &IPlayerInfo::ChangeTeam)
			.addFunction("GetFragCount", &IPlayerInfo::GetFragCount)
			.addFunction("GetDeathCount", &IPlayerInfo::GetDeathCount)
			.addFunction("IsConnected", &IPlayerInfo::IsConnected)
			.addFunction("GetArmorValue", &IPlayerInfo::GetArmorValue)
			.addFunction("IsHLTV", &IPlayerInfo::IsHLTV)
			.addFunction("IsPlayer", &IPlayerInfo::IsPlayer)
			.addFunction("IsFakeClient", &IPlayerInfo::IsFakeClient)
			.addFunction("IsDead", &IPlayerInfo::IsDead)
			.addFunction("IsInAVehicle", &IPlayerInfo::IsInAVehicle)
			.addFunction("IsObserver", &IPlayerInfo::IsObserver)
			.addFunction("GetAbsOrigin", std::function<Vector(IPlayerInfo* p)> ([](IPlayerInfo* p){ return p->GetAbsOrigin(); }))
			.addFunction("GetAbsAngles", std::function<QAngle(IPlayerInfo* p)> ([](IPlayerInfo* p){ return p->GetAbsAngles(); }))
			.addFunction("GetPlayerMins", std::function<Vector(IPlayerInfo* p)>([](IPlayerInfo* p){ return p->GetPlayerMins();}))
			.addFunction("GetPlayerMaxs", std::function<Vector(IPlayerInfo* p)>([](IPlayerInfo* p){ return p->GetPlayerMaxs();}))
			.addFunction("GetWeaponName", &IPlayerInfo::GetWeaponName)
			.addFunction("GetModelName", &IPlayerInfo::GetModelName)
			.addFunction("GetHealth", std::function<int (IPlayerInfo*)>([](IPlayerInfo* p){ return p->GetHealth();}))
			.addFunction("GetMaxHealth", std::function<int (IPlayerInfo*)>([](IPlayerInfo* p){ return p->GetMaxHealth();}))
		.endClass()
		.beginClass<CBaseEntity>("ent")
			.addFunction("SetModel", std::function<void(CBaseEntity *, const char *)>([](CBaseEntity *pThisPtr, const char *szName) { GetVirtualClass<ITerrorPlayer>(pThisPtr)->SetModel(szName); }))
			.addFunction("Dissolved", &Disolved)
			.addFunction("GetModelName", std::function<const char*(CBaseEntity*)>([](CBaseEntity* pThisPtr) { return GetVirtualClass<ITerrorPlayer>(pThisPtr)->GetModelName().ToCStr(); }))
			.addFunction("Teleport", std::function<void(CBaseEntity *, Vector *, QAngle *, Vector *)>([](CBaseEntity *pThis_ptr, Vector *pos, QAngle *angles, Vector *velocity) { GetVirtualClass<ITerrorPlayer>(pThis_ptr)->Teleport(pos, angles, velocity); }))
			.addProperty("ispawnflags", &Prop_get_spawnflags, &Prop_set_spawnflags)
			.addFunction("Activate", std::function<void(CBaseEntity *)>([](CBaseEntity *pThisPtr) { GetVirtualClass<ITerrorPlayer>(pThisPtr)->Activate(); }))
			.addFunction("TakeHealth", std::function<int (CBaseEntity *, float, int)>([](CBaseEntity *pThisPtr, float iHealth, int bitsDamageType) { return GetVirtualClass<ITerrorPlayer>(pThisPtr)->TakeHealth(iHealth, bitsDamageType); }))
			.addFunction("WorldSpaceCenter", std::function<Vector(CBaseEntity *)>([](CBaseEntity *pThisPtr) { return GetVirtualClass<ITerrorPlayer>(pThisPtr)->WorldSpaceCenter(); }))
			.addFunction("OnTakeDamage", std::function<int(CBaseEntity *, CTakeDamageInfoHack&)>([](CBaseEntity* pThisPtr, CTakeDamageInfoHack& mDamag){return GetVirtualClass<ITerrorPlayer>(pThisPtr)->OnTakeDamage((CTakeDamageInfo&)mDamag);}))
			.addProperty("fFlags", &Prop_get_fFlags, &Prop_set_fFlags)
			.addProperty("EFlags", &Prop_get_EFlags, &Prop_set_EFlags)
			.addFunction("RoundRespawn", std::function<void(CBaseEntity *)>([](CBaseEntity *p) { return GetVirtualClass<ITerrorPlayer>(p)->RoundRespawn(); } ))
			.addFunction("WeaponDrop", std::function<void(CBaseEntity *, CBaseEntity*, Vector *, Vector *)>([](CBaseEntity* p, CBaseEntity* pWeapon, Vector* vec, Vector* velosity) { GetVirtualClass<ITerrorPlayer>(p)->Weapon_Drop((CBaseCombatWeapon *)pWeapon, vec, velosity); }))
			.addFunction("GiveNamedItem", std::function<CBaseEntity *(CBaseEntity *, char const *, int)>([](CBaseEntity *pThisPtr, char const *szItem, int iSubType) { return GetVirtualClass<ITerrorPlayer>(pThisPtr)->GiveNamedItem(szItem, iSubType, true); } ))
			.addFunction("RemovePlayerItem", std::function<bool(CBaseEntity *, CBaseEntity *)>([](CBaseEntity *pThisPtr, CBaseEntity *pWeapon) { return GetVirtualClass<ITerrorPlayer>(pThisPtr)->RemovePlayerItem((CBaseCombatWeapon *)pWeapon); }))
			.addFunction("CommitSuicide", std::function<void(CBaseEntity *, bool)>([](CBaseEntity *pThisPtr, bool Val1) { GetVirtualClass<ISurvivorBot>(pThisPtr)->CommitSuicide(Val1); }))
			.addProperty("vecOrigin", &Prop_get_vecOrigin, &Prop_set_vecOrigin)
			.addProperty("angRotation", &Prop_get_angRotation, &Prop_set_angRotation)
			.addFunction("AcceptInput", std::function<bool(CBaseEntity*, const char*, CBaseEntity*, CBaseEntity*, variant_t, int)>([](CBaseEntity* pThisPtr, const char*sInputName, CBaseEntity* pActivate, CBaseEntity* pCalled, variant_t var, int outputID) { return GetVirtualClass<ITerrorPlayer>(pThisPtr)->AcceptInput(sInputName, pActivate, pCalled, var, outputID); } ))
			.addFunction("GetClassName", &GetNameClass)
			.addFunction("SetThink", &LuaSetThink)
			.addFunction("SetSequence", std::function<void(CBaseEntity*, int)>([](CBaseEntity* pThisPtr, int nSequence) { GetVirtualClass<IBaseAnimating>(pThisPtr)->SetSequence(nSequence); }))
			.addProperty("flCycle", &Prop_get_flCycle, &Prop_set_flCycle)
			.addProperty("nSequence", &Prop_get_nSequence, &Prop_set_nSequence)
			.addProperty("flAnimTime", &Prop_get_flAnimTime, &Prop_set_flAnimTime)
			.addProperty("nRenderFX", &Prop_get_nRenderFX, &Prop_set_nRenderFX)
			.addFunction("GetPoseParameter", &GetPoseParameter)
			.addFunction("SetPoseParameter", &SetPoseParameter)
			.addProperty("mobRush", &Prop_get_mobRush, &Prop_set_mobRush)
			.addProperty("hGroundEntity", &Prop_get_hGroundEntity, &Prop_set_hGroundEntity)
			.addProperty("hOwnerEntity", &Prop_get_hOwnerEntity, &Prop_set_hOwnerEntity)
			.addProperty("rage", &Prop_get_rage, &Prop_set_rage)
			.addProperty("flPlaybackRate", &Prop_get_flPlaybackRate, &Prop_set_flPlaybackRate)
			.addProperty("CollisionGroup", &Prop_get_CollisionGroup, &Prop_set_CollisionGroup)
			.addProperty("SolidFlags", &Prop_get_SolidFlags, &Prop_set_SolidFlags)
			.addProperty("MoveType", &Prop_get_MoveType, &Prop_set_MoveType)
			.addProperty("RenedMode", &Prop_get_nRenderMode, &Prop_set_nRenderMode)
			.addProperty("pPhysicsObject", &Prop_get_PhysicsObject)
			.addProperty("flSimulationTime", &Prop_get_flSimulationTime, &Prop_set_flSimulationTime)
			.addProperty("hEffectEntity", &Prop_get_hEffectEntity, &Prop_set_hEffectEntity)
			.addProperty("fEffects", &Prop_get_fEffects, &Prop_set_fEffects)
			.addProperty("isIncapacitated", &Prop_get_isIncapacitated, &Prop_set_isIncapacitated)
			.addFunction("GetClientAimTarget", &GetClientAimTarget)
			.addProperty("bLocked", &Prop_get_bLocked, &Prop_set_bLocked)
			.addProperty("eDoorState", &Prop_get_eDoorState, &Prop_set_eDoorState)
			.addProperty("m_iMaxHealth", &Prop_get_MaxHealth, &Prop_set_MaxHealth)
			.addProperty("m_iHealth", &Prop_get_Health, &Prop_set_Health)
			.addFunction("SetFadeDistance", std::function<void(CBaseEntity*, float, float)>([](CBaseEntity* pThis, float min, float max) { GetVirtualClass<IBaseAnimating>(pThis)->SetFadeDistance(min, max); }))
			.addFunction("SetGlobalFadeScale", std::function<void(CBaseEntity*, float)>([](CBaseEntity* pThis, float fadeScale){ GetVirtualClass<IBaseAnimating>(pThis)->SetGlobalFadeScale(fadeScale); }))
		.endClass()
		.beginClass<edict_t>("edict")
			.addFunction("IsNull", std::function<bool(edict_t*)>([](edict_t *pThis) { return (pThis == nullptr); } ))
			.addFunction("IsFree", std::function<bool(edict_t*)>([](edict_t* pThis) { return pThis->IsFree(); } ))
		.endClass()
		.beginClass<IGamePlayer>("GamePlayer")
			.addFunction("GetName", &IGamePlayer::GetName)
			.addFunction("GetIPAddress", &IGamePlayer::GetIPAddress)
			.addFunction("GetAuthString", &IGamePlayer::GetAuthString)
			.addFunction("GetEdict", &IGamePlayer::GetEdict)
			.addFunction("InGame", &IGamePlayer::IsInGame)
			.addFunction("IsConnect", &IGamePlayer::IsConnected)
			.addFunction("IsFakeClient", &IGamePlayer::IsFakeClient)
			.addFunction("GetAdminId", &IGamePlayer::GetAdminId)
			.addFunction("SetAdminId", &IGamePlayer::SetAdminId)
			.addFunction("GetUserId", &IGamePlayer::GetUserId)
			.addFunction("GetLanguageId", &IGamePlayer::GetLanguageId)
			.addFunction("GetPlayerInfo", &IGamePlayer::GetPlayerInfo)
			.addFunction("RunAdminCacheChecks", &IGamePlayer::RunAdminCacheChecks)
			.addFunction("NotifyPostAdminChecks", &IGamePlayer::NotifyPostAdminChecks)
			.addFunction("GetSerial", &IGamePlayer::GetSerial)
			.addFunction("IsAuthorized", &IGamePlayer::IsAuthorized)
			.addFunction("Kick", &IGamePlayer::Kick)
			.addFunction("IsInKickQueue", &IGamePlayer::IsInKickQueue)
			.addFunction("MarkAsBeingKicked", &IGamePlayer::MarkAsBeingKicked)
			.addFunction("SetLanguageId", &IGamePlayer::SetLanguageId)
			.addFunction("IsSourceTV", &IGamePlayer::IsSourceTV)
			.addFunction("IsReplay", &IGamePlayer::IsReplay)
			.addFunction("GetSteamAccountID", &IGamePlayer::GetSteamAccountID)
			.addFunction("GetIndex", &IGamePlayer::GetIndex)
			.addFunction("PrintToConsole", &IGamePlayer::PrintToConsole)
			.addFunction("ClearAdmin", &IGamePlayer::ClearAdmin)
			.addFunction("GetSteamId64", &IGamePlayer::GetSteamId64)
			.addFunction("GetSteam2Id", &IGamePlayer::GetSteam2Id)
			.addFunction("GetSteam3Id", &IGamePlayer::GetSteam3Id)
		.endClass()
		.beginClass<Sample>("UkrCoop")
			.addFunction("GetGamePlayer", &Sample::GetGamePlayer)
			.addFunction("GetMaxClient", &Sample::GetMaxPlayers)
			.addFunction("GetClientUserID", &Sample::GetClientUserID)
			.addFunction("DispatchKeyValues", &Sample::DispatchKeyValues)
			.addFunction("DispatchKeyValuesFloat", &Sample::DispatchKeyValuesFloat)
			.addFunction("DispatchKeyValuesVector", &Sample::DispatchKeyValuesVector)
			.addFunction("CreateEntityByName", &Sample::CreateEntityByName)
			.addFunction("DispatchSpawn", &Sample::DispatchSpawn)
			.addFunction("GetUserSteamId", &Sample::GetUserSteamId)
		.endClass()
		.beginClass<IGameEvent>("GameEvent")
			.addFunction("GetBool", &IGameEvent::GetBool)
			.addFunction("GetInt", &IGameEvent::GetInt)
			.addFunction("GetUint64", &IGameEvent::GetUint64)
			.addFunction("GetFloat", &IGameEvent::GetFloat)
			.addFunction("GetString", &IGameEvent::GetString)
		.endClass()
		.beginClass<IGameHelpers>("GameHelpers")
			.addFunction("TextMsg", &IGameHelpers::TextMsg)
			.addFunction("IsLANServer", &IGameHelpers::IsLANServer)
			.addFunction("EdictOfIndex", &IGameHelpers::EdictOfIndex)
			.addFunction("IndexOfEdict", &IGameHelpers::IndexOfEdict)
			.addFunction("GetHandleEntity", &IGameHelpers::GetHandleEntity)
			.addFunction("SetHandleEntity", &IGameHelpers::SetHandleEntity)
			.addFunction("GetCurrentMap", &IGameHelpers::GetCurrentMap)
			.addFunction("ServerCommand", &IGameHelpers::ServerCommand)
			.addFunction("ReferenceToEntity", &IGameHelpers::ReferenceToEntity)
			.addFunction("EntityToReference", &IGameHelpers::EntityToReference)
			.addFunction("EntityToBCompatRef", &IGameHelpers::EntityToBCompatRef)
			.addFunction("IndexToReference", &IGameHelpers::IndexToReference)
			.addFunction("ReferenceToIndex", &IGameHelpers::ReferenceToIndex)
			.addFunction("ReferenceToBCompatRef", &IGameHelpers::ReferenceToBCompatRef)
			.addFunction("AddDelayedKick", &IGameHelpers::AddDelayedKick)
			.addFunction("HintTextMsg", &IGameHelpers::HintTextMsg)
			.addFunction("GetValveCommandLine", &IGameHelpers::GetValveCommandLine)
			.addFunction("IsMapValid", &IGameHelpers::IsMapValid)
		.endClass()
		.beginClass<IVEngineServer>("engine")
			.addFunction("IsModelPrecached", &IVEngineServer::IsModelPrecached)
			.addFunction("PrecacheModel", &IVEngineServer::PrecacheModel)
		.endClass()
		.beginClass<CSteamID>("SteamID")
			.addFunction("GetSteamId64", std::function<unsigned long long(CSteamID*)>([](CSteamID* pThisPtr) { return pThisPtr->ConvertToUint64(); }))
		.endClass()
		.beginClass<CEventListenerLua>("Event")
			.addFunction("AddListenerEvent", &CEventListenerLua::AddEventListLua)
		.endClass()
		.beginClass<CBotPlayerCreate>("BotCreator")
			.addFunction("SpawnBoomer",		std::function<ITerrorPlayer*(CBotPlayerCreate*, const Vector&, const QAngle&)>([](CBotPlayerCreate* pThis, const Vector& vecPos, const QAngle& vecAng)	{ return pThis->SpawnBoomer(vecPos, vecAng); }))
			.addFunction("SpawnCommon", 	std::function<IInfected*(CBotPlayerCreate*, const Vector&, const QAngle&)>([](CBotPlayerCreate* pThis, const Vector& vecPos, const QAngle& vecAng)		{ return pThis->SpawnCommon(vecPos, vecAng); }))
			.addFunction("SpawnHunter", 	std::function<ITerrorPlayer*(CBotPlayerCreate*, const Vector&, const QAngle&)>([](CBotPlayerCreate* pThis, const Vector& vecPos, const QAngle& vecAng)	{ return pThis->SpawnHunter(vecPos, vecAng); }))
			.addFunction("SpawnSmoker", 	std::function<ITerrorPlayer*(CBotPlayerCreate*, const Vector&, const QAngle&)>([](CBotPlayerCreate* pThis, const Vector& vecPos, const QAngle& vecAng)	{ return pThis->SpawnSmoker(vecPos, vecAng); }))
			.addFunction("SpawnTank",		std::function<ITerrorPlayer*(CBotPlayerCreate*, const Vector&, const QAngle&)>([](CBotPlayerCreate* pThis, const Vector& vecPos, const QAngle& vecAng)	{ return pThis->SpawnTank(vecPos, vecAng); }))
			.addFunction("SpawnWitch",		std::function<IWitch*(CBotPlayerCreate*, const Vector&, const QAngle&)>([](CBotPlayerCreate* pThis, const Vector& vecPos, const QAngle& vecAng)			{ return pThis->SpawnWitch(vecPos, vecAng); }))
			.addFunction("SpawnSurvivor", 	&CBotPlayerCreate::SpawnSurvivor)
		.endClass();

	luabridge::setGlobal(L_sctipt, this,			"UkrCoop");
	luabridge::setGlobal(L_sctipt, engine,			"engine");
	luabridge::setGlobal(L_sctipt, g_pEventList,	"Event");
	luabridge::setGlobal(L_sctipt, g_pBotCreator,	"BotCreator");

	return this->HookInit(error, maxlen);
}

bool Sample::HookInit(char *error, size_t maxlen)
{
	SH_ADD_HOOK(IServerGameClients, ClientConnect, 				serverClients, 	SH_MEMBER(this, &Sample::OnClientConnect), 						false);
	SH_ADD_HOOK(IServerGameClients, SetCommandClient, 			serverClients, 	SH_MEMBER(this, &Sample::SetComandClient), 						false);
	SH_ADD_HOOK(IServerGameClients, ClientDisconnect, 			serverClients, 	SH_MEMBER(this, &Sample::OnClientDisconnect), 					false);
	SH_ADD_HOOK(IServerGameClients, ClientPutInServer, 			serverClients, 	SH_MEMBER(g_pRegen, &TickRegen::OnClientConnect), 				false);
	SH_ADD_HOOK(IServerGameClients, ClientPutInServer, 			serverClients, 	SH_MEMBER(g_pCmdRunHook, &CCmdRunHook::OnClientPutInServer), 	true);
	SH_ADD_HOOK(IServerGameClients, ClientCommand, 				serverClients, 	SH_MEMBER(this, &Sample::OnClientCommand), 						false);
	SH_ADD_HOOK(IServerGameDLL, 	LevelInit, 					gamedll, 		SH_MEMBER(this, &Sample::OnLevelInit), 							false);
	SH_ADD_HOOK(IServerGameDLL, 	GameFrame, 					gamedll, 		SH_MEMBER(g_pRegen, &TickRegen::GameFrame), 					true);
	SH_ADD_HOOK(IServerGameDLL, 	LevelShutdown, 				gamedll, 		SH_MEMBER(g_pRegen, &TickRegen::LevelShutdown), 				false);
	SH_ADD_HOOK(ICvar, 				CallGlobalChangeCallbacks, 	icvar, 			SH_STATIC(ConVar_l4d::OnConVarGanger), 							false);

	if((m_pSayCmd = this->FindComands("say")) != nullptr)
	{
		SH_ADD_HOOK(ConCommand, Dispatch, m_pSayCmd, SH_STATIC(OnSayChat), false);
	} else {
		snprintf(error, maxlen, "[Error] Say Find Commands is Failed!");
	}

	if((m_pSayTeamCmd = this->FindComands("say_team")) != nullptr)
	{
		SH_ADD_HOOK(ConCommand, Dispatch, m_pSayTeamCmd, SH_STATIC(OnSayTeamChat), false);
	} else {
		snprintf(error, maxlen, "[Error] Say_Team Find Commands is Failed!");
	}

	if((m_pExecPtr = this->FindComands("exec")) != nullptr)
	{
		SH_ADD_HOOK(ConCommand, Dispatch, m_pExecPtr, SH_STATIC(OnExecDispatchPre), false);
		SH_ADD_HOOK(ConCommand, Dispatch, m_pExecPtr, SH_STATIC(OnExecDispatchPost), true);
	} else {
		snprintf(error, maxlen, "[Error] Exec Find Commands is Failed!");
	}

	return true;
}

bool Sample::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if(!my_bStrcmp(g_pSM->GetGameFolderName(), "left4dead"))
	{
		this->UTIL_Format(error, maxlength, "[Error] Cannot Load UkrCoop Ext on mods other than L4D");
		return false;
	}

	if (!gameconfs->LoadGameConfigFile("UkrCoop.l4d", &g_pGameConf, error, maxlength))
	{
		this->UTIL_Format(error, maxlength, "[Error] Cannot Load config file \"UkrCoop.l4d\"");
		return false;
	}
	extern void InitVirtualMap();
	InitVirtualMap();

	g_pConVar->SetBounds("z_max_player_zombies", true, 4.0, true, 16.0);
	g_pConVar->SetBounds("survivor_limit", true, 4.0, true, 16.0);

	g_pConVar->AddHookConVarChanged("z_max_player_zombies", &pHookChagedMaxPlayerZombies);
	g_pConVar->AddHookConVarChanged("ukr_player_zombi", &pHookChagedMaxPlayerZombies);
	g_pConVar->AddHookConVarChanged("survivor_limit", &pHookChagedSurvivorLimit);
	g_pConVar->AddHookConVarChanged("ukr_survivor_limit", &pHookChagedSurvivorLimit);

	if(g_pConVar->GetConVarInt("z_max_player_zombies") != l4dPlayerZombi.GetInt())
	{
		g_pConVar->SetConVarFloat("z_max_player_zombies", l4dPlayerZombi.GetFloat());
	}

	if(g_pConVar->GetConVarInt("survivor_limit") != l4dSurvivorLimit.GetInt())
	{
		g_pConVar->SetConVarFloat("survivor_limit", l4dSurvivorLimit.GetFloat());
	}

	sharesys->AddDependency(myself, "bintools.ext", true, true);
	sharesys->AddDependency(myself, "geoip.ext", true, true);
	sharesys->AddDependency(myself, "topmenus.ext", true, true);

	g_pShareSys->AddNatives(myself, g_UkrCoopNatives);

#ifdef IS_VOTE_ENABLE
	g_pOnRegisterVote = forwards->CreateForward("OnRegisterVote", ET_Ignore, 0, NULL);
#endif
	m_sLog->InitLogMesseg();
	m_sChatLog->InitChatLog();

	luabridge::setGlobal(L_sctipt, gamehelpers, "GameHelpers");

	MY_VOID_SH_MANUALHOOK_RECONFIGURE_NOPARAM(IBaseEntity, Think);

	char Path[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_SM, Path, sizeof(Path), "vscripts/");

	lua_getglobal(L_sctipt, "package");
	lua_getfield(L_sctipt, -1, "path");
	lua_pop(L_sctipt, 1);
	
	char sPackagePath[PLATFORM_MAX_PATH];
	UTIL_Format(sPackagePath, sizeof(sPackagePath), "%s?.lua;%s", Path, lua_tostring(L_sctipt, -1));

	lua_pushstring(L_sctipt, sPackagePath);
	lua_setfield(L_sctipt, -2, "path");
	lua_pop(L_sctipt, 1);

	char sPath[PLATFORM_MAX_PATH];
	UTIL_Format(sPath, sizeof(sPath), "%sUkrCoop.lua", Path);
	if(luaL_loadfile(L_sctipt, sPath) || lua_pcall(L_sctipt, 0, LUA_MULTRET, 0))
	{
		m_sLog->LogToFileEx(false, "[LUA ERROR] %s", lua_tostring(L_sctipt, -1));
		lua_pop(L_sctipt, 1);
	}

	lua_pcall(L_sctipt, 0, 0, 0);

	static luabridge::LuaRef r_load = luabridge::getGlobal(L_sctipt, "OnSourceLoad");
	if(r_load.isFunction())
	{
		r_load();
	}

	ipharases = translator->CreatePhraseCollection();
	ipharases->AddPhraseFile("ukrcoop.phrases");

	g_pSDKUkrCoop = &g_SDKUkrCoop_API;
	g_pShareSys->AddInterface(myself, g_pSDKUkrCoop);

	playerhelpers->AddClientListener(g_pRegen);

	if(g_pEntityList == nullptr)
    {
        g_pEntityList = (CBaseEntityList *)gamehelpers->GetGlobalEntityList();
    }

	if(!g_pGameConf->GetOffset("WitchAttackCharaster", &g_WitchACharasterOffset))
	{
		g_WitchACharasterOffset = -1;
	}

	SetupHooks();
	
	for(int b = 0; b < m_ListCall.Count(); b++)
	{
		m_ListCall[b]->OnLoad();
	}

	g_pSM->LogMessage(myself, "UkrCoop is load, Version \"%s\"", SMEXT_CONF_VERSION);
	return true;
}

void Sample::SDK_OnUnload(void)
{
	static luabridge::LuaRef r_Unload = luabridge::getGlobal(L_sctipt, "OnSourceUnload");
	if(r_Unload.isFunction())
	{
		r_Unload();
	}

	if(g_pOnCreateStandartEntity)
	{
		g_pOnCreateStandartEntity->Destroy();
		g_pOnCreateStandartEntity = nullptr;
	}

	if(g_OnGSClientDeny)
	{
		g_OnGSClientDeny->Destroy();
		g_OnGSClientDeny = nullptr;
	}

	if(g_BoneIndexByName)
	{
		g_BoneIndexByName->Destroy();
		g_BoneIndexByName = nullptr;
	}

	if(g_pCCommandBuffer_InsertCommand)
	{
		g_pCCommandBuffer_InsertCommand->Destroy();
		g_pCCommandBuffer_InsertCommand = nullptr;
	}

	if (WitchAttackCreate)
	{
		WitchAttackCreate->Destroy();
		WitchAttackCreate = nullptr;
	}

	g_TEManager.Shutdown();
	g_pCmdRunHook->Shutdown();

	gameconfs->CloseGameConfigFile(g_pGameConf);
#ifdef IS_VOTE_ENABLE
	forwards->ReleaseForward(g_pOnRegisterVote);
#endif
	g_pConVar->RemoveConVarChanged("z_max_player_zombies", &pHookChagedMaxPlayerZombies);
	g_pConVar->RemoveConVarChanged("ukr_player_zombi", &pHookChagedMaxPlayerZombies);
	g_pConVar->RemoveConVarChanged("survivor_limit", &pHookChagedSurvivorLimit);
	g_pConVar->RemoveConVarChanged("ukr_survivor_limit", &pHookChagedSurvivorLimit);

	playerhelpers->RemoveClientListener(g_pRegen);

	extern void StopActionProcessing();
	StopActionProcessing();

	for(int b = 0; b < m_ListCall.Count(); b++)
		m_ListCall[b]->OnUnload();
}

bool Sample::RegisterConCommandBase(ConCommandBase* pVar)
{
	return META_REGCVAR(pVar);
}

void Sample::OnEntityCreated(CBaseEntity *pEntity, const char *classname)
{
	static luabridge::LuaRef r_EntCreate = luabridge::getGlobal(L_sctipt, "OnEntCreate");
	if(r_EntCreate.isFunction())
	{
		r_EntCreate(pEntity, classname);
	}

	for(int b = 0; b < m_ListCall.Count(); b++)
	{
		m_ListCall[b]->OnEntityCreated((IBaseEntity*)pEntity, classname);
	}
}

void Sample::OnEntityDestroyed(CBaseEntity *pEntity)
{
	static luabridge::LuaRef r_EntDestroy = luabridge::getGlobal(L_sctipt, "OnEntDestroy");
	if(r_EntDestroy.isFunction())
	{
		r_EntDestroy(pEntity);
	}

	if(!ListCallThink.empty())
	{
		LuaListThink *pList = nullptr;

		SourceHook::List<LuaListThink>::iterator iter = ListCallThink.begin();
		while(iter != ListCallThink.end())
		{
			pList = &(*iter);
			if(*pList == pEntity)
			{
				if(pList->GetHookPostId() > 0)
				{
					SH_REMOVE_HOOK_ID(pList->GetHookPostId());
				}
				if(pList->GetHookPreId() > 0)
				{
					SH_REMOVE_HOOK_ID(pList->GetHookPreId());
				}
				ListCallThink.erase(iter);
			}
			iter++;
		}
	}

	for(int b = 0; b < m_ListCall.Count(); b++)
	{
		m_ListCall[b]->OnEntityDestroyed((IBaseEntity*)pEntity);
	}
}

bool Sample::OnLevelInit(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f)
{
	this->OnMapStart(pMapName);
	static luabridge::LuaRef r_mapStart = luabridge::getGlobal(L_sctipt, "OnMapStart");
	if(r_mapStart.isFunction())
	{
		r_mapStart(pMapName);
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool Sample::OnClientConnect(edict_t *pEdict, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
{
	int client = this->IndexOfEdict(pEdict);
	timersys->CreateTimer(&m_Timers, 35.0f, (void *)client, TIMER_FLAG_NO_MAPCHANGE|TIMER_FLAG_REPEAT);
	
	IGamePlayer* pPlayer = nullptr;
	if((pPlayer = playerhelpers->GetGamePlayer(client)) != nullptr)
	{
		char ip[24], *prt;
		strncopy(ip, pszAddress, sizeof(ip));
		if((prt = strchr(ip, ':')) != NULL)
		{
		    *prt = '\0';
		}

		char formatName[MAX_PLAYER_NAME_LENGTH + 1];
		memset(formatName, 0, sizeof(formatName));
		FormatName(pPlayer->GetName(), formatName);

		m_sChatLog->ChatLogMsg("[%s] %-35s has joined(%s | %s | %d)", g_pGeoIP->GetGeoIPCode3(ip), formatName, pPlayer->GetSteam2Id(false), ip, client);
	}

	static luabridge::LuaRef rClientCon = luabridge::getGlobal(L_sctipt, "OnClientConnected");
	if(rClientCon.isFunction())
	{
		rClientCon(client);
	}

	return true;
}

void Sample::OnClientDisconnect(edict_t *pEdict)
{
	int client = this->IndexOfEdict(pEdict);

	static luabridge::LuaRef rClientDisconnect = luabridge::getGlobal(L_sctipt, "OnClientDisconnected");
	if(rClientDisconnect.isFunction())
	{
		rClientDisconnect(client);
	}
}

void Sample::OnClientCommand(edict_t *pEdict, const CCommand &args)
{
	int index = this->IndexOfEdict(pEdict);
	IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(index);

	if(pPlayer == nullptr || !pPlayer->IsConnected())
	{
		return;
	}

	ResultType oldResult = Pl_Continue;
	for(int b = 0; b < m_ListCall.Count(); b++)
	{
		ResultType newResult = m_ListCall[b]->OnClientCommand(index, args);
		if(newResult == Pl_Handled) {
			oldResult = Pl_Handled;
		} else if(newResult == Pl_Stop) {
			oldResult = Pl_Stop;
		}
	}

	if(oldResult >= Pl_Handled) {
		RETURN_META(MRES_SUPERCEDE);
	}
}

void Sample::SetupHooks(void)
{
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	g_pOnCreateStandartEntity = DETOUR_CREATE_MEMBER(CTerrorGameRules_CreateStandartEntitys, "CTerrorGameRules__CreateStandardEntities");
	if(g_pOnCreateStandartEntity)
	{
		g_pOnCreateStandartEntity->EnableDetour();
	} else {
		g_pSM->LogError(myself, "Cannot find signature of \"CTerrorGameRules__CreateStandardEntities\"");
	}

	g_OnGSClientDeny = DETOUR_CREATE_MEMBER(OnGSClientDeny, "CSteam3Server_OnGSClientDeny");
	if(g_OnGSClientDeny)
	{
		g_OnGSClientDeny->EnableDetour();
	} else {
		g_pSM->LogError(myself, "Cannot find signature of \"CSteam3Server_OnGSClientDeny\"");
	}

	g_BoneIndexByName = DETOUR_CREATE_STATIC(BoneIndexByName, "Studio_BoneIndexByName");
	if(g_BoneIndexByName)
	{
		g_BoneIndexByName->EnableDetour();
	} else {
		g_pSM->LogError(myself, "Cannot find signature of \"Studio_BoneIndexByName\"");
	}

	g_pCCommandBuffer_InsertCommand = DETOUR_CREATE_MEMBER(CCommandBuffer__InsertCommand, "CCommandBuffer::InsertCommand");
	if(g_pCCommandBuffer_InsertCommand)
	{
		g_pCCommandBuffer_InsertCommand->EnableDetour();
	} else {
		g_pSM->LogError(myself, "Cannot find signature of \"CCommandBuffer::InsertCommand\"");
	}

	WitchAttackCreate = DETOUR_CREATE_MEMBER(WitchAttack__WitchAttack, "WitchAttack::WitchAttack");
	if (WitchAttackCreate)
	{
		WitchAttackCreate->EnableDetour();
	} else {
        g_pSM->LogError(myself,"Cannot find signature of \"WitchAttack::WitchAttack\"");
	}

	return;
}

bool Sample::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	static luabridge::LuaRef rMetamodUnload = luabridge::getGlobal(L_sctipt, "OnMetamodUnload");
	if(rMetamodUnload.isFunction())
	{
		rMetamodUnload();
	}

	g_pSDKHooks->RemoveEntityListener(this);
	
	ConVar_Unregister();

	return this->HookRemove();
}

void Sample::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);
	if(!g_pBinTools)
	{
		return;
	}

	SM_GET_LATE_IFACE(GEOIP, g_pGeoIP);
	if(!g_pGeoIP)
	{
        return;
	}

	SM_GET_LATE_IFACE(SDKHOOKS, g_pSDKHooks);
	if(!g_pSDKHooks)
	{
		return;
	}

	g_pSDKHooks->AddEntityListener(this);
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);
	if(!g_pSDKTools)
	{
		return;
	}

	SM_GET_LATE_IFACE(TOPMENUS, g_pTopMenu);
	if(!g_pTopMenu)
	{
		return;
	}

	g_TEManager.Initialize();
	g_pCmdRunHook->Initialize();

	g_pServer = g_pSDKTools->GetIServer();

	static luabridge::LuaRef rAllLoad = luabridge::getGlobal(L_sctipt, "OnAllLoaded");
	if(rAllLoad.isFunction())
	{
		rAllLoad();
	}

	for(int b = 0; b < m_ListCall.Count(); b++)
		m_ListCall[b]->OnAllLoaded();
}

bool Sample::QueryRunning(char *error, size_t maxlength)
{
	SM_CHECK_IFACE(BINTOOLS, g_pBinTools);
	SM_CHECK_IFACE(GEOIP,	 g_pGeoIP);
	SM_CHECK_IFACE(SDKHOOKS, g_pSDKHooks);
	SM_CHECK_IFACE(SDKTOOLS, g_pSDKTools);
	SM_CHECK_IFACE(TOPMENUS, g_pTopMenu);

	return true;
}

#include "IBaseBans.h"
void Sample::NotifyInterfaceDrop(SMInterface *pInterface)
{
	g_TEManager.Shutdown();
	
	if(g_pBanDB != nullptr)
	{
		if(pInterface == (void*)g_pBanDB->GetDriver())
		{
			g_pBanDB->Release();
		}
	}
}

bool Sample::QueryInterfaceDrop(SMInterface *pInterface)
{
	auto InterfaceDrop = [&pInterface](void *pDrop)
	{
		if(pInterface == pDrop)
		{
			return true;
		}
		return false;
	};

	if(InterfaceDrop(g_pBinTools))
	{
		return false;
	}

	if(InterfaceDrop(g_pGeoIP))
	{
        return false;
	}

	if(InterfaceDrop(g_pSDKHooks))
	{
		return false;
	}

	if(InterfaceDrop(g_pSDKTools))
	{
		return false;
	}

	if(InterfaceDrop(g_pTopMenu))
	{
		return false;
	}

	if(g_pBanDB != nullptr)
	{
		if(InterfaceDrop(g_pBanDB->GetDriver()))
		{
			return false;
		}
	}

	return IExtensionInterface::QueryInterfaceDrop(pInterface);
}

bool Sample::HookRemove()
{
	SH_REMOVE_HOOK(IServerGameDLL, 		LevelInit, 					gamedll, 		SH_MEMBER(this, &Sample::OnLevelInit), false);
	SH_REMOVE_HOOK(IServerGameDLL, 		GameFrame, 					gamedll, 		SH_MEMBER(g_pRegen, &TickRegen::GameFrame), true);
	SH_REMOVE_HOOK(IServerGameDLL, 		LevelShutdown, 				gamedll, 		SH_MEMBER(g_pRegen, &TickRegen::LevelShutdown), false);
	SH_REMOVE_HOOK(IServerGameClients, 	ClientConnect, 				serverClients, 	SH_MEMBER(this, &Sample::OnClientConnect), false);
	SH_REMOVE_HOOK(IServerGameClients, 	ClientDisconnect, 			serverClients, 	SH_MEMBER(this, &Sample::OnClientDisconnect), false);
	SH_REMOVE_HOOK(IServerGameClients, 	SetCommandClient, 			serverClients, 	SH_MEMBER(this, &Sample::SetComandClient), false);
	SH_REMOVE_HOOK(IServerGameClients, 	ClientPutInServer, 			serverClients, 	SH_MEMBER(g_pRegen, &TickRegen::OnClientConnect), false);
	SH_REMOVE_HOOK(IServerGameClients, 	ClientPutInServer, 			serverClients, 	SH_MEMBER(g_pCmdRunHook, &CCmdRunHook::OnClientPutInServer), true);
	SH_REMOVE_HOOK(IServerGameClients, 	ClientCommand, 				serverClients, 	SH_MEMBER(this, &Sample::OnClientCommand), false);
	SH_REMOVE_HOOK(ICvar, 				CallGlobalChangeCallbacks, 	icvar, 			SH_STATIC(ConVar_l4d::OnConVarGanger), false);

	if(m_pSayCmd)
	{	
		SH_REMOVE_HOOK(ConCommand, Dispatch, m_pSayCmd, SH_STATIC(OnSayChat), false);
		m_pSayCmd = nullptr;
	}

	if(m_pSayTeamCmd)
	{
		SH_REMOVE_HOOK(ConCommand, Dispatch, m_pSayTeamCmd, SH_STATIC(OnSayTeamChat), false);
		m_pSayTeamCmd = nullptr;
	}

	if (m_pExecPtr)
	{
		SH_REMOVE_HOOK(ConCommand, Dispatch, m_pExecPtr, SH_STATIC(OnExecDispatchPre), false);
		SH_REMOVE_HOOK(ConCommand, Dispatch, m_pExecPtr, SH_STATIC(OnExecDispatchPost), true);
		m_pExecPtr = nullptr;
	}

	RELEASE(m_sLog)
	RELEASE(g_HL2)
	RELEASE(g_pRegen)
	RELEASE(g_pConVar)
	RELEASE(m_sChatLog)
	RELEASE(g_pEventList)

	return true;
}

void Sample::SetComandClient(int client)
{
	m_CmdClient = client + 1;
}

bool b_GotTrigger = false;
bool b_ServerExec = false;

void Sample::OnExecDispatchPre(const CCommand &args)
{
	const char *arg = args.Arg(1);
	if(arg != NULL)
	{
		static ConVar* _convar = g_pCVar->FindVar("servercfgfile");
		if(_convar)
		{
			if(!b_ServerExec && g_Sample.my_strcmp(arg, _convar->GetString()) == 0)
			{
				b_GotTrigger = true;
			}
		}
	}
}

void Sample::OnExecDispatchPost(const CCommand &args)
{
	if(b_GotTrigger)
	{
		b_GotTrigger = false;
		b_ServerExec = true;
		g_Sample.OnConfigExec();
	}
}

void Sample::RegisterCallBackList(CAppSystem *pCallBack)
{
	m_ListCall.AddToTail(pCallBack);
}

void Sample::OnConfigExec()
{
	if(b_ServerExec)
	{
		InitPrecache();

		static luabridge::LuaRef rConf = luabridge::getGlobal(L_sctipt, "OnConfigsExec");
		if(rConf.isFunction())
		{
			rConf();
		}

		b_ServerExec = false;
	}
}

void Sample::EntityThinkPost()
{
	if(ListCallThink.empty())
	{
		RETURN_META(MRES_IGNORED);
	}

	auto iface = META_IFACEPTR(CBaseEntity);
	for(auto ListCall : ListCallThink)
	{
		if(ListCall == iface)
		{
			auto refList = ListCall.GetListPost();
			if(refList.empty())
			{
				break;
			}

			for(auto _Ref : refList)
			{
				_Ref(iface);
			}
			break;
		}
	}

	RETURN_META(MRES_IGNORED);
}

void Sample::EntityThinkPre()
{
	if(ListCallThink.empty())
	{
		RETURN_META(MRES_IGNORED);
	}

	auto iface = META_IFACEPTR(CBaseEntity);
	for(auto ListCall : ListCallThink)
	{
		if(ListCall == iface)
		{
			auto refList = ListCall.GetListPre();
			if(refList.empty())
			{
				break;
			}

			for(auto _Ref : refList)
			{
				_Ref(iface);
			}
			break;
		}
	}

	RETURN_META(MRES_IGNORED);
}

void Sample::OnSayTeamChat(const CCommand &args)
{
	int client = g_Sample.GetComandsClient();
	const char* argss = args.ArgS();
	if(!argss) {
		RETURN_META(MRES_IGNORED);
	}

	if(g_Sample.ClientOnSayChat(argss, client) >= PL_Handled)
	{
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void Sample::OnSayChat(const CCommand &args)
{
	int client = g_Sample.GetComandsClient();
	const char* argss = args.ArgS();
	if(!argss) {
		RETURN_META(MRES_IGNORED);
	}

	if(g_Sample.ClientOnSayChat(argss, client, false) >= PL_Handled)
	{
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

Results Sample::ClientOnSayChat(const char* msg, const int client, bool team)
{
	const char *szTeamName = nullptr;
	if(client == 0)
	{
		szTeamName = GetTeamName(client);
        m_sChatLog->ChatLogMsg("[%-10s] %-35s: %s", szTeamName, "Console", msg);
        return PL_Continue;
    }

	IGamePlayer* pPlayer = playerhelpers->GetGamePlayer(client);
	if(!pPlayer || !pPlayer->IsConnected())
	{
		return PL_Continue;
	}

	char formatName[MAX_PLAYER_NAME_LENGTH + 1];
	memset(formatName, 0, sizeof(formatName));
	FormatName(pPlayer->GetName(), formatName);

	if(!szTeamName)
		szTeamName = GetTeamName(client);

	if(team)
	{
		m_sChatLog->ChatLogMsg("[%-10s] %-35s:(TEAM) %s", szTeamName, formatName, msg);
	} else {
		m_sChatLog->ChatLogMsg("[%-10s] %-35s: %s", szTeamName, formatName, msg);
	}
	return PL_Continue;
}

void Sample::OnMapStart(const char *MapName)
{
	InitPrecache();

	m_sLog->LogToFileEx(false, "[UkrCoop]----------> MapChange \"%s\" \"%s\" <----------", g_pConVar->GetConVarString("mp_gamemode"), MapName);
	m_sChatLog->ChatLogMsg("[UkrCoop]----------> MapChange \"%s\" \"%s\" <----------", g_pConVar->GetConVarString("mp_gamemode"), MapName);
}

CAppSystem::CAppSystem()
{
	g_Sample.RegisterCallBackList(this);
}

bool CAppSystem::IsAdminAccess(AdminId adm, FlagBits cmdflags)
{
	if(adm != INVALID_ADMIN_ID)
	{
		FlagBits bits = adminsys->GetAdminFlags(adm, Access_Effective);
		if((bits & ADMFLAG_ROOT) == ADMFLAG_ROOT)
		{
			return true;
		}
		if((bits & cmdflags) == cmdflags)
		{
			return true;
		}
	}
	return false;
}
