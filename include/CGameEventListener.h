#ifndef _HEADER_GAME_EVENT_LISTENER_INCLUDE_
#define _HEADER_GAME_EVENT_LISTENER_INCLUDE_
#include <igameevents.h>

extern IGameEventManager2	*gameevents;

class CGameEventListeners : public IGameEventListener2
{
public:
    CGameEventListeners() : m_bRegisteredForEvents(false) { m_nDebugID = EVENT_DEBUG_ID_INIT;}
    ~CGameEventListeners() { m_nDebugID = EVENT_DEBUG_ID_SHUTDOWN; StopListeningForAllEvents(); }

	void ListenForGameEvent( const char *szName )
	{
		m_bRegisteredForEvents = true;
		if ( gameevents )
		{
			gameevents->AddListener( this, szName, true );
		}
	}

	void StopListeningForAllEvents()
	{
		if ( m_bRegisteredForEvents )
		{
			if ( gameevents )
				gameevents->RemoveListener( this );
			m_bRegisteredForEvents = false;
		}
	}

	virtual void FireGameEvent( IGameEvent *event ) = 0;
    virtual int	 GetEventDebugID( void ) override { return m_nDebugID; }
private:
	int m_nDebugID;
    bool m_bRegisteredForEvents;
};

namespace EVENTS
{
	class CBaseEvent 
	{
	public:
		CBaseEvent() : pEvent(nullptr) {}

		CBaseEvent(const char* szEventName)
		{
			pEvent = gameevents->CreateEvent(szEventName);
		}

		virtual ~CBaseEvent()
		{
			if(pEvent)
			{
				gameevents->FireEvent(pEvent);
			}
		}
	protected:
		IGameEvent* pEvent;
	};
}


#endif