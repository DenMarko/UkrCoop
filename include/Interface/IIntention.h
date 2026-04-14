#ifndef _INCLUDE_INTENTION_H_
#define _INCLUDE_INTENTION_H_
#include "IContextualQuery.h"
#include "INextBotComponent.h"

class IIntention : public INextBotComponent, public IContextualQuery
{
public:
	IIntention(INextBot *bot) : INextBotComponent(bot) {}
    virtual ~IIntention() {}

	virtual void                        Reset( void ) { INextBotComponent::Reset(); }
	virtual void                        Update( void ) {}

	virtual QueryResultType			    ShouldPickUp( const INextBot *me, CBaseEntity *item ) const;
	virtual QueryResultType			    ShouldHurry( const INextBot *me ) const;
	virtual QueryResultType			    IsHindrance( const INextBot *me, CBaseEntity *blocker ) const;
	virtual Vector					    SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const;
	virtual QueryResultType             IsPositionAllowed( const INextBot *me, const Vector &pos ) const;
    virtual PathFollower             	*QueryCurrentPath(INextBot const*) const;
	virtual const CBaseCombatCharacter*	SelectMoreDangerousThreat( const INextBot *me, const CBaseCombatCharacter *subject, const CBaseCombatCharacter *threat1, const CBaseCombatCharacter *threat2 ) const;
};

inline QueryResultType IIntention::ShouldPickUp(const INextBot *me, CBaseEntity *item) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			QueryResultType result = query->ShouldPickUp( me, item );
			if ( result != ANSWER_UNDEFINED )
			{
				return result;
			}
		}
	}	
	return ANSWER_UNDEFINED;
}


inline QueryResultType IIntention::ShouldHurry( const INextBot *me ) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			QueryResultType result = query->ShouldHurry( me );
			if ( result != ANSWER_UNDEFINED )
			{
				return result;
			}
		}
	}	
	return ANSWER_UNDEFINED;
}

inline QueryResultType IIntention::IsHindrance( const INextBot *me, CBaseEntity *blocker ) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			QueryResultType result = query->IsHindrance( me, blocker );
			if ( result != ANSWER_UNDEFINED )
			{
				return result;
			}
		}
	}	
	return ANSWER_UNDEFINED;
}

inline QueryResultType IIntention::IsPositionAllowed( const INextBot *me, const Vector &pos ) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			QueryResultType result = query->IsPositionAllowed( me, pos );
			if ( result != ANSWER_UNDEFINED )
			{
				return result;
			}
		}
	}	
	return ANSWER_UNDEFINED;
}

inline PathFollower *IIntention::QueryCurrentPath(const INextBot *me) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			PathFollower *result = query->QueryCurrentPath( me );
			if ( result != nullptr )
			{
				return result;
			}
		}
	}	
	return nullptr;
}


#endif