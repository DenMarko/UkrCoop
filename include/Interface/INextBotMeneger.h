#ifndef _HEADER_NEXT_BOT_MENEGER_INCLUDE_
#define _HEADER_NEXT_BOT_MENEGER_INCLUDE_
#include "INextBot.h"
#include "IBasePlayer.h"


class INextBotMeneger
{
	friend class INextBot;
public:
	virtual ~INextBotMeneger() {}
	virtual void Update( void ) = 0;

	virtual void OnMapLoaded( void ) = 0;																		// when the server has changed maps
	virtual void OnRoundRestart( void ) = 0;																	// when the scenario restarts
	virtual void OnBeginChangeLevel( void ) = 0;																// when the server is about to change maps
	virtual void OnKilled( CBaseCombatCharacter *victim, const CTakeDamageInfo &info ) = 0;						// when an actor is killed
	virtual void OnSound( CBaseEntity *source, const Vector &pos, KeyValues *keys ) = 0;						// when an entity emits a sound
	virtual void OnSpokeConcept( CBaseCombatCharacter *who, AIConcept_t concepts, AI_Response *response ) = 0;	// when an Actor speaks a concept
	virtual void OnWeaponFired( CBaseCombatCharacter *whoFired, CBaseCombatWeapon *weapon ) = 0;				// when someone fires a weapon

	int GetNextBotCount( void ) const;
	bool IsDebugging(unsigned int type) const;
	void SetDebugTypes(NextBotDebugType type);
	void Select( INextBot *bot );					// mark bot as selected for further operations
	void DeselectAll( void );
	INextBot *GetSelected( void ) const;
	bool IsDebugFilterMatch( const INextBot *bot ) const;	// return true if the given bot matches the debug filter

	void CollectAllBot(CUtlVector<INextBot*> *botVector);

	/** 
	* ЗАСТАРЕЛО: використовуйте CollectAllBots(). 
	* Виконати функтор для кожного NextBot у системі. 
	* Якщо функтор повертає false, зупиніть ітерацію раніше 
	* і повертає false. 
	*/
	template< typename Functor >
	bool ForEachBot(Functor &func)
	{
		for(int i = m_botList.Head(); i != m_botList.InvalidIndex(); i = m_botList.Next(i))
		{
			if(!func(m_botList[i]))
			{
				return false;
			}
		}
		return true;
	}

	/** 
	* ЗАСТАРЕЛО: використовуйте CollectAllBots(). 
	* Виконати функтор для кожного NextBot у системі як 
	* ICBaseCombatCharacter. 
	* Якщо функтор повертає false, зупиніть ітерацію раніше 
	* і повертає false. 
	*/
	template < typename Functor >
	bool ForEachCombatCharacter( Functor &func )
	{
		for( int i=m_botList.Head(); i != m_botList.InvalidIndex(); i = m_botList.Next( i ) )
		{
			if ( !func( (IBaseCombatCharacter*)m_botList[i]->GetEntity() ) )
			{
				return false;
			}
		}

		return true;
	}

protected:
	int Register( INextBot* bot );
	void UnRegister( INextBot *bot );

	CUtlLinkedList< INextBot * > m_botList;				// list of all active NextBots
	int m_iUpdateTickrate;
	double m_CurUpdateStartTime;
	double m_SumFrameTime;

	unsigned int m_debugType;						// debug flags

	struct DebugFilter
	{
		int index;			// entindex
		enum { MAX_DEBUG_NAME_SIZE = 128 };
		char name[ MAX_DEBUG_NAME_SIZE ];
	};
	CUtlVector< DebugFilter > m_debugFilterList;

	INextBot *m_selectedBot;						// selected bot for further debug operations
};

inline int INextBotMeneger::GetNextBotCount(void) const
{
    return m_botList.Count();
}

inline bool INextBotMeneger::IsDebugging(unsigned int type) const
{
	if(type & m_debugType)
	{
		return true;
	}
	return false;
}

inline void INextBotMeneger::SetDebugTypes(NextBotDebugType type)
{
	m_debugType = (unsigned int)type;
}

inline void INextBotMeneger::Select(INextBot *bot)
{
	m_selectedBot = bot;
}

inline void INextBotMeneger::DeselectAll(void)
{
	m_selectedBot = nullptr;
}

inline INextBot *INextBotMeneger::GetSelected(void) const
{
    return m_selectedBot;
}

inline bool IsNullEnt(const edict_t* pent)
{
	return pent == nullptr || g_Sample.IndexOfEdict(pent) == 0;
}

template < typename Functor >
inline bool ForEachActor( Functor &func )
{
	// iterate all non-bot players
	for( int i=1; i<=g_pGlobals->maxClients; ++i )
	{
		IBasePlayer *player = (IBasePlayer*)gamehelpers->ReferenceToEntity( i );

		if ( player == NULL )
			continue;

		if ( IsNullEnt( player->edict() ) )
			continue;

		if ( !player->IsPlayer() )
			continue;

		if ( !player->IsConnected() )
			continue;

		// skip bots - ForEachCombatCharacter will catch them
		INextBot *bot = player->MyNextBotPointer();
		if ( bot )
		{
			continue;
		}

		if ( func( player ) == false )
		{
			return false;
		}
	}

	return reinterpret_cast<INextBotMeneger *>(g_CallHelper->GetTheNextBots())->ForEachCombatCharacter( func );
}

#endif