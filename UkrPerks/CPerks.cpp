#include "CPerks.h"
#include "game/server/iplayerinfo.h"
#include "UkrPerks/Utils.h"

size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...);

enum Teams
{
	Team_None = 0,
	Team_Spectator, // 1
	Team_Survivor,	// 2
	Team_Infected	// 3
};

constexpr auto MAX_UPGRADES = 19;

unsigned long iUpgrades[SM_MAXPLAYERS + 1][MAX_UPGRADES + 1] = { NULL };
int iUpgradeDisabled[SM_MAXPLAYERS + 1][MAX_UPGRADES + 1] = { NULL };
unsigned long iBitsUpgrades[SM_MAXPLAYERS + 1] = { NULL };
UPGRADES UpgradeIndex[MAX_UPGRADES + 1];

ConVar *pUpgradeEnable[MAX_UPGRADES] = { nullptr };
ConVar *pEnable = new ConVar("ukr_enable_perks", "1", FCVAR_NONE, "1 enable perks, 0 disable perks", true, 0, true, 1);


CPerks::CPerks() : msg_Id(usermsgs->GetMessageIndex("SayText"))
{
	usermsgs->HookUserMessage2(msg_Id, this, false);

	g_pHookEvent.HookEvent("survivor_rescued", &CPerks::event_Rescued);
	g_pHookEvent.HookEvent("tank_killed", &CPerks::event_TankKilled);
	g_pHookEvent.HookEvent("player_jump", &CPerks::event_PlayerJump);
	g_pHookEvent.HookEvent("round_start", &CPerks::round_start);
	g_pHookEvent.HookEvent("award_earned", &CPerks::event_AwardEarned);
	g_pHookEvent.HookEvent("player_death", &CPerks::event_PlayerDeath);
	g_pHookEvent.HookEvent("infected_hurt", &CPerks::Event_InfectedHurt);
	g_pHookEvent.HookEvent("player_hurt", &CPerks::Event_PlayerHurt);
	g_pHookEvent.HookEvent("witch_killed", &CPerks::Event_WitchKilled);
	g_pHookEvent.HookEvent("player_incapacitated", &CPerks::Event_PlayerIncaped);
	g_pHookEvent.HookEvent("heal_success", &CPerks::event_HealSuccess);
	g_pHookEvent.HookEvent("round_end", &CPerks::round_end);
	g_pHookEvent.HookEvent("map_transition", &CPerks::round_end);
	g_pHookEvent.HookEvent("player_use", &CPerks::event_PlayerUse);
	g_pHookEvent.HookEvent("ammo_pickup", &CPerks::event_AmmoPickup);
	g_pHookEvent.HookEvent("item_pickup", &CPerks::event_ItemPickup);
	g_pHookEvent.HookEvent("player_spawn", &CPerks::event_PlayerSpawn);
	g_pHookEvent.HookEvent("weapon_fire", &CPerks::Event_WeaponFire);
	g_pHookEvent.HookEvent("bullet_impact", &CPerks::Event_BulletImpact);

	DatabaseInfo info;
	info.database = "Perks";
	info.driver = "sqlite";
	info.host = "";
	info.maxTimeout = 0;
	info.pass = "";
	info.port = 0;
	info.user = "";

	driver = dbi->FindOrLoadDriver(info.driver);

	char error[128];
	db = driver->Connect(&info, false, error, 128);
	if (db == nullptr)
		g_IUkrCoop->UkrCoop_LogMessage("[UKR PERKS] %s", error);
	else
		db->DoSimpleQuery("CREATE TABLE IF NOT EXISTS accounts (steamid TEXT PRIMARY KEY, laser_load SMALLINT, silencer_load SMALLINT, hollow_point SMALLINT, fiery_ammo SMALLINT, menu_ammo_fiery SMALLINT, menu_ammo_hollow SMALLINT, upgrades_binary VARCHAR(42), disabled_binary VARCHAR(42));");
}

CPerks::~CPerks()
{
	usermsgs->UnhookUserMessage2(msg_Id, this, false);
}

void CPerks::SaveData(IGamePlayer * player)
{
	if (player->GetPlayerInfo()->GetTeamIndex() != Team_Infected)
	{
		char TQuery[3000];
		char UpgradeBinary[64];
		char DisabledBinary[64];

		for (int i(0); i < MAX_UPGRADES; i++)
		{
			if (i == 0)
			{
				if (iUpgrades[player->GetIndex()][i] > 0)
				{
					UTIL_Format(UpgradeBinary, sizeof(UpgradeBinary), "1");
				}
				else
				{
					UTIL_Format(UpgradeBinary, sizeof(UpgradeBinary), "0");
				}
				if (iUpgradeDisabled[player->GetIndex()][i] > 0)
				{
					UTIL_Format(DisabledBinary, sizeof(DisabledBinary), "1");
				}
				else
				{
					UTIL_Format(DisabledBinary, sizeof(DisabledBinary), "0");
				}
			}
			else
			{
				if (iUpgrades[player->GetIndex()][i] > 0)
				{
					UTIL_Format(UpgradeBinary, sizeof(UpgradeBinary), "%s1", UpgradeBinary);
				}
				else
				{
					UTIL_Format(UpgradeBinary, sizeof(UpgradeBinary), "%s0", UpgradeBinary);
				}
				if (iUpgradeDisabled[player->GetIndex()][i] > 0)
				{
					UTIL_Format(DisabledBinary, sizeof(DisabledBinary), "%s1", UpgradeBinary);
				}
				else
				{
					UTIL_Format(DisabledBinary, sizeof(DisabledBinary), "%s0", UpgradeBinary);
				}
			}
		}

		UTIL_Format(TQuery, sizeof(TQuery), "INSERT OR REPLACE INTO accounts VALUES ('%s', %d, %d, %d, %d, %d, %d, '%s', '%s');", player->GetSteam2Id(), LaserLoad[client], SilencerLoad[client], g_SpecialAmmoHollowPoint[client], g_SpecialAmmoFiery[client], MenuAmmoFieryCurect[client], MenuAmmoHollowPointCurect[client], UpgradeBinary, DisabledBinary);
		db->DoSimpleQuery(TQuery);
	}
}

void CPerks::LoadData(IGamePlayer * player)
{
	if (player->IsInGame())
	{
		char TQuery[192];
		IQuery *m_pQuery = nullptr;
		UTIL_Format(TQuery, sizeof(TQuery), "SELECT * FROM accounts WHERE steamId = '%s';", player->GetSteam2Id());

		db->LockForFullAtomicOperation();
		m_pQuery = db->DoQuery(TQuery);
		if (!m_pQuery)
		{
			g_IUkrCoop->UkrCoop_LogMessage("[UKR PERKS] load data for connect client error: %s", db->GetError());
			db->UnlockFromFullAtomicOperation();

			if (!player->IsFakeClient())
			{
				for (int i = 0; i < MAX_UPGRADES; i++)
				{
					iUpgrades[player->GetIndex()][i] = 0;
					iUpgradeDisabled[player->GetIndex()][i] = 0;
				}
				iBitsUpgrades[player->GetIndex()] = 0;
			}
			return;
		}
		db->UnlockFromFullAtomicOperation();

		for (int i = 0; i < MAX_UPGRADES; i++)
		{
			iUpgrades[player->GetIndex()][i] = 0;
			iUpgradeDisabled[player->GetIndex()][i] = 0;
		}
		iBitsUpgrades[player->GetIndex()] = 0;


		IResultSet *rs = m_pQuery->GetResultSet();
		if (!rs)
		{
			g_IUkrCoop->UkrCoop_LogMessage("[UKR PERKS] No current result set");
			return;
		}

		IResultRow *row = rs->CurrentRow();
		if (!row)
		{
			g_IUkrCoop->UkrCoop_LogMessage("[UKR PERKS] Current result set has no fetched rows");
			return;
		}

		const char *UpgradesBinary;
		const char *DisabledBinary;
		size_t length;


		row->GetString(7, &UpgradesBinary, &length);
		row->GetString(8, &DisabledBinary, &length);

		int len = strlen(UpgradesBinary);
		for (int i(0); i <= len; i)
		{
			if (UpgradesBinary[i] == '1')
			{
				iUpgrades[player->GetIndex()][i] = UpgradeIndex[i];
			}
		}

		len = strlen(DisabledBinary);
		for (int i(0); i <= len; i++)
		{
			if (DisabledBinary[i] == '1')
			{
				iUpgradeDisabled[player->GetIndex()][i] = 1;
			}
		}
	}
}

void CPerks::OnUserMessage(int msg_id, bf_write *bf, IRecipientFilter *pFilter)
{
	bf_read bf_r;
	bf_r.StartReading(bf->GetBasePointer(), bf->GetNumBytesWritten());

	bf_r.ReadShort();
	bf_r.ReadShort();

	char msgType[256];

	bf_r.ReadString(msgType, sizeof(msgType));

	const auto StrContains = [](const char *str1, const char *str2) {
		const char *pos = Q_strstr(str1, str2);
		if (pos)
		{
			return (pos - str1);
		}
		return -1;
	};

	DelayPrintExpire  p_DelayPrint;
	int pData;

	if (StrContains(msgType, "prevent_it_expire") != -1)
	{
		pData = 8;
		timersys->CreateTimer(&p_DelayPrint, 0.25f, (LPVOID)pData, TIMER_FLAG_NO_MAPCHANGE);
		return;
	}
	if (StrContains(msgType, "Smoker's Tongue attack") != -1)
	{
		pData = 7;
		timersys->CreateTimer(&p_DelayPrint, 0.25f, (LPVOID)pData, TIMER_FLAG_NO_MAPCHANGE);
		return;
	}
	if (StrContains(msgType, "ledge_save_expire") != -1)
	{
		pData = 9;
		timersys->CreateTimer(&p_DelayPrint, 0.25f, (LPVOID)pData, TIMER_FLAG_NO_MAPCHANGE);
		return;
	}
	if (StrContains(msgType, "revive_self_expire") != -1)
	{
		pData = 10;
		timersys->CreateTimer(&p_DelayPrint, 0.25f, (LPVOID)pData, TIMER_FLAG_NO_MAPCHANGE);
		return;
	}
	if (StrContains(msgType, "knife_expire") != -1)
	{
		pData = 6;
		timersys->CreateTimer(&p_DelayPrint, 0.25f, (LPVOID)pData, TIMER_FLAG_NO_MAPCHANGE);
		return;
	}
	if (StrContains(msgType, "_expire") != -1)
	{
		return;
	}
	if ((StrContains(msgType, "#L4D_Upgrade") != -1) && (StrContains(msgType, "description") != -1))
	{
		return;
	}
	if (StrContains(msgType, "NOTIFY_VOMIT_ON") != -1)
	{
		return;
	}
	return;
}

void CPerks::OnUserMessageSent(int msg_id)
{
}

class DelayPrintExpire : public ITimedEvent
{
public:
	virtual ResultType OnTimer(ITimer *pTimer, void *pData) 
	{
		int type = (int)pData;
		switch (type)
		{
			case 6:
			{
				break;
			}
			case 7:
			{
				break;
			}
			case 8:
			{
				break;
			}
			case 9:
			{
				break;
			}
			case 10:
			{
				break;
			}
		}

		return ResultType::Pl_Stop;
	}
	virtual void OnTimerEnd(ITimer *pTimer, void *pData)
	{}
};

bool CPerks::event_Rescued(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_TankKilled(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_PlayerJump(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::round_start(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_AwardEarned(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_PlayerDeath(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::Event_InfectedHurt(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::Event_PlayerHurt(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::Event_WitchKilled(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::Event_PlayerIncaped(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_HealSuccess(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::round_end(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_PlayerUse(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_AmmoPickup(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_ItemPickup(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::event_PlayerSpawn(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::Event_WeaponFire(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

bool CPerks::Event_BulletImpact(IGameEvent *pEvent, bool bDontBroatcast)
{
	return true;
}

void CPerks::OnClientPutInServer(edict_t * pEntity, const char * playername)
{
	IGamePlayer *GPlayer = playerhelpers->GetGamePlayer(pEntity);
	if (GPlayer->IsConnected() && GPlayer->IsInGame())
	{
		if (!GPlayer->IsFakeClient() && GPlayer->GetPlayerInfo()->GetTeamIndex() != Teams::Team_Infected)
		{
			timersys->CreateTimer(&hLoadData, 0.25f, (LPVOID)GPlayer->GetIndex(), TIMER_FLAG_NO_MAPCHANGE);
		}
	}
}

class DelayLoadData : public ITimedEvent
{
public:
	virtual ResultType OnTimer(ITimer *pTimer, void *pData)
	{
		pPerks->LoadData(playerhelpers->GetGamePlayer((int)pData));
		return ResultType::Pl_Stop;
	}
	virtual void OnTimerEnd(ITimer *pTimer, void *pData)
	{}
} hLoadData;

unsigned long CPerks::SetUpgradeBitVec(int client)
{
	int upgradeBitVec = 0;
	for (int i = 0; i < MAX_UPGRADES; i++)
	{
		if (iUpgrades[client][i] > 0 && iUpgradeDisabled[client][i] != 1)
		{
			if (iUpgrades[client][i] == UPGRADES_LASER_SIGHT/* && LaserLoad[client] == 1*/)
			{
				upgradeBitVec += UPGRADES_LASER_SIGHT;
			}
			else if (iUpgrades[client][i] == UPGRADES_SILENCER /*&& SilencerLoad[client] == 1*/)
			{
				upgradeBitVec += UPGRADES_SILENCER;
			}
			else if (iUpgrades[client][i] == UPGRADES_EXTENDED_MAGAZINE)
			{
				upgradeBitVec += UPGRADES_EXTENDED_MAGAZINE;
			}
			else if (iUpgrades[client][i] == UPGRADES_SCOPE)
			{
				upgradeBitVec += UPGRADES_SCOPE;
			}
			else if (iUpgrades[client][i] == UPGRADES_RELOADER)
			{
				upgradeBitVec += UPGRADES_RELOADER;
			}
			else if (iUpgrades[client][i] == UPGRADES_ADRENALIN_IMPLANT)
			{
				upgradeBitVec += UPGRADES_ADRENALIN_IMPLANT;
			}
			else if (iUpgrades[client][i] == UPGRADES_KNIFE)
			{
				upgradeBitVec += UPGRADES_KNIFE;
			}
			else if (iUpgrades[client][i] == UPGRADES_SMOKER_NEUTRALIZE)
			{
				upgradeBitVec += UPGRADES_SMOKER_NEUTRALIZE;
			}
			else if (iUpgrades[client][i] == UPGRADES_BOOMER_NEUTRALIZE)
			{
				upgradeBitVec += UPGRADES_BOOMER_NEUTRALIZE;
			}
			else if (iUpgrades[client][i] == UPGRADES_CLIMBING_CHALK)
			{
				upgradeBitVec += UPGRADES_CLIMBING_CHALK;
			}
			else if (iUpgrades[client][i] == UPGRADES_SECOND_WIND)
			{
				upgradeBitVec += UPGRADES_SECOND_WIND;
			}
			else if (iUpgrades[client][i] == UPGRADES_KEVLAR_ARMOR)
			{
				upgradeBitVec += UPGRADES_KEVLAR_ARMOR;
			}
			else if (iUpgrades[client][i] == UPGRADES_HOT_MEAL)
			{
				upgradeBitVec += UPGRADES_HOT_MEAL;
			}
			else if (iUpgrades[client][i] == UPGRADES_OINTMENT)
			{
				upgradeBitVec += UPGRADES_OINTMENT;
			}
			else if (iUpgrades[client][i] == UPGRADES_AIR_BOOTS)
			{
				upgradeBitVec += UPGRADES_AIR_BOOTS;
			}
			else if (iUpgrades[client][i] == UPGRADES_BACKPACK)
			{
				upgradeBitVec += UPGRADES_BACKPACK;
			}
			else if (iUpgrades[client][i] == UPGRADES_SMELLING_SALTS)
			{
				upgradeBitVec += UPGRADES_SMELLING_SALTS;
			}
			else if (iUpgrades[client][i] == UPGRADES_HOLLOW_POINT /*&& MenuAmmoHollowPointCurect[client] == 1*/)
			{
				upgradeBitVec += UPGRADES_HOLLOW_POINT;
			}
		}
	}
	return upgradeBitVec;
}

int CPerks::GetSurvivorUpgrades(int client)
{
	int upgrades = 0;
	for (int i = 0; i < MAX_UPGRADES; i++)
	{
		if (iUpgrades[client][i] > 0)
		{
			upgrades++;
		}
	}
	return upgrades;
}

int CPerks::GetAbSurvivorUpgrades(int client)
{
	int upgrades = 0;
	for (int i = 0; i < MAX_UPGRADES; i++)
	{
		if (iUpgrades[client][i] > 0 || iUpgradeDisabled[client][i] == 1)
		{
			upgrades++;
		}
	}
	return upgrades;
}

int CPerks::MissingSurvivorUpgrades(int client)
{
	int upgrades = 0;
	for (int i = 0; i < MAX_UPGRADES; i++)
	{
		if (iUpgrades[client][i] <= 0)
		{
			upgrades++;
		}
	}
	return upgrades;
}
