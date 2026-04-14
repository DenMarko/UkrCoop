#ifndef CONVARS_INCLUDE
#define CONVARS_INCLUDE

#include "extension.h"
#include "log_messege.h"

class IConVarChangeListener
{
public:
	virtual void OnConVarChanged(ConVar *pCVar, const char *oldValue, const char* newValue) = 0;
};

class ConVar_l4d
{
public:
	ConVar_l4d();
	~ConVar_l4d();

	static void OnConVarGanger(ConVar* pConVar, const char* oldValue, float flOldValue);
	
	void AddHookConVarChanged(const char* CVarName, IConVarChangeListener *pCallBack);
	void RemoveConVarChanged(const char* CvarName, IConVarChangeListener *pCallBack);

	const char *GetConVarString(const char *name);
	float GetConVarFloat(const char *cvar);
	int GetConVarInt(const char *cvar);

	void SetConVarString(const char *cvar, const char *values);
	void SetConVarFloat(const char *cvar, float values);
	void SetConVarInt(const char *cvar, int value);

	void SetBounds(const char *cvar, bool haxMin, float valMin, bool haxMax, float valMax);
};

extern ConVar_l4d *g_pConVar;

#endif	// CONVARS_INCLUDE