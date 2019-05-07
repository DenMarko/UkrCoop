#ifndef CONVARS_INCLUDE
#define CONVARS_INCLUDE

#include "extension.h"
#include "sourcehook\sh_list.h"

struct ConVarInfo
{
	ke::String name;
	ConVar *pVar;
};

class ConVar_l4d
{
private:
	SourceHook::List<ConVarInfo> m_ComVarInfo;
public:
	ConVar *FindConVar(const char *cvar)
	{
		for (auto iter = m_ComVarInfo.begin(); iter != m_ComVarInfo.end(); iter++)
		{
			if((*iter).name.compare(cvar) == 0)
				return (*iter).pVar;
		}

		ConVar *pVar = icvar->FindVar(cvar);
		ConVarInfo pInfo;

		pInfo.name = cvar;
		pInfo.pVar = pVar;
		m_ComVarInfo.push_back(pInfo);

		return pVar;
	}

	const char *GetConVarString(const char *name)
	{
		ConVar *Cvar = this->FindConVar(name);
		Assert(Cvar != NULL);
		if(Cvar == NULL)
			return NULL;
		return Cvar->GetString();
	}

	void SetConVarString(const char *cvar, const char *values)
	{
		ConVar *Cvar = this->FindConVar(cvar);
		Assert(Cvar != NULL);
		if(Cvar == NULL)
			return;
		Cvar->SetValue(values);
		return;
	}

	const float GetConVarFloat(const char *cvar){
		ConVar *Cvar = this->FindConVar(cvar);
		Assert(Cvar != NULL);
		if(Cvar == NULL)
			return NULL;
		return Cvar->GetFloat();
	}

	void SetConVarFloat(const char *cvar, float values){
		ConVar *Cvar = this->FindConVar(cvar);
		Assert(Cvar != NULL);
		if(Cvar == NULL)
			return;
		Cvar->SetValue(values);
		return;
	}

	const int GetConVarInt(const char *cvar){
		ConVar *Cvar = this->FindConVar(cvar);
		Assert(Cvar != NULL);
		if(Cvar == NULL)
			return NULL;
		return Cvar->GetInt();
	}

	void SetConVarInt(const char *cvar, int value){
		ConVar *Cvar = this->FindConVar(cvar);
		Assert(Cvar != NULL);
		if(Cvar == NULL)
			return;
		Cvar->SetValue(value);
		return;
	}

	void SetBounds(const char *cvar, bool haxMin, float valMin, bool haxMax, float valMax)
	{
		ConVar *Cvar = this->FindConVar(cvar);
		if (Cvar == NULL)
			return;

		float MinVal = 0.f;
		float MaxVal = 0.f;

		Cvar->GetMin(MinVal);
		if(valMin != MinVal)
			Cvar->SetMin(haxMin, valMin);

		Cvar->GetMax(MaxVal);
		if (valMax != MaxVal)
			Cvar->SetMax(haxMax, valMax);
	}
};

extern ConVar_l4d g_pConVar;

#endif	// CONVARS_INCLUDE