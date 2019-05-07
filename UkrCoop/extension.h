/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "sdk\smsdk_ext.h"
#include "iplayerinfo.h"
#include "toolframework\itoolentity.h"
#include "sdk\Strings.h"
#include "interface\IUkrCoop.h"
#include "extensions\IBinTools.h"
#include "extensions\IGeoIP.h"
#include "iserverplugin.h"
#include "sourcehook\sh_list.h"

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */

#define IS_VALID_CLIENT(client) \
	if((client >= 1) && (client <= playerhelpers->GetMaxClients())) \
	{ \
		IGamePlayer *I_Client = playerhelpers->GetGamePlayer(client); \
		if(!I_Client) \
			return pContext->ThrowNativeError("Client index %d is not valid", client); \
			\
		if(!I_Client->IsConnected() && !I_Client->IsInGame()) \
			return pContext->ThrowNativeError("Client is not connect or is not in game"); \
	} \
	else if(client < 0 || client > g_pGlobals->maxEntities) \
	{ \
		return pContext->ThrowNativeError("Entity index %d is not valid", client); \
	} \
	else \
	{ \
		edict_t *pEdict = g_pUkrCoop.PEntityOfEntIndex(client); \
		if(!pEdict || pEdict->IsFree()) \
			return pContext->ThrowNativeError("Entity %d is not valid or is freed", client); \
	}

struct logvalues {
	ke::String	m_NrmFileName;
	int			m_NrmCurDay;
	bool		m_DailPrinted;
};

enum Resultat{
	PL_Continue	= 0,
	PL_Changed	= 1,
	PL_Handled	= 2,
	PL_Stop		= 3,
};

extern ICvar		*icvar;
extern CGlobalVars	*g_pGlobals;

class Ukr_coop : public SDKExtension,
					public IConCommandBaseAccessor
{
public:
	Ukr_coop():	m_pSayCmd(NULL),
				m_pSayTeamCmd(NULL)
	{}
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
	virtual void SDK_OnAllLoaded();
	virtual void SDK_OnUnload(void);
	virtual bool QueryInterfaceDrop(SMInterface *pInterface);
	virtual bool QueryRunning(char *error, size_t maxlen);
	const char *GetExtensionVerString();
	const char *GetExtensionDateString();
#if defined SMEXT_CONF_METAMOD
public:
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	virtual bool SDK_OnMetamodUnload(char *error, size_t maxlen);
#endif
public:
	virtual bool RegisterConCommandBase(ConCommandBase *pVar);
private:
	ConCommand*	m_pSayCmd;
	ConCommand*	m_pSayTeamCmd;

	int			m_CmdClient;
public:
	const int GetComandsClient(void) const
	{
		return m_CmdClient;
	}

	ConCommand* FindComands(const char* name)
	{
		return icvar->FindCommand(name);
	}

	const char* GetTeamName(const int team) const
	{
		switch(team)
		{
		case 1:
			return "Spectate";
		case 2:
			return "Survivor";
		case 3:
			return "Infected";
		case 0:
			return "Console";
		default:
			return "Error";
		}
	}

	const int IndexOfEdict(const edict_t *pEdict) const
	{
		return (int)(pEdict - g_pGlobals->pEdicts);
	}

	edict_t *PEntityOfEntIndex(int iEntIndex)
	{
		if (iEntIndex >= 0 && iEntIndex < g_pGlobals->maxEntities)
		{
			return (edict_t *)(g_pGlobals->pEdicts + iEntIndex);
		}
		return NULL;
	}

	size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		size_t len = vsnprintf(buffer, maxlength, fmt, ap);
		va_end(ap);

		if (len >= maxlength)
		{
			buffer[maxlength - 1] = '\0';
			return (maxlength - 1);
		}
		else
			return len;
	}

	tm GetCurDate(char *date, size_t len)
	{
		time_t t = time(NULL);
		tm curtime;
		localtime_s(&curtime, &t);
		strftime(date, len, "%H:%M:%S", &curtime);
		return curtime;
	}
	tm GetCurDate()
	{
		time_t t = time(NULL);
		tm curtime;
		localtime_s(&curtime, &t);
		return curtime;
	}

public:
	bool OnLevelInit(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f);
	void OnMapStart(char const *MapName);
	bool OnClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen);
	void OnClientDisconnect(edict_t *pEntity);
	bool HookInit(char *error, size_t maxlen);
	bool HookRemove(void);
	bool SetupHooks(void);
	void RemoveHooks(void);
	void SetComandClient(int client);
	static void OnSayChat(const CCommand &args);
	static void OnSayTeamChat(const CCommand &args);
	Resultat ClientOnSayChat(const char* msg, const int client, const bool team);
};

extern IPhraseCollection	*ipharases;
extern IServerTools			*servertools;
extern Ukr_coop				g_pUkrCoop;
extern IGameConfig			*g_pGameConf;
extern IBinTools			*g_pBinTools;

extern const sp_nativeinfo_t MyNative[];

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
