#include "HookEvent.h"
#include "log_messege.h"

CHookEvent::CHookEvent(void)
{
}

CHookEvent::~CHookEvent(void)
{
	m_EventHook.clear();
}

void CHookEvent::HookEvent(const char *EventName, hookFunck FunckName)
{
	if(!EventName)
	{
		m_sLog.LogToFileEx("Event Name is not valid, HookEvent is not added ty steck");
		return;
	}

	if(FunckName == nullptr)
	{
		m_sLog.LogToFileEx("Event hook function is NULL, HookEvent is not added ty steck");
		return;
	}

	EventHooks info;
	info.name = EventName;
	info.func = FunckName;

	m_EventHook.push_back(info);
}

bool CHookEvent::OnFireEvent(IGameEvent *pEvent, bool bDontBroatcast)
{
	if(!pEvent)
		RETURN_META_VALUE(MRES_IGNORED, false);

	EventHooks *info;
	const char *name = pEvent->GetName();
	auto iter = m_EventHook.begin();

	while (iter != m_EventHook.end())
	{
		info = &(*iter);
		if(strcmp(name, info->name) == 0)
		{
			m_sLog.LogToFileEx("Event is hook: %s", name);
			info->func(pEvent, bDontBroatcast);
		}
		iter++;
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}