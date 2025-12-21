#include "INextBotMeneger.h"

NOINLINE IBasePlayer *UTIL_GetListenerServerHost(void)
{
	if(engine->IsDedicatedServer())
	{
		return nullptr;
	}
	return (IBasePlayer *)gamehelpers->ReferenceToEntity(1);
}

bool INextBotMeneger::IsDebugFilterMatch(const INextBot *bot) const
{
	if ( m_debugFilterList.Count() == 0 )
	{
		return true;
	}

	for( int i = 0; i < m_debugFilterList.Count(); ++i )
	{
		if ( m_debugFilterList[i].index == reinterpret_cast<IBaseCombatCharacter *>(const_cast< INextBot * >( bot )->GetEntity())->entindex() )
		{
			return true;
		}

		if ( m_debugFilterList[i].name[0] != '\000' && bot->IsDebugFilterMatch( m_debugFilterList[i].name ) )
		{
			return true;
		}

		if ( !Q_strnicmp( m_debugFilterList[i].name, "lookat", Q_strlen( m_debugFilterList[i].name ) ) )
		{
			IBasePlayer *watcher = UTIL_GetListenerServerHost();
			if ( watcher )
			{
				CBaseEntity *subject = watcher->GetObserverTarget();

				if ( subject && bot->IsSelf( (CBaseCombatCharacter *)subject ) )
				{
					return true;
				}
			}
		}

		if ( !Q_strnicmp( m_debugFilterList[i].name, "selected", Q_strlen( m_debugFilterList[i].name ) ) )
		{
			INextBot *selected = GetSelected();
			if ( selected && bot->IsSelf( selected->GetEntity() ) )
			{
				return true;
			}
		}
	}

	return false;
}

void INextBotMeneger::CollectAllBot(CUtlVector<INextBot *> *botVector)
{
	if(!botVector)
		return;

	botVector->RemoveAll();

	for(int i = m_botList.Head(); i != m_botList.InvalidIndex(); i = m_botList.Next(i))
	{
		botVector->AddToTail( m_botList[i] );
	}
}

int INextBotMeneger::Register(INextBot *bot)
{
    return m_botList.AddToHead( bot );
}

void INextBotMeneger::UnRegister(INextBot *bot)
{
	m_botList.Remove( bot->GetBotId() );

	if( bot == m_selectedBot )
	{
		m_selectedBot = nullptr;
	}
}
