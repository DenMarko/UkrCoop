#include "CPerks.h"

CPerks::CPerks() : msg_Id(usermsgs->GetMessageIndex("SayText"))
{
	usermsgs->HookUserMessage2(msg_Id, this, false);

	g_pHookEvent.HookEvent("survivor_rescued",		this->event_Rescued);
	g_pHookEvent.HookEvent("tank_killed",			this->event_TankKilled);
	g_pHookEvent.HookEvent("player_jump",			this->event_PlayerJump);
	g_pHookEvent.HookEvent("round_start",			this->round_start);
	g_pHookEvent.HookEvent("award_earned",			this->event_AwardEarned);
	g_pHookEvent.HookEvent("player_death",			this->event_PlayerDeath);
	g_pHookEvent.HookEvent("infected_hurt",			this->Event_InfectedHurt);
	g_pHookEvent.HookEvent("player_hurt",			this->Event_PlayerHurt);
	g_pHookEvent.HookEvent("witch_killed",			this->Event_WitchKilled);
	g_pHookEvent.HookEvent("player_incapacitated",	this->Event_PlayerIncaped);
	g_pHookEvent.HookEvent("heal_success",			this->event_HealSuccess);
	g_pHookEvent.HookEvent("round_end",				this->round_end);
	g_pHookEvent.HookEvent("map_transition",		this->round_end);
	g_pHookEvent.HookEvent("player_use",			this->event_PlayerUse);
	g_pHookEvent.HookEvent("ammo_pickup",			this->event_AmmoPickup);
	g_pHookEvent.HookEvent("item_pickup",			this->event_ItemPickup);
	g_pHookEvent.HookEvent("player_spawn",			this->event_PlayerSpawn);
	g_pHookEvent.HookEvent("weapon_fire",			this->Event_WeaponFire);
	g_pHookEvent.HookEvent("bullet_impact",			this->Event_BulletImpact);

}

CPerks::~CPerks()
{
	usermsgs->UnhookUserMessage2(msg_Id, this, false);
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
