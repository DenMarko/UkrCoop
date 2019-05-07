#ifndef HEDER_FILE_PERKS
#define HEDER_FILE_PERKS

#pragma once

#include "extension.h"

enum UPGRADES
{
	UPGRADES_KEVLAR_ARMOR				= (1 << 1),
	UPGRADES_AIR_BOOTS					= (1 << 6),
	UPGRADES_BACKPACK					= (1 << 7),
	UPGRADES_BOOMER_NEUTRALIZE			= (1 << 8),
	UPGRADES_SMOKER_NEUTRALIZE			= (1 << 9),
	UPGRADES_CLIMBING_CHALK				= (1 << 11),
	UPGRADES_SECOND_WIND				= (1 << 12),
	UPGRADES_ADRENALIN_IMPLANT			= (1 << 15),
	UPGRADES_HOT_MEAL					= (1 << 16),
	UPGRADES_LASER_SIGHT				= (1 << 17),
	UPGRADES_SILENCER					= (1 << 18),
	UPGRADES_EXTENDED_MAGAZINE			= (1 << 20),
	UPGRADES_HOLLOW_POINT				= (1 << 21),
	UPGRADES_SCOPE						= (1 << 24),
	UPGRADES_KNIFE						= (1 << 26),
	UPGRADES_SMELLING_SALTS				= (1 << 27),
	UPGRADES_OINTMENT					= (1 << 28),
	UPGRADES_RELOADER					= (1 << 29),
};

class CPerks : public Sample,
	public IBitBufUserMessageListener
{
	int msg_Id;
public:
	CPerks();
	~CPerks();
public:
	void OnUserMessage(int msg_id, bf_write *bf, IRecipientFilter *pFilter);
	void OnUserMessageSent(int msg_id);
public:
	bool event_Rescued(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_TankKilled(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_PlayerJump(IGameEvent *pEvent, bool bDontBroatcast);
	bool round_start(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_AwardEarned(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_PlayerDeath(IGameEvent *pEvent, bool bDontBroatcast);
	bool Event_InfectedHurt(IGameEvent *pEvent, bool bDontBroatcast);
	bool Event_PlayerHurt(IGameEvent *pEvent, bool bDontBroatcast);
	bool Event_WitchKilled(IGameEvent *pEvent, bool bDontBroatcast);
	bool Event_PlayerIncaped(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_HealSuccess(IGameEvent *pEvent, bool bDontBroatcast);
	bool round_end(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_PlayerUse(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_AmmoPickup(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_ItemPickup(IGameEvent *pEvent, bool bDontBroatcast);
	bool event_PlayerSpawn(IGameEvent *pEvent, bool bDontBroatcast);
	bool Event_WeaponFire(IGameEvent *pEvent, bool bDontBroatcast);
	bool Event_BulletImpact(IGameEvent *pEvent, bool bDontBroatcast);
};

#endif //HEDER_FILE_PERKS