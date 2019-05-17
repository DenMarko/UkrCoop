#include "CHookEvent.h"

CHookEvent g_pHookEvent;

CHookEvent::CHookEvent()
{
}

CHookEvent::~CHookEvent()
{
}

void CHookEvent::HookEvent(const char *EventName, hookFunck FunckName)
{
	if (!EventName)
	{
		g_IUkrCoop->UkrCoop_LogMessage("[UkrPerks] Event Name is not valid, HookEvent is not added ty steck");
		return;
	}

	if (FunckName == nullptr)
	{
		g_IUkrCoop->UkrCoop_LogMessage("[UkrPerks] Event hook function is NULL, HookEvent is not added ty steck");
		return;
	}

	EventHooks info;
	info.name = EventName;
	info.func = &FunckName;

	m_EventHook.push_back(info);
}

bool CHookEvent::OnFireEvent(IGameEvent *pEvent, bool bDontBroatcast)
{
	if (!pEvent)
		RETURN_META_VALUE(MRES_IGNORED, false);

	EventHooks *info;
	const char *name = pEvent->GetName();
	auto iter = m_EventHook.begin();

	while (iter != m_EventHook.end())
	{
		info = &(*iter);
		if (strcmp(name, info->name) == 0)
		{
//			g_IUkrCoop->UkrCoop_LogMessage("[UkrPerks] Event is hook: %s", name);
			(pPerks->*(*info->func))(pEvent, bDontBroatcast);
		}
		iter++;
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}