#include "INextBotComponent.h"
#include "INextBot.h"

INextBotComponent::INextBotComponent(INextBot *bot)
{
    m_curInterval = g_pGlobals->interval_per_tick;
    m_lastUpdateTime = 0;
    m_bot = bot;

    bot->RegisterComponent( this );
}

