#include "TickRegen.h"
#include "LuaBridge/LuaBridge.h"
#include "CCmdRunHook.h"
#include "Interface/ITerrorPlayer.h"

ConVar ukr_regen_tick(		"ukr_regen_tick", 		"1.0", 	0, "Time regen tick", 			true, 0.1f, true, 50.0f);
ConVar ukr_regen_hp(		"ukr_regen_hp", 		"1", 	0, "Regen of time hp", 			true, 1.0f, true, 150.0f);
ConVar ukr_regen_enable(	"ukr_regen_enable", 	"1", 	0, "Enable regen", 				true, 0.f, 	true, 1.0f);
ConVar ukr_regen_infected(	"ukr_regen_infected", 	"0", 	0, "Enable regen for infected", true, 0.f, 	true, 1.f);

luabridge::LuaRef *rGameFrame = nullptr;
luabridge::LuaRef *rClientPutInServer = nullptr;


/**
 * базовий клас Регену
 */
class CBaseRegenTick
{
public:
	CBaseRegenTick(bool bRoundStart) : IsRoundStart(bRoundStart) {}
	virtual ~CBaseRegenTick() {}

	/**
	 * якщо вертає true цикил продовжується до кінця списку, якщо false цикил останолюється 
	*/
	virtual bool operator() (CInfoClient *&pInfo) { return false; }

protected:
	/**
	 * Функція перевірки валідності клієнта
	 */
	virtual bool IsClientValid(int client) const
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
		IPlayerInfo* pInfo = nullptr;

		if(!pPlayer)											// якщо pPlayer рівний 0 вертвєм false
			return false;
		else if(!pPlayer->IsConnected())						// якщо клієнт не підключений вертаєм false
			return false;
		else if(!pPlayer->IsInGame())							// якщо клієнт підключений але не в грі вертвєм false
			return false;
		else if((pInfo = pPlayer->GetPlayerInfo()) == nullptr)	// якщо клієнт інфо не існує вертаєм false
			return false;
		else if(pInfo->IsDead())								// якщо клієнт мертвий то вертаєм false
			return false;
		else if(pInfo->GetTeamIndex() == 1)						// якщо клієнт в зрітілях вертвєм false 
			return false;

		return true;
	}
	bool IsRoundStart; // храним дані про раунд
};


/**
 * Клас Екстра Регену
 */
class CExtraTick : public CBaseRegenTick
{
public:
	CExtraTick(bool bRoundStart) : CBaseRegenTick(bRoundStart) {}

	/**
	 * Функція обробки клієнтів
	 */
	virtual bool operator() (CInfoClient *&pInfo) override {
		if(!pInfo->IsExtraExec()) {
			return true;
		}

		pInfo->SetNextExtraExecThink(pInfo->GetTExtra());
		if(!pInfo->IsExtra()) {
			return true;
		}

		if(!IsRoundStart) {
			return true;
		}

		if(!CBaseRegenTick::IsClientValid(pInfo->GetPlayerId())) {
			return true;
		}

		ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(pInfo->GetPlayerId());
		if(pPlayer->GetHealth() < pPlayer->GetMaxHealth()) {
			pPlayer->TakeHealth(static_cast<float>(pInfo->GetHp()), DMG_GENERIC);
		}
		return true;
	}
};

/**
 * Основий клас Регену
 */
class CRegenTick : public CBaseRegenTick
{
public:
	CRegenTick(bool bRoundStart) : CBaseRegenTick(bRoundStart) {}

	/**
	 * Функція обробки клієнтів
	 */
	virtual bool operator() (CInfoClient* &pInfo) override {
		if(!pInfo->IsExec()) {
			return true;
		}

		pInfo->SetNextToExecThink(ukr_regen_tick.GetFloat());
		if(pInfo->IsExtra()) {
			return true;
		}

		if(!IsRoundStart) {
			return true;
		}

		if(!CBaseRegenTick::IsClientValid(pInfo->GetPlayerId())) {
			return true;
		}

		ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(pInfo->GetPlayerId());
		if(pPlayer->GetHealth() < pPlayer->GetMaxHealth() && pPlayer->GetHealth() >= 2)
		{
			pPlayer->TakeHealth(ukr_regen_hp.GetFloat(), DMG_GENERIC);
		}
		return true;
	}
};

TickRegen::TickRegen(void) : CGameEventListeners(), m_infoClient(SourceHook::List<CInfoClient*>())
{
	m_fLastTickedTime	= 0.0f;
	g_fUniversalTime	= 0.0f;

	g_fRegenThink		= 0.0f;
	g_fExtraRegenThink	= 0.0f;
	g_fLuaThink			= 0.0f;

	m_bHasMapTickedYet = false;
	roundStart = true;

	luabridge::getGlobalNamespace(g_Sample.GetLuaState())
		.beginClass<TickRegen>("Frame")
			.addFunction("GetSimulateTime", &TickRegen::GetSimulateTime)
			.addFunction("SetNextThink", std::function<double(TickRegen*, double, float)>([](TickRegen* pThisPtr, double last, float interval)
			{
				double univTime = pThisPtr->GetSimulateTime();
				if(univTime - last - interval <= THINK_INTEVAL)
					return last + interval;
				else
					return univTime + interval;
			}))
			.addFunction("GetEngineTime", std::function<double(TickRegen*)>([](TickRegen *p){ return Plat_FloatTime(); }))
		.endClass();

	luabridge::setGlobal(g_Sample.GetLuaState(), this, "Frame");

	rGameFrame = new luabridge::LuaRef(g_Sample.GetLuaState());
	rClientPutInServer = new luabridge::LuaRef(g_Sample.GetLuaState());

	ListenForGameEvent("round_start");
	ListenForGameEvent("round_end");
	ListenForGameEvent("map_transition");
}

TickRegen::~TickRegen(void)
{
	for(auto iClient : m_infoClient)
	{
		delete iClient;
	}
	m_infoClient.clear();

	delete rGameFrame;
	delete rClientPutInServer;
}

void TickRegen::LevelShutdown()
{
	static luabridge::LuaRef rOnMapEnd = luabridge::getGlobal(g_Sample.GetLuaState(), "OnMapEnd");
	if(rOnMapEnd.isFunction())
	{
		rOnMapEnd();
	}
	g_HL2->ClearCollisionHash();
	m_bHasMapTickedYet = false;
}

void TickRegen::OnClientDisconnected(int client)
{
	auto iter = m_infoClient.begin();
	while (iter != m_infoClient.end())
	{
		if(*(*iter) == client)
		{
			delete (*iter);
			m_infoClient.erase(iter);
			return;
		}
		iter++;
	}
}

/**
 * Main game frame function that updates universal time and triggers regeneration logic.
 * It checks if the map has ticked yet and updates the universal time accordingly.
 * Depending on the current universal time, it calls the appropriate regeneration functions
 * and sets the next think times for regeneration and Lua frame execution.
 */
void TickRegen::GameFrame(bool simul)
{
	if(simul && m_bHasMapTickedYet)
	{
		g_fUniversalTime += g_pGlobals->curtime - m_fLastTickedTime;
	}
	else
	{
		g_fUniversalTime += g_pGlobals->interval_per_tick;
	}

	m_fLastTickedTime = g_pGlobals->curtime;
	m_bHasMapTickedYet = true;
	if(g_fUniversalTime >= g_fRegenThink)
	{
		if(ukr_regen_enable.GetInt() == 1) {
			RegenFrame();
		}

		SetNextThink(g_fRegenThink, THINK_INTEVAL);
		return;
    }

	if(g_fUniversalTime >= g_fExtraRegenThink)
	{
		ExtraRegenFrame();
		SetNextThink(g_fExtraRegenThink, THINK_INTEVAL);
		return;
	}

	if(g_fUniversalTime >= g_fLuaThink)
	{
		if(rGameFrame->isNil())
		{
			(*rGameFrame) = luabridge::getGlobal(g_Sample.GetLuaState(), "OnFrame");
		}

		if(rGameFrame->isFunction())
		{
			auto ret = (*rGameFrame)();

			if(ret.isNumber()) {
				SetNextThink(g_fLuaThink, ret);
				return;
			}
		}

		SetNextThink(g_fLuaThink, THINK_INTEVAL);
		return;
	}
}

/**
 * Executes the extra regeneration logic for each client in the list.
 * If the client list is empty, the function returns immediately.
 * Uses the CExtraTick class to process each client based on whether the round has started.
 */
void TickRegen::ExtraRegenFrame()
{
	if(m_infoClient.empty()) return;

	CExtraTick m_ExtraRegen(IsRoundStart());
	ForEachClientList(m_ExtraRegen);
}

/**
 * Executes the regeneration logic for each client in the list.
 * If the client list is empty, the function returns immediately.
 * Uses the CRegenTick class to process each client based on whether the round has started.
 */
void TickRegen::RegenFrame()
{
	if(m_infoClient.empty()) return;

	CRegenTick m_Regen(IsRoundStart());
	ForEachClientList(m_Regen);
}

/**
 * Handles client connection events by adding them to the regeneration list.
 * If the client is already in the list, it checks if they are still valid.
 * If not valid, it removes them from the list. If valid, it does nothing.
 * If the client is new, it checks their team and adds them to the list if appropriate.
 */
void TickRegen::OnClientConnect(edict_t *pEdict, const char *pName)
{
    int client = g_Sample.IndexOfEdict(pEdict);
	if(rClientPutInServer->isNil())
	{
		(*rClientPutInServer) = luabridge::getGlobal(g_Sample.GetLuaState(), "OnClientPutInServer");
	}

	if(rClientPutInServer->isFunction())
	{
		(*rClientPutInServer)(client);
	}

	CInfoClient pInfo(
		ukr_regen_tick.GetFloat(), 
		0.1f, 
		client, 
		1, 
		false, 
		g_fUniversalTime);

	auto iter = m_infoClient.begin();
	while (iter != m_infoClient.end())
	{
		if(pInfo == (*iter))
		{
			if(playerhelpers->GetGamePlayer(client))
			{
				return;
			}
			else
			{
				delete (*iter);
				m_infoClient.erase(iter);
				break;
			}
		}
		iter++;
	}

    IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
    if(pPlayer)
    {
        if(pPlayer->IsFakeClient())
        {
			if(ukr_regen_infected.GetBool())
			{
				if(pPlayer->GetPlayerInfo()->GetTeamIndex() != 1)
				{
					m_infoClient.push_back(new CInfoClient(pInfo));
				}
			}
			else
			{
				if(pPlayer->GetPlayerInfo()->GetTeamIndex() == 2)
				{
					m_infoClient.push_back(new CInfoClient(pInfo));
				}
			}
        } else {
    		m_infoClient.push_back(new CInfoClient(pInfo));
        }
    }
	return;
}

/**
 * Sets extra execution parameters for a specific client in the regeneration list.
 * If the client is found, it updates their extra execution status, health points, and tick interval.
 * If the client is not found in the list, the function simply returns without making any changes.
 */
void TickRegen::SetExtraExec(int client, bool extra, int hp, float t_tick)
{
	for(auto iClient : m_infoClient)
	{
		if(*iClient == client)
		{
			if(iClient->IsExtra() != extra) {
				iClient->SetExtra(extra);
			}

			if(iClient->GetTExtra() != t_tick) {
				iClient->SetTExtra(t_tick);
			}

			if(iClient->GetHp() != hp) {
				iClient->SetHp(hp);
			}
			return;
		}
	}
	return;
}

/**
 * Retrieves the current simulated time in the game.
 *
 * @return The current simulated time as a double.
 */
double TickRegen::GetSimulateTime() const
{
	return g_fUniversalTime;
}

/**
 * Handles game events related to round start and end.
 * Updates the round start status based on the event name.
 *
 * @param event Pointer to the IGameEvent object containing event details.
 */
void TickRegen::FireGameEvent(IGameEvent *event)
{
	const char* szEventName = event->GetName();
	if(g_Sample.my_bStrcmp(szEventName, "round_start"))
	{
		SetRoundStart(true);
	}

	if(	g_Sample.my_bStrcmp(szEventName, "round_end") ||
		g_Sample.my_bStrcmp(szEventName, "map_transition"))
	{
		SetRoundStart(false);
	}
}

/**
 * Constructor for the CInfoClient class.
 *
 * @param ToExecTick The time until the next execution tick.
 * @param ExtraTick The time interval for extra execution.
 * @param iIndex The index of the client.
 * @param iHp The health points for regeneration.
 * @param bExtra Flag indicating if extra execution is enabled.
 * @param flUniversal Reference to the universal time variable.
 */
CInfoClient::CInfoClient(float ToExecTick, float ExtraTick, int iIndex, int iHp, bool bExtra, double &flUniversal) : 
	flUniversalTime(&flUniversal),
	m_ToExec(flUniversal + ToExecTick), 
	m_ExtraExec(flUniversal + ExtraTick), 
	t_extra(ExtraTick), 
	index(iIndex), 
	p_hp(iHp), 
	b_Extra(bExtra)
{
}

CInfoClient::CInfoClient(const CInfoClient &other)
{
	this->flUniversalTime = other.flUniversalTime;
	this->b_Extra = other.b_Extra;
	this->index = other.index;
	this->m_ExtraExec = other.m_ExtraExec;
	this->m_ToExec = other.m_ToExec;
	this->p_hp = other.p_hp;
	this->t_extra = other.t_extra;
}

CInfoClient::CInfoClient(const CInfoClient *other)
{
	this->flUniversalTime = other->flUniversalTime;
	this->b_Extra = other->b_Extra;
	this->index = other->index;
	this->m_ExtraExec = other->m_ExtraExec;
	this->m_ToExec = other->m_ToExec;
	this->p_hp = other->p_hp;
	this->t_extra = other->t_extra;
}

void CInfoClient::SetNextToExecThink(float interal)
{
	if((*flUniversalTime) - m_ToExec - interal <= THINK_INTEVAL)
	{
		m_ToExec += interal;
	} else {
		m_ToExec = (*flUniversalTime) + interal;
	}
}

void CInfoClient::SetNextExtraExecThink(float interal)
{
	if((*flUniversalTime) - m_ExtraExec - interal <= THINK_INTEVAL)
	{
		m_ExtraExec += interal;
	} else {
		m_ExtraExec = (*flUniversalTime) + interal;
	}
}
