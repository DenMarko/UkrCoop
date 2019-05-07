#ifndef HEDER_FILE_HOOK_EVENT
#define HEDER_FILE_HOOK_EVENT

#ifdef _WIN32
#pragma once
#endif //_WIN32

#include "extension.h"
#include "igameevents.h"

typedef bool (_cdecl *hookFunck)(
						IGameEvent *pEvent,
						bool bDontBroatcast);

struct EventHooks
{
	const char *name;
	hookFunck func;
};

class CHookEvent
{
public:
	CHookEvent(void);
	~CHookEvent(void);

public: //public function
	void HookEvent(const char *EventName, hookFunck FunckName);
	bool OnFireEvent(IGameEvent *pEvent, bool bDontBroatcast);
private: //private values
	SourceHook::List<EventHooks> m_EventHook;
};

extern CHookEvent g_pHookEvent;

#endif //HEDER_FILE_HOOK_EVENT