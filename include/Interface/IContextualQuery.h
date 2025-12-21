#ifndef _INCLUDE_CONTEXTUAL_QUERY_H_
#define _INCLUDE_CONTEXTUAL_QUERY_H_
#include "IBaseCombatCharacter.h"


class INextBot;
class PathFollower;

enum QueryResultType
{
	ANSWER_NO,
	ANSWER_YES,
	ANSWER_UNDEFINED
};

#define IS_ANY_HINDRANCE_POSSIBLE	( (CBaseEntity*)0xFFFFFFFF )

class IContextualQuery
{
public:
    virtual ~IContextualQuery() {}

	virtual QueryResultType				ShouldPickUp( const INextBot *me, CBaseEntity *item ) const;							// якщо потрібний товар був у наявності прямо зараз, ми повинні його забрати?
	virtual QueryResultType				ShouldHurry( const INextBot *me ) const;												// ми поспішаємо?
	virtual QueryResultType				IsHindrance( const INextBot *me, CBaseEntity *blocker ) const;							// повертає істину, якщо ми маємо чекати «блокувальника», який знаходиться на нашому шляху десь попереду.
	virtual Vector						SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const;		// враховуючи предмет, повернути позицію у світовому просторі, до якої ми повинні прагнути
	
	/**
	 * Дозволити боту затверджувати позиції, у які його намагається поставити рух гри. 
	 * Це найбільш корисно для ботів, отриманих від CBasePlayer, 
	 * які проходять через систему руху гравців.
	 */
	virtual QueryResultType         	IsPositionAllowed( const INextBot *me, const Vector &pos ) const;
    virtual PathFollower*				QueryCurrentPath( const INextBot *me ) const;
	virtual const CBaseCombatCharacter*	SelectMoreDangerousThreat( const INextBot *me, const CBaseCombatCharacter *subject, const CBaseCombatCharacter *threat1, const CBaseCombatCharacter *threat2 ) const; // повернути більш небезпечну з двох загроз у 'subject' або NULL, якщо у нас немає думки
};

inline QueryResultType IContextualQuery::ShouldPickUp(const INextBot* me, CBaseEntity* item) const
{
	return ANSWER_UNDEFINED;
}

inline QueryResultType IContextualQuery::ShouldHurry( const INextBot *me ) const
{
	return ANSWER_UNDEFINED;
}

inline QueryResultType	IContextualQuery::IsHindrance( const INextBot *me, CBaseEntity *blocker ) const
{
	return ANSWER_UNDEFINED;
}

inline Vector	IContextualQuery::SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const
{
	return Vector(0.f);
}

inline QueryResultType  IContextualQuery::IsPositionAllowed( const INextBot *me, const Vector &pos ) const
{
	return ANSWER_UNDEFINED;
}

inline PathFollower* IContextualQuery::QueryCurrentPath( const INextBot *me ) const
{
	return nullptr;
}

inline const CBaseCombatCharacter* IContextualQuery::SelectMoreDangerousThreat( const INextBot *me, const CBaseCombatCharacter *subject, const CBaseCombatCharacter *threat1, const CBaseCombatCharacter *threat2 ) const
{
	return NULL;
}

#endif