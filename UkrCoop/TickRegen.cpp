#include "TickRegen.h"

ConVar ukr_regen_tick("ukr_regen_tick", "1.0", 0, "Time regen tick", true, 0.1f, true, 25.0f);
ConVar ukr_regen_hp("ukr_regen_hp", "1", 0, "Regen of time hp", true, 1.0f, true, 100.0f);
ConVar ukr_regen_enable("ukr_regen_enable", "1", 0, "Enable regen", true, 0.f, true, 1.f);

TickRegen::TickRegen(void) : m_fLastTickedTime(0.f), m_bHasMapTickedYet(false), g_fUniversalTime(0.f), g_fTimerThink(0.f), roundStart(true)
{}

TickRegen::~TickRegen(void)
{
	m_infoClient.clear();
}

void TickRegen::LevelShutdown()
{
	m_bHasMapTickedYet = false;
}

void TickRegen::GameFrame(bool simul)
{
	if (ukr_regen_enable.GetInt() != 1)
	{
		return;
	}

	if(simul && m_bHasMapTickedYet) {
		g_fUniversalTime += g_pGlobals->curtime - m_fLastTickedTime;
	} else {
		g_fUniversalTime += g_pGlobals->interval_per_tick;
	}

	m_fLastTickedTime = g_pGlobals->curtime;
	m_bHasMapTickedYet = true;
	if(g_fUniversalTime >= g_fTimerThink)
	{
		this->RegenFrame();
		g_fTimerThink = this->CalcNextThink(g_fTimerThink, 0.1f);
	}
}

void TickRegen::RegenFrame()
{
	double curtime	= this->GetSimulatedTime();
	auto iter		= m_infoClient.begin();

	infoClient *iClient;
	while(iter != m_infoClient.end())
	{
		iClient = &(*iter);
		if(curtime >= iClient->m_ToExec)
		{
			if(roundStart)
			{
				IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(iClient->index);
				if(!pPlayer){
					goto end;
				}
				else if(!pPlayer->IsConnected()){
					goto end;
				}
				else if(!pPlayer->IsInGame()){
					goto end;
				}
				else if(!IsPlayerAlive(iClient->index)){
					goto end;
				}
			
				int Healt		= this->GetPlayerHealth(iClient->index);
				int MaxHealt	= this->GetPlayerMaxHealth(iClient->index);

				if(Healt <= MaxHealt && Healt >= 2)
				{
					if(Healt + ukr_regen_hp.GetInt() >= MaxHealt)
						this->SetPlayerHealth(iClient->index, MaxHealt);
					else
						this->SetPlayerHealth(iClient->index, Healt + ukr_regen_hp.GetInt());
				}
			}
end:;
			iClient->m_ToExec = this->CalcNextThink(iClient->m_ToExec, ukr_regen_tick.GetFloat());
		}
		iter++;
	}
}

void TickRegen::OnClientConnect(edict_t *pEntity, char const *playername)
{
	int client = g_pUkrCoop.IndexOfEdict(pEntity);
	auto iter = m_infoClient.begin();
	while (iter != m_infoClient.end())
	{
		if((*iter).index == client)
		{
			return;
		}
		iter++;
	}
	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
	if(pPlayer)
	{
		if(pPlayer->IsFakeClient())
		{
			if(pPlayer->GetPlayerInfo()->GetTeamIndex() == 2)
			{
				infoClient info;
				info.index = client;
				info.m_ToExec = (g_fUniversalTime + ukr_regen_tick.GetFloat());
				m_infoClient.push_back(info);
			}
		}
		else
		{
			infoClient info;
			info.index = client;
			info.m_ToExec = (g_fUniversalTime + ukr_regen_tick.GetFloat());
			m_infoClient.push_back(info);
		}
	}
}

void TickRegen::OnClientDisconnect(int index)
{
	auto iter = m_infoClient.begin();
	infoClient *iClient;
	while(iter != m_infoClient.end())
	{
		iClient = &(*iter);
		if(iClient->index == index)
		{
			m_infoClient.erase(iter);
			return;
		}
		iter++;
	}
}
