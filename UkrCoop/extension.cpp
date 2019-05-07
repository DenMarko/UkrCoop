#include "extension.h"
#include "chat_log.h"
#include "ConVars.h"
#include "CDetour\detours.h"
#include "inetchannelinfo.h"
#include "TickRegen.h"
#include "log_messege.h"
#include "HookEvent.h"

CHookEvent		g_pHookEvent;
LM				m_sLog;
chat_log		m_sChatLog;
HL2				g_HL2;
Ukr_coop		g_pUkrCoop;
ConVar_l4d		g_pConVar;
TickRegen		g_pRegen;
SMEXT_LINK(&g_pUkrCoop);

IServerGameClients	*serverClients = nullptr;
IPhraseCollection	*ipharases = nullptr;
IServerTools		*servertools = nullptr;
ICvar				*icvar = nullptr;
IGameEventManager2	*gameevents = nullptr;
IBinTools			*g_pBinTools = nullptr;
IGeoIP				*g_pGeoIP = nullptr;
IGameConfig			*g_pGameConf = nullptr;
CDetour				*g_pWintchAttackCreater = nullptr;
CDetour				*g_pWintchAttackGetVictim = nullptr;
CGlobalVars			*g_pGlobals;
UkrCoop				*g_pSDKUkrCoop;
IServerGameEnts		*gameents = nullptr;

int g_WitchACharacterOffeset = -1;

class SDKUkrCoop : public UkrCoop
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
	virtual int UkrCoop_GetEntPropEnt(cell_t entity, PropTypes type, const char *prop, int element)
	{
		return g_HL2.GetEntPropEnt(entity, type, prop, element);
	}
	virtual bool UkrCoop_SetEntPropEnt(cell_t entity, PropTypes type, const char* prop, cell_t other, int element)
	{
		if (g_HL2.SetEntPropEnt(entity, type, prop, other, element))
			return true;
		return false;
	}
	virtual int UkrCoop_GetEntProp(cell_t entity, PropTypes type, const char *prop, int size = 4, int element = 0)
	{
		return g_HL2.GetEntProp(entity, type, prop, size, element);
	}
	virtual bool UkrCoop_SetEntProp(cell_t entity, PropTypes type, const char *prop, int value, int size = 4, int element = 0)
	{
		if (g_HL2.SetEntProp(entity, type, prop, value, size, element))
			return true;
		return false;
	}
	virtual void UkrCoop_Stargget(int client, int target, Vector *sVector)
	{
		if ((client >= 1) && (client <= playerhelpers->GetMaxClients()))
		{
			IGamePlayer *I_Client = playerhelpers->GetGamePlayer(client);
			if (!I_Client)
				return m_sLog.LogToFileEx("Client index %d is not valid", client);
	
				if (!I_Client->IsConnected() && !I_Client->IsInGame())
					return m_sLog.LogToFileEx("Client is not connect or is not in game");
		}
		else if (client < 0 || client > g_pGlobals->maxEntities)
		{
			return m_sLog.LogToFileEx("Entity index %d is not valid", client);
		}
		else
		{
			edict_t *pEdict = g_pUkrCoop.PEntityOfEntIndex(client);
			if (!pEdict || pEdict->IsFree())
				return m_sLog.LogToFileEx("Entity %d is not valid or is freed", client);
		}

		if ((target >= 1) && (target <= playerhelpers->GetMaxClients()))
		{
			IGamePlayer *I_Client = playerhelpers->GetGamePlayer(target);
			if (!I_Client)
				return m_sLog.LogToFileEx("Client index %d is not valid", target);

			if (!I_Client->IsConnected() && !I_Client->IsInGame())
				return m_sLog.LogToFileEx("Client is not connect or is not in game");
		}
		else if (target < 0 || target > g_pGlobals->maxEntities)
		{
			return m_sLog.LogToFileEx("Entity index %d is not valid", target);
		}
		else
		{
			edict_t *pEdict = g_pUkrCoop.PEntityOfEntIndex(target);
			if (!pEdict || pEdict->IsFree())
				return m_sLog.LogToFileEx("Entity %d is not valid or is freed", target);
		}

		g_HL2.PlayerStartget(	g_pUkrCoop.PEntityOfEntIndex(client)->GetUnknown()->GetBaseEntity(),
								g_pUkrCoop.PEntityOfEntIndex(target)->GetUnknown()->GetBaseEntity(),
								sVector);
	}
	virtual bool UkrCoop_PlayerMsg(int client, DEST type, const char *msg, ...)
	{
		char buffer[2048];
		va_list ap;

		va_start(ap, msg);
		vsnprintf(buffer, sizeof(buffer), msg, ap);
		g_HL2.TextMsg(client, type, buffer);
		va_end(ap);
	}
	virtual void UkrCoop_LogMessage(const char *msg, ...)
	{
		char buffer[2048];
		va_list ap;

		va_start(ap, msg);
		vsnprintf(buffer, sizeof(buffer), msg, ap);
		m_sLog.LogToFileEx(buffer);
		va_end(ap);
	}
	virtual void UkrCoop_PlayerRespawn(edict_t *pEdict)
	{
		g_HL2.PlayerRespawn(pEdict->GetUnknown()->GetBaseEntity());
	}
} g_SDKUkrCoop_API;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, false, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK5(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, edict_t *, const char *, const char *, char *, int);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, false, const CCommand &);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, false, bool);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, false);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, const char *);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, false, bool, IGameEvent *, bool);
SH_DECL_HOOK0(IServerGameDLL, GetTickInterval, const, 0, float);

DETOUR_DECL_MEMBER1(WitchAttack__WitchAttack, void*, CBaseEntity*, pEntity)
{
	int client = gamehelpers->IndexOfEdict(gameents->BaseEntityToEdict((CBaseEntity *)pEntity));

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
	if(pPlayer)
	{
		m_sLog.LogToFileEx("WitchAttack create for client %s", pPlayer->GetName());
	} else {
		m_sLog.LogToFileEx("WitchAttack create for %d entity", client);
	}
	
	void *result = DETOUR_MEMBER_CALL(WitchAttack__WitchAttack)(pEntity);
	DWORD *CharId = ((DWORD *)this + g_WitchACharacterOffeset);

	m_sLog.LogToFileEx("WitchAttack CharId = %d", *CharId);

	*CharId = 4;
	return result;
}

DETOUR_DECL_MEMBER0(WitchAttack__GetVictim, void*)
{
	void *res = DETOUR_MEMBER_CALL(WitchAttack__GetVictim)();

	DWORD *CharId = ((DWORD *)this + g_WitchACharacterOffeset);
	*CharId = 4;

	return res;
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

		char msg[256];
		g_HL2.Translate(msg, sizeof(msg), "%T", 2, NULL, "Visit_grup", &client);
		g_HL2.TextMsg(client, DEST::CHAT, msg);

		return Pl_Stop;
	}
	void OnTimerEnd(ITimer* pTimer, void* pData){}
} m_Timers;

bool Ukr_coop::SDK_OnMetamodLoad(SourceMM::ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_ANY(GetServerFactory, gamedll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, serverClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	g_pCVar = icvar;
	ConVar_Register(0, this);

	g_pGlobals = ismm->GetCGlobals();

	new ConVar("ukr_coop_version", SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY, "UKRCOOP Version");

	return this->HookInit(error, maxlen);
}

bool _cdecl RountStart(IGameEvent *pEvent, bool bBroadcast)
{
	g_pRegen.SetRoundStart(true);
	return true;
}

bool _cdecl RountEnd(IGameEvent *pEvent, bool bBroadcast)
{
	g_pRegen.SetRoundStart(false);
	return true;
}

float GetTickInterval()
{
	float tickinterval = (1.0f / 30.0f);

	const float tickrate = 200;
	if (tickrate > 10)
		tickinterval = 1.0f / tickrate;

	RETURN_META_VALUE(MRES_SUPERCEDE, tickinterval);
}

bool Ukr_coop::HookInit(char *error, size_t maxlen){
	SH_ADD_HOOK(IServerGameClients, ClientConnect, serverClients, SH_MEMBER(this, &Ukr_coop::OnClientConnect), false);
	SH_ADD_HOOK(IServerGameClients, SetCommandClient, serverClients, SH_MEMBER(this, &Ukr_coop::SetComandClient), false);
	SH_ADD_HOOK(IServerGameClients, ClientDisconnect, serverClients, SH_MEMBER(this, &Ukr_coop::OnClientDisconnect), false);
	SH_ADD_HOOK(IServerGameClients, ClientPutInServer, serverClients, SH_MEMBER(&g_pRegen, &TickRegen::OnClientConnect), false);
	SH_ADD_HOOK(IServerGameDLL, LevelInit, gamedll, SH_MEMBER(this, &Ukr_coop::OnLevelInit), false);
	SH_ADD_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(&g_pRegen, &TickRegen::GameFrame), false);
	SH_ADD_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_MEMBER(&g_pRegen, &TickRegen::LevelShutdown), false);
	SH_ADD_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(&g_pHookEvent, &CHookEvent::OnFireEvent), false);
	SH_ADD_HOOK(IServerGameDLL, GetTickInterval, gamedll, SH_STATIC(GetTickInterval), false);

	m_pSayCmd = this->FindComands("say");

	if(m_pSayCmd){
		SH_ADD_HOOK(ConCommand, Dispatch, m_pSayCmd, SH_STATIC(OnSayChat), false);
	} else {
		this->UTIL_Format(error, maxlen, "[Error] Say Find Commands is Failed!");
		return false;
	}

	m_pSayTeamCmd = this->FindComands("say_team");

	if(m_pSayTeamCmd){
		SH_ADD_HOOK(ConCommand, Dispatch, m_pSayTeamCmd, SH_STATIC(OnSayTeamChat), false);
	} else {
		this->UTIL_Format(error, maxlen, "[Error] Say_Team Find Commands is Failed!");
		return false;
	}

	g_pHookEvent.HookEvent("round_start", &RountStart);
	g_pHookEvent.HookEvent("round_end", &RountEnd);
	g_pHookEvent.HookEvent("map_transition", &RountEnd);

	return true;
}

bool Ukr_coop::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if(strcmp(g_pSM->GetGameFolderName(), "left4dead") != 0){
		this->UTIL_Format(error, maxlength, "[Error] Cannot Load UkrCoop Ext on mods other than L4D");
		return false;
	}

	if (!gameconfs->LoadGameConfigFile("UkrCoop.l4d", &g_pGameConf, error, maxlength))
		return false;

	g_pGameConf->GetOffset("WitchAttackCharaster", &g_WitchACharacterOffeset);

	sharesys->AddDependency(myself, "bintools.ext", true, true);
	sharesys->AddDependency(myself, "geoip.ext", true, true);

	g_pShareSys->AddNatives(myself, MyNative);

	g_HL2.SetMsgId(usermsgs->GetMessageIndex("HintText"), usermsgs->GetMessageIndex("TextMsg"));
	m_sLog.InitLogMesseg();
	m_sChatLog.InitChatLog();

	ipharases = translator->CreatePhraseCollection();
	ipharases->AddPhraseFile("ukrcoop.phrases");

	g_pSDKUkrCoop = &g_SDKUkrCoop_API;
	g_pShareSys->AddInterface(myself, g_pSDKUkrCoop);

	if(!SetupHooks())
		return false;

	g_pSM->LogMessage(myself, "Plugins is loads, Version %s", SMEXT_CONF_VERSION);
	return true;
}

void Ukr_coop::SDK_OnUnload(void)
{
	RemoveHooks();
	gameconfs->CloseGameConfigFile(g_pGameConf);
}

bool Ukr_coop::RegisterConCommandBase(ConCommandBase* pVar){
	return META_REGCVAR(pVar);
}

const char *Ukr_coop::GetExtensionVerString(){
	return SMEXT_CONF_VERSION;
}

const char *Ukr_coop::GetExtensionDateString(){
	return SMEXT_CONF_DATESTRING;
}

bool Ukr_coop::SetupHooks(void)
{
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	g_pWintchAttackCreater = DETOUR_CREATE_MEMBER(WitchAttack__WitchAttack, "WitchAttack::WitchAttack");
	g_pWintchAttackGetVictim = DETOUR_CREATE_MEMBER(WitchAttack__GetVictim, "WitchAttack::GetVictim");
	if (g_pWintchAttackCreater)
	{
		g_pWintchAttackCreater->EnableDetour();
		return true;
	}

	if (g_pWintchAttackGetVictim)
	{
		g_pWintchAttackGetVictim->EnableDetour();
		return true;
	}

	g_pSM->LogError(myself, "Cannot find signature of WitchAttack::WitchAttack");
	RemoveHooks();
	return false;
}

void Ukr_coop::RemoveHooks(void)
{
	if (g_pWintchAttackCreater)
	{
		g_pWintchAttackCreater->Destroy();
		g_pWintchAttackCreater = nullptr;
	}

	if (g_pWintchAttackGetVictim)
	{
		g_pWintchAttackGetVictim->Destroy();
		g_pWintchAttackGetVictim = nullptr;
	}
}

bool Ukr_coop::OnLevelInit(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f){
	this->OnMapStart(pMapName);
	RETURN_META_VALUE(MRES_IGNORED, true);
}

//unsigned int strncopy(char *dest, const char *src, size_t count)
//{
//	if (!count)
//	{
//		return 0;
//	}
//
//	char *start = dest;
//	while ((*src) && (--count))
//	{
//		*dest++ = *src++;
//	}
//	*dest = '\0';
//
//	return (dest - start);
//}

bool Ukr_coop::OnClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
{
	const auto strncopy = [](char *dest, const char *src, size_t count)
	{
		if (!count)
		{
			return 0;
		}

		char *start = dest;
		while ((*src) && (--count))
		{
			*dest++ = *src++;
		}
		*dest = '\0';

		return (dest - start);
	};

	int client = this->IndexOfEdict(pEntity);
	IGamePlayer* pPlayer = playerhelpers->GetGamePlayer(client);
	timersys->CreateTimer(&m_Timers, 35.0f, (LPVOID)client, TIMER_FLAG_NO_MAPCHANGE|TIMER_FLAG_REPEAT);
	if(pPlayer){
		char ip[24];
		strncopy(ip, pszAddress, sizeof(ip));
		strchr(ip, ':');

		m_sChatLog.ChatLogMsg("[%s] %-35s has joined(%s | %s)", g_pGeoIP->GetGeoIPCode3(ip), pPlayer->GetName(), pPlayer->GetSteam2Id(false), ip);
	}
	return true;
}

void Ukr_coop::OnClientDisconnect(edict_t *pEntity)
{
	g_pRegen.OnClientDisconnect(this->IndexOfEdict(pEntity));
}

bool Ukr_coop::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	return this->HookRemove();
}

void Ukr_coop::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);
	if(!g_pBinTools)
		return;

	SM_GET_LATE_IFACE(GEOIP, g_pGeoIP);
	if(!g_pGeoIP)
		return;
}

bool Ukr_coop::QueryRunning(char *error, size_t maxlength)
{
	SM_CHECK_IFACE(BINTOOLS, g_pBinTools);
	SM_CHECK_IFACE(GEOIP, g_pGeoIP);
	return true;
}

bool Ukr_coop::QueryInterfaceDrop(SMInterface *pInterface)
{
	if(pInterface == g_pBinTools)
		return false;
	if(pInterface == g_pGeoIP)
		return false;

	return IExtensionInterface::QueryInterfaceDrop(pInterface);
}

bool Ukr_coop::HookRemove()
{
	SH_REMOVE_HOOK(IServerGameDLL, LevelInit, gamedll, SH_MEMBER(this, &Ukr_coop::OnLevelInit), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientConnect, serverClients, SH_MEMBER(this, &Ukr_coop::OnClientConnect), false);
	SH_REMOVE_HOOK(IServerGameClients, SetCommandClient, serverClients, SH_MEMBER(this, &Ukr_coop::SetComandClient), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientDisconnect, serverClients, SH_MEMBER(this, &Ukr_coop::OnClientDisconnect), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientPutInServer, serverClients, SH_MEMBER(&g_pRegen, &TickRegen::OnClientConnect), false);
	SH_REMOVE_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(&g_pRegen, &TickRegen::GameFrame), false);
	SH_REMOVE_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_MEMBER(&g_pRegen, &TickRegen::LevelShutdown), false);
	SH_REMOVE_HOOK(ConCommand, Dispatch, m_pSayCmd, SH_STATIC(OnSayChat), false);
	SH_REMOVE_HOOK(ConCommand, Dispatch, m_pSayTeamCmd, SH_STATIC(OnSayTeamChat), false);
	SH_REMOVE_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(&g_pHookEvent, &CHookEvent::OnFireEvent), false);
	SH_REMOVE_HOOK(IServerGameDLL, GetTickInterval, gamedll, SH_STATIC(GetTickInterval), false);

	return true;
}

void Ukr_coop::SetComandClient(int client)
{
	m_CmdClient = ++client;
}

void Ukr_coop::OnSayTeamChat(const CCommand &args)
{
	int client = g_pUkrCoop.GetComandsClient();
	const char* argss = args.ArgS();
	if(!argss)
		RETURN_META(MRES_IGNORED);

	if(g_pUkrCoop.ClientOnSayChat(argss, client, true) >= PL_Handled)
		RETURN_META(MRES_SUPERCEDE);

	RETURN_META(MRES_IGNORED);
}

void Ukr_coop::OnSayChat(const CCommand &args)
{
	int client = g_pUkrCoop.GetComandsClient();
	
	const char* argss = args.ArgS();
	if(!argss)
		RETURN_META(MRES_IGNORED);

	if(g_pUkrCoop.ClientOnSayChat(argss, client, false) >= PL_Handled)
		RETURN_META(MRES_SUPERCEDE);

	RETURN_META(MRES_IGNORED);
}

Resultat Ukr_coop::ClientOnSayChat(const char* msg, const int client, const bool team)
{
	if(client == 0){
		m_sChatLog.ChatLogMsg("[%-9s] %-35s: %s", this->GetTeamName(client), "Console", msg);
		return PL_Continue;
	}
	
	IGamePlayer* pPlayer = playerhelpers->GetGamePlayer(client);
	if(!pPlayer){
		return PL_Continue;
	}

	IPlayerInfo *pInfo = pPlayer->GetPlayerInfo();
	if(!pInfo){
		return PL_Continue;
	}

	if(team){
		m_sChatLog.ChatLogMsg("[%-9s] %-35s:(TEAM) %s", this->GetTeamName(pInfo->GetTeamIndex()), pPlayer->GetName(), msg);
	} else {
		m_sChatLog.ChatLogMsg("[%-9s] %-35s: %s", this->GetTeamName(pInfo->GetTeamIndex()), pPlayer->GetName(), msg);
	}	
	return PL_Continue;
}

void Ukr_coop::OnMapStart(const char *MapName)
{
	m_sLog.LogToFileEx("[UkrCoop]----------> MapChange \"%s\" GameMode \"%s\" <----------", MapName, g_pConVar.GetConVarString("mp_gamemode"));
	m_sChatLog.ChatLogMsg("[UkrCoop]----------> MapChange \"%s\" GameMode \"%s\" <----------", MapName, g_pConVar.GetConVarString("mp_gamemode"));
}

static cell_t LogMessegeToFile(IPluginContext *pContext, const cell_t *params)
{
	g_pSM->SetGlobalTarget(0);
	char meseg[1024];

	g_pSM->FormatString(meseg, sizeof(meseg), pContext, params, 1);

	if(pContext->GetLastNativeError() != 0){
		return 0;
	}
	IPlugin *pPlugins = plsys->FindPluginByContext(pContext->GetContext());
	m_sLog.LogToFileEx("[%s] %s", pPlugins->GetFilename(), meseg);
	return 0;
}

static cell_t BotCreater(IPluginContext *pContext, const cell_t *params)
{
	edict_t *pEdict = engine->CreateFakeClient("New_Bot");
	if(!pEdict){
		return pContext->ThrowNativeError("Create Fake Client is failet!!");
	}

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(pEdict);
	if(!pPlayer){
		return pContext->ThrowNativeError("pEndict is failed!!");
	} else if(!pPlayer->IsInGame()){
		return pContext->ThrowNativeError("Client is not in game!!");
	}

	IPlayerInfo *pInfo = pPlayer->GetPlayerInfo();
	if(!pInfo){
		return pContext->ThrowNativeError("-IPlayerInfo- is invalid!!");
	}

	pInfo->ChangeTeam(2);
	CBaseEntity *pEntity = pEdict->GetUnknown()->GetBaseEntity();
	if(!servertools->SetKeyValue(pEntity, "classname", "SurvivorBot")){
		return pContext->ThrowNativeError("-SetKeyValue- failed!!!");
	}

	servertools->DispatchSpawn(pEntity);
	pPlayer->Kick("Survivor bot left the survivor team!");
	return 1;
}

static cell_t PRespawn(IPluginContext *pContext, const cell_t *params)
{
	int client = params[1];
	
	IS_VALID_CLIENT(client);

	g_HL2.PlayerRespawn(g_pUkrCoop.PEntityOfEntIndex(client)->GetUnknown()->GetBaseEntity());
	return 1;
}

static cell_t l4dStargget(IPluginContext *pContext, const cell_t *param)
{
	int client, target;
	cell_t *sVector;
	Vector *s_Vector = nullptr, Vector_s;

	client = param[1];
	target = param[2];
	pContext->LocalToPhysAddr(param[3], &sVector);

	if(sVector != pContext->GetNullRef(SP_NULL_VECTOR))
	{
		Vector_s[0] = sp_ctof(sVector[0]);
		Vector_s[1] = sp_ctof(sVector[1]);
		Vector_s[2] = sp_ctof(sVector[2]);
		s_Vector = &Vector_s;
	}

	IS_VALID_CLIENT(client);
	IS_VALID_CLIENT(target);
	
	g_HL2.PlayerStartget(	g_pUkrCoop.PEntityOfEntIndex(client)->GetUnknown()->GetBaseEntity(), 
							g_pUkrCoop.PEntityOfEntIndex(target)->GetUnknown()->GetBaseEntity(), 
							s_Vector);
	return 1;
}

static cell_t l4dVomitUpon(IPluginContext *pContext, const cell_t *param)
{
	int pClient = param[1];
	int aClient = param[2];

	IS_VALID_CLIENT(pClient);
	IS_VALID_CLIENT(aClient);

	g_HL2.PlayerVomitUpon(	g_pUkrCoop.PEntityOfEntIndex(pClient)->GetUnknown()->GetBaseEntity(),
							g_pUkrCoop.PEntityOfEntIndex(aClient)->GetUnknown()->GetBaseEntity(),
							param[3]);
	return 1;
}

const sp_nativeinfo_t MyNative[] =
{
	{"LogMessegToFile", LogMessegeToFile},
	{"BotCreater",		BotCreater},
	{"Respawn",			PRespawn},
	{"l4d_staggered",	l4dStargget},
	{"l4d_VomitUpon",	l4dVomitUpon},
	{NULL,				NULL},
};