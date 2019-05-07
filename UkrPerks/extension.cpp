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

#include "extension.h"
#include "CPerks.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Sample				g_Sample;		/**< Global singleton for extension's main interface */
ICvar				*icvar = nullptr;
IGameEventManager2	*gameevents = nullptr;
SourceMod::UkrCoop	*g_IUkrCoop = nullptr;
IPhraseCollection	*ipharases = nullptr;


SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, false, bool, IGameEvent *, bool);

SMEXT_LINK(&g_Sample);

bool Sample::SDK_OnLoad(char * error, size_t maxlen, bool late)
{
	if (Q_strcmp(g_pSM->GetGameFolderName(), "left4dead") != 0)
	{
		return false;
	}

	sharesys->AddDependency(myself, "UkrCoop.ext", true, true);

	ipharases = translator->CreatePhraseCollection();
	ipharases->AddPhraseFile("perks.phrases");
	ipharases->AddPhraseFile("common.pharases");


	return true;
}

void Sample::SDK_OnUnload()
{
}

void Sample::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(UKRCOOP, g_IUkrCoop);
	if (!g_IUkrCoop)
		return;
}

void Sample::SDK_OnPauseChange(bool paused)
{
}

bool Sample::QueryRunning(char * error, size_t maxlength)
{
	SM_CHECK_IFACE(UKRCOOP, g_IUkrCoop);
	return true;
}

bool Sample::QueryInterfaceDrop(SMInterface * pInterface)
{
	if(pInterface == g_IUkrCoop)
		return false;

	return IExtensionInterface::QueryInterfaceDrop(pInterface);
}

bool Sample::SDK_OnMetamodLoad(ISmmAPI * ismm, char * error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);

	g_pCVar = icvar;
	ConVar_Register(0, this);

	SH_ADD_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(&g_pHookEvent, &CHookEvent::OnFireEvent), false);

	return true;
}

bool Sample::SDK_OnMetamodUnload(char * error, size_t maxlen)
{
	SH_REMOVE_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(&g_pHookEvent, &CHookEvent::OnFireEvent), false);


	return true;
}

bool Sample::SDK_OnMetamodPauseChange(bool paused, char * error, size_t maxlen)
{
	return true;
}

bool Sample::RegisterConCommandBase(ConCommandBase * pVar)
{
	return META_REGCVAR(pVar);
}

bool Sample::Translate(char *buffer, size_t maxlength, const char *format, unsigned int numparams, size_t *pOutLength, ...)
{
	va_list ap;
	unsigned int i;
	const char *fail_phrase;
	void *params[MAX_TRANSLATE_PARAMS];

	if (numparams > MAX_TRANSLATE_PARAMS)
	{
		assert(false);
		return false;
	}

	va_start(ap, pOutLength);
	for (i = 0; i < numparams; i++)
	{
		params[i] = va_arg(ap, void *);
	}
	va_end(ap);

	if (!ipharases->FormatString(buffer, maxlength, format, params, numparams, pOutLength, &fail_phrase))
	{
		if (fail_phrase != NULL)
		{
			g_pSM->LogError(myself, "[UkrCoop] Could not find core phrase: %s", fail_phrase);
		}
		else {
			g_pSM->LogError(myself, "[UkrCoop] Unknown fatal error while translating a core phrase.");
		}
		return false;
	}
	return true;
}
