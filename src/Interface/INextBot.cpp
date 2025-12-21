#include "log_messege.h"
#include "INextBot.h"
#include "INextBotMeneger.h"

void INextBot::UpdateImmobileStatus(void)
{
	if ( m_immobileCheckTimer.IsElapsed() )
	{
		m_immobileCheckTimer.Start( 1.0f );

		// if we haven't moved farther than this in 1 second, we're immobile
		IBaseEntity *pEnt = (IBaseEntity*)GetEntity();
        if ( ( pEnt->GetAbsOrigin() - m_immobileAnchor ).IsLengthGreaterThan( GetImmobileSpeedThreshold() ) )
		{
			// moved far enough, not immobile
			m_immobileAnchor = pEnt->GetAbsOrigin();
			m_immobileTimer.Invalidate();
		}
		else
		{
			// haven't escaped our anchor - we are immobile
			if ( !m_immobileTimer.HasStarted() )
			{
				m_immobileTimer.Start();
			}
		}
	}
}

void INextBot::ResetDebugHistory(void)
{
	for ( int i=0; i<m_debugHistory.Count(); ++i )
	{
		delete m_debugHistory[i];
	}

	m_debugHistory.RemoveAll();
}

INextBotEventResponder *INextBot::FirstContainedResponder(void) const
{
    return m_componentList;
}

INextBotEventResponder *INextBot::NextContainedResponder(INextBotEventResponder *current) const
{
    return reinterpret_cast<INextBotComponent*>(current)->m_nextComponent;
}

void INextBot::GetDebugHistory(unsigned int type, CUtlVector<const NextBotDebugLineType *> *lines) const
{
	if ( !lines )
		return;

	lines->RemoveAll();

	for ( int i=0; i<m_debugHistory.Count(); ++i )
	{
		NextBotDebugLineType *line = m_debugHistory[i];
		if ( line->debugType & type )
		{
			lines->AddToTail( line );
		}
	}
}

void INextBot::RemoveComponent(INextBotComponent *comp)
{
	INextBotComponent *begin = m_componentList;
	if(begin == comp)
	{
		m_componentList = begin->m_nextComponent;
		delete begin;
		return;
	}
	INextBotComponent *temp = begin->m_nextComponent;
	while(temp != nullptr)
	{
		if(temp == comp)
		{
			begin->m_nextComponent = temp->m_nextComponent;
			delete temp;
			return;
		}
		begin = temp;
		temp = temp->m_nextComponent;
	}
}

void INextBot::RegisterComponent(INextBotComponent *comp)
{
	comp->m_nextComponent = m_componentList;
	m_componentList = comp;
}

bool INextBot::IsDebugging(unsigned int type) const
{
	if(ukr_next_bot_debug.GetBool())
	{
		return true;
	}

	INextBotMeneger *pMeneger = (INextBotMeneger *)g_CallHelper->GetTheNextBots();
	if(pMeneger)
	{
		if(pMeneger->IsDebugging(type))
		{
			return pMeneger->IsDebugFilterMatch(this);
		}
	}

    return false;
}

void INextBot::DebugConColorMsg(NextBotDebugType eDebugType, const Color &color, const char *fmt, ...)
{
	ConVarRef			NextBotDebugHistory("nb_debug_history");
	bool isDataFormatted = false;

	va_list argptr;
	char szData[256];

	if(IsDebugging(eDebugType))
	{
		va_start(argptr, fmt);
		Q_vsnprintf(szData, sizeof( szData ), fmt, argptr);
		va_end(argptr);
		isDataFormatted = true;

		if(ukr_next_bot_debug.GetBool())
			m_sLog->LogToFileEx(false, "%s", szData);
		else
			ConColorMsg( color, "%s", szData );
	}

	if ( !NextBotDebugHistory.GetBool() )
	{
		if ( m_debugHistory.Count() )
		{
			ResetDebugHistory();
		}
		return;
	}
	// Don't bother with event data - it's spammy enough to overshadow everything else.
	if ( eDebugType == NEXTBOT_EVENTS )
		return;

	if ( !isDataFormatted )
	{
		va_start(argptr, fmt);
		Q_vsnprintf(szData, sizeof( szData ), fmt, argptr);
		va_end(argptr);
		isDataFormatted = true;
	}

	int lastLine = m_debugHistory.Count() - 1;
	if ( lastLine >= 0 )
	{
		NextBotDebugLineType *line = m_debugHistory[lastLine];
		if ( line->debugType == eDebugType && V_strstr( line->data, "\n" ) == NULL )
		{
			// append onto previous line
			V_strncat( line->data, szData, MAX_NEXTBOT_DEBUG_LINE_LENGTH );
			return;
		}
	}

	// Prune out an old line if needed, keeping a pointer to re-use the memory
	NextBotDebugLineType *line = NULL;
	if ( m_debugHistory.Count() == MAX_NEXTBOT_DEBUG_HISTORY )
	{
		line = m_debugHistory[0];
		m_debugHistory.Remove( 0 );
	}

	// Add to debug history
	if ( !line )
	{
		line = new NextBotDebugLineType;
	}
	line->debugType = eDebugType;
	V_strncpy( line->data, szData, MAX_NEXTBOT_DEBUG_LINE_LENGTH );
	m_debugHistory.AddToTail( line );
}
