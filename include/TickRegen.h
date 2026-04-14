#ifndef _TICKREGEN_HEDER_FILE_
#define _TICKREGEN_HEDER_FILE_

#include "extension.h"
#include "sh_list.h"
#include "HL2.h"
#include "log_messege.h"

#define THINK_INTEVAL 0.1f

class CInfoClient
{
public:
	CInfoClient(float ExtraExec, float ExtraTick, int iIndex, int iHp, bool bExtra, double &flUniversal);
	CInfoClient(const CInfoClient &other);
	CInfoClient(const CInfoClient *other);

	~CInfoClient() = default;

	bool IsExtra() const
	{
		return b_Extra;
	}
	void SetExtra(bool val)
	{
		b_Extra = val;
	}

	int GetHp() const
	{
		return p_hp;
	}

	void SetHp(int hp)
	{
		this->p_hp = hp;
	}

	int GetPlayerId() const
	{
		return index;
	}

	float GetTExtra() const
	{
		return t_extra;
	}

	void SetTExtra(float val)
	{
		t_extra = val;
	}

	bool IsExec() const
	{
		return ((*flUniversalTime) >= m_ToExec);
	}
	bool IsExtraExec() const
	{
		return ((*flUniversalTime) >= m_ExtraExec);
	}

	void SetNextToExecThink(float interal);
	void SetNextExtraExecThink(float interal);

public:
	bool operator == (const CInfoClient &other) const
	{
		return (this->GetPlayerId() == other.GetPlayerId());
	}

	bool operator == (const CInfoClient *other) const
	{
		return (this->GetPlayerId() == other->GetPlayerId());
	}

	bool operator == (int client) const
	{
		return (this->GetPlayerId() == client);
	}

	bool operator != (const CInfoClient &other) const
	{
		return (this->GetPlayerId() != other.GetPlayerId());
	}

	bool operator != (const CInfoClient *other) const
	{
		return (this->GetPlayerId() != other->GetPlayerId());
	}

	bool operator != (int client) const
	{
		return (this->GetPlayerId() != client);
	}

private:
	const double *flUniversalTime;

	double m_ToExec;
	double m_ExtraExec;
	float t_extra;
	int index;
	int p_hp;
	bool b_Extra;
};

class TickRegen : public CGameEventListeners, public IClientListener
{
public:
	TickRegen(void);
	~TickRegen(void);
public:
	void GameFrame(bool simulating);
	void LevelShutdown();
public:
	virtual void OnClientDisconnected(int client) override;
public:
	void RegenFrame();
	void ExtraRegenFrame();
	void OnClientConnect(edict_t *pEdict, const char *pName);
	inline void SetRoundStart(bool value)
	{
	    if(value != roundStart)
		{
            roundStart = value;
		}
	}
	inline bool IsRoundStart()
	{
	    return roundStart;
	}
	void SetExtraExec(int i_client, bool extra, int hp, float t_tick);
    double GetSimulateTime() const;

	virtual void FireGameEvent( IGameEvent *event ) override;
private:
	template<typename FUNC>
	bool ForEachClientList(FUNC &func)
	{
		auto iter = m_infoClient.begin();
		while (iter != m_infoClient.end())
		{
			if(!func(*iter))
			{
				return false;
			}
			iter++;
		}
		return true;
	}

	inline void SetNextThink(double &last, float interal)
	{
		if(g_fUniversalTime - last - interal <= THINK_INTEVAL)
		{
			last += interal;
		} else {
			last = g_fUniversalTime + interal;
		}
	}

	SourceHook::List<CInfoClient *> m_infoClient;
	double g_fUniversalTime;
	
	double g_fRegenThink;
	double g_fExtraRegenThink;
	double g_fLuaThink;

	float m_fLastTickedTime;
	bool m_bHasMapTickedYet;
	bool roundStart;
};

#endif
