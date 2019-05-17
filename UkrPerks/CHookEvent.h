#ifndef HEDER_FILE_HOOK_EVENT
#define HEDER_FILE_HOOK_EVENT

#ifdef _WIN32
#pragma once
#endif //_WIN32

#include "CPerks.h"

struct EventHooks
{
	const char *name;
	hookFunck *func;
};

class CHookEvent
{
public:
	CHookEvent();
	~CHookEvent();
public: //public function
	void HookEvent(const char *EventName, hookFunck FunckName);
	bool OnFireEvent(IGameEvent *pEvent, bool bDontBroatcast);
private: //private values
	SourceHook::List<EventHooks> m_EventHook;
};

extern CHookEvent g_pHookEvent;
#endif // HEDER_FILE_HOOK_EVENT