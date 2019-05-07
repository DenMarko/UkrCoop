#ifndef _TICKREGEN_HEDER_FILE_
#define _TICKREGEN_HEDER_FILE_

#include "extension.h"
#include "HL2.h"

struct infoClient
{
	int index;
	double m_ToExec;
};

class TickRegen
{
public:
	TickRegen(void);
	~TickRegen(void);
public:
	void GameFrame(bool simulating);
	void LevelShutdown();
public:
	void RegenFrame();
	void OnClientConnect(edict_t *pEntity, char const *playername);
	void OnClientDisconnect(int index);
	void SetRoundStart(bool values)
	{
		if(values != roundStart)
			roundStart = values;
	}
private:
	inline double CalcNextThink(double last, float interal)
	{
		if(g_fUniversalTime - last - interal <= 0.1)
			return last + interal;
		else
			return g_fUniversalTime + interal;
	}
	inline int GetPlayerHealth(int index)
	{
		return g_HL2.GetEntProp(index, Prop_Sends, "m_iHealth");
	}
	inline int GetPlayerMaxHealth(int client)
	{
		return g_HL2.GetEntProp(client, Prop_Sends, "m_iMaxHealth");
	}
	inline void SetPlayerHealth(int index, int HP)
	{
		g_HL2.SetEntProp(index, Prop_Sends, "m_iHealth", HP);
	}
	inline bool IsPlayerAlive(int client)
	{
		if(!g_HL2.GetEntProp(client, Prop_Sends, "m_lifeState"))
			return true;
		return false;
	}
	inline double GetSimulatedTime()
	{
		return g_fUniversalTime;
	}

	SourceHook::List<infoClient> m_infoClient;
	double g_fUniversalTime;
	double g_fTimerThink;
	float m_fLastTickedTime;
	bool m_bHasMapTickedYet;
	bool roundStart;
};

#endif