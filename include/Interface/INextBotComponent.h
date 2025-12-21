#ifndef _INCLUDE_NEXT_BOT_COMPONENT_H_
#define _INCLUDE_NEXT_BOT_COMPONENT_H_
#include "INextBotEventResponder.h"
#include "edict.h"

class INextBot;

class INextBotReply
{
public:
	virtual void OnSuccess( INextBot *bot )  { }						// invoked when process completed successfully

	enum FailureReason
	{
		DENIED,
		INTERRUPTED,
		FAILED
	};
	virtual void OnFail( INextBot *bot, FailureReason reason ) { }		// invoked when process failed
};

class INextBotComponent : public INextBotEventResponder
{
	friend class INextBot;
public:
	INextBotComponent(INextBot* bot);
    virtual ~INextBotComponent() {};

	virtual void Reset( void )
    {
        extern CGlobalVars	*g_pGlobals;
        m_lastUpdateTime = 0;
        m_curInterval = g_pGlobals->interval_per_tick;
    }
    
	virtual void Update( void ) = 0;
	virtual void Upkeep( void ) {}

	inline bool ComputeUpdateInterval();								// return false is no time has elapsed (interval is zero)
	inline float GetUpdateInterval();

	virtual INextBot *GetBot( void ) const { return m_bot; }

private:
	float m_lastUpdateTime;
	float m_curInterval;

	INextBot *m_bot;
	INextBotComponent *m_nextComponent;									// simple linked list of components in the bot
};

inline bool INextBotComponent::ComputeUpdateInterval()
{
    extern CGlobalVars	*g_pGlobals;
    if(m_lastUpdateTime)
    {
        float interval = g_pGlobals->curtime - m_lastUpdateTime;
        const float minInterval = 0.0001f;
        if(interval > minInterval)
        {
            m_curInterval = interval;
            m_lastUpdateTime = g_pGlobals->curtime;
            return true;
        }
        return false;
    }
    m_curInterval = 0.033f;
    m_lastUpdateTime = g_pGlobals->curtime - m_curInterval;

    return true;
}

inline float INextBotComponent::GetUpdateInterval()
{
    return m_curInterval;
}

#endif