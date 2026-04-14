#ifndef _HEADER_NEXT_BOT_INCLUDE_
#define _HEADER_NEXT_BOT_INCLUDE_

#include "INextBotEventResponder.h"
#include "IWitchLocomotion.h"
#include "IZombieBotVision.h"
#include "IIntention.h"
#include "IZombieBotBody.h"

enum NextBotDebugType 
{
	NEXTBOT_DEBUG_NONE	= 0,
	NEXTBOT_BEHAVIOR	= 0x0001,
	NEXTBOT_LOOK_AT		= 0x0002,
	NEXTBOT_PATH		= 0x0004,
	NEXTBOT_ANIMATION	= 0x0008,
	NEXTBOT_LOCOMOTION	= 0x0010,
	NEXTBOT_VISION		= 0x0020,
	NEXTBOT_HEARING		= 0x0040,
	NEXTBOT_EVENTS		= 0x0080,
	NEXTBOT_ERRORS		= 0x0100,		// when things go wrong, like being stuck

	NEXTBOT_DEBUG_ALL	= 0xFFFF
};

class INextBotEventResponder;
class IIntention;

class INextBotFilter
{
public:
	virtual bool IsSelected( const CBaseEntity *candidate ) const = 0;			// return true if this entity passes the filter
};

class INextBot : public INextBotEventResponder
{
public:
    virtual ~INextBot() {}
	
	int GetBotId() const;

    virtual INextBotEventResponder      *FirstContainedResponder( void ) const;
    virtual INextBotEventResponder      *NextContainedResponder( INextBotEventResponder *current ) const;
	virtual void                        Reset( void ) = 0;														// (EXTEND) reset to initial state
	virtual void                        Update( void ) = 0;														// (EXTEND) update internal state
	virtual void                        Upkeep( void ) = 0;														// (EXTEND) lightweight update guaranteed to occur every server tick
	virtual bool                        IsRemovedOnReset( void ) const = 0;										// remove this bot when the NextBot manager calls Reset

	void FlagForUpdate( bool b = true );
	bool IsFlaggedForUpdate();
	int GetTickLastUpdate() const;
	void SetTickLastUpdate( int );

	virtual CBaseCombatCharacter        *GetEntity( void ) const = 0;
	virtual class NextBotCombatCharacter *GetNextBotCombatCharacter( void ) const = 0;
	virtual class SurvivorBot           *MySurvivorBotPointer() const = 0;
	virtual ILocomotion                 *GetLocomotionInterface( void ) const = 0;
	virtual IBody                       *GetBodyInterface( void ) const = 0;
	virtual IIntention                  *GetIntentionInterface( void ) const = 0;
	virtual IVision                     *GetVisionInterface( void ) const = 0;
	virtual bool                        SetPosition( const Vector &pos ) = 0;
	virtual const Vector                &GetPosition( void ) const = 0;											// get the global position of the bot
	virtual bool                        IsEnemy( const CBaseCombatCharacter *them ) const = 0;					// return true if given entity is our enemy
	virtual bool                        IsFriend( const CBaseCombatCharacter *them ) const = 0;					// return true if given entity is our friend
	virtual bool                        IsSelf( const CBaseCombatCharacter *them ) const = 0;					// return true if 'them' is actually me
	virtual bool                        IsAbleToClimbOnto( const CBaseEntity *object ) const = 0;
	virtual bool                        IsAbleToBreak( const CBaseEntity *object ) const = 0;
	virtual bool                        IsAbleToBlockMovementOf( const INextBot *botInMotion ) const = 0;
	virtual bool                        ShouldTouch( const CBaseEntity *object ) const	= 0;
	virtual bool                        IsImmobile( void ) const = 0;											// return true if we haven't moved in awhile
	virtual float                       GetImmobileDuration( void ) const = 0;									// how long have we been immobile
	virtual void                        ClearImmobileStatus( void ) = 0;		
	virtual float                       GetImmobileSpeedThreshold( void ) const = 0;							// return units/second below which this actor is considered "immobile"
	virtual bool                        IsRangeLessThan( CBaseEntity *subject, float range ) const = 0;
	virtual bool                        IsRangeLessThan( const Vector &pos, float range ) const = 0;
	virtual bool                        IsRangeGreaterThan( CBaseEntity *subject, float range ) const = 0;
	virtual bool                        IsRangeGreaterThan( const Vector &pos, float range ) const = 0;
	virtual float                       GetRangeTo( CBaseEntity *subject ) const = 0;
	virtual float                       GetRangeTo( const Vector &pos ) const = 0;
	virtual float                       GetRangeSquaredTo( CBaseEntity *subject ) const = 0;
	virtual float                       GetRangeSquaredTo( const Vector &pos ) const = 0;
	virtual const char                  *GetDebugIdentifier( void ) const = 0;									// return the name of this bot for debugging purposes
	virtual bool                        IsDebugFilterMatch( const char *name ) const = 0;						// return true if we match the given debug symbol
	virtual void                        DisplayDebugText( const char *text ) const = 0;							// show a line of text on the bot in the world
	
	bool IsDebugging(unsigned int type) const;
	void DebugConColorMsg(NextBotDebugType eDebugType, const Color& color, const char *fmt, ...);

	enum {
		MAX_NEXTBOT_DEBUG_HISTORY = 100,
		MAX_NEXTBOT_DEBUG_LINE_LENGTH = 256,
	};
	struct NextBotDebugLineType
	{
		NextBotDebugType debugType;
		char data[ MAX_NEXTBOT_DEBUG_LINE_LENGTH ];
	};
	void GetDebugHistory( unsigned int type, CUtlVector< const NextBotDebugLineType * > *lines ) const;	// build a vector of debug history of the given types

	INextBotComponent *GetComponents() const
	{
		return m_componentList;
	}
	
	void RemoveComponent(INextBotComponent *comp);

private:
	friend class INextBotComponent;
	void RegisterComponent( INextBotComponent *comp );		// components call this to register themselves with the bot that contains them
	INextBotComponent *m_componentList;						// offset 1 the first component

	int m_id;												// offset 2
	bool m_bFlaggedForUpdate;								// offset 3
	int m_tickLastUpdate;									// offset 4

	unsigned int m_debugType;								// offset 5
	mutable int m_debugDisplayLine;							// offset 6

	Vector m_immobileAnchor;								// offset 7
	CountdownTimers m_immobileCheckTimer;					// offset 12
	IntervalTimers m_immobileTimer;							// offset 14
	void UpdateImmobileStatus( void );

	mutable ILocomotion m_baseLocomotion;					// offset 15
	mutable IBody		m_baseBody;							// offset 39
	mutable IIntention	m_baseIntention;					// offset 44
	mutable IVision		m_baseVision;						// offset 50

	void ResetDebugHistory( void );
	CUtlVector< NextBotDebugLineType * > m_debugHistory;	// offset 132
};

inline int INextBot::GetBotId() const
{
    return m_id;
}

inline void INextBot::FlagForUpdate( bool b )
{
	m_bFlaggedForUpdate = b;
}

inline bool INextBot::IsFlaggedForUpdate()
{
	return m_bFlaggedForUpdate;
}

inline int INextBot::GetTickLastUpdate() const
{
	return m_tickLastUpdate;
}

inline void INextBot::SetTickLastUpdate( int tick )
{
	m_tickLastUpdate = tick;
}

#endif