#ifndef _INCLUDE_NEXT_BOT_EVENT_RESPONDER_H_
#define _INCLUDE_NEXT_BOT_EVENT_RESPONDER_H_

#include "extension.h"

class Path;
class CNavArea;
class CBaseEntity;
class CGameTrace;
class CTakeDamageInfo;
class CBaseCombatCharacter;
class Vector;
class KeyValues;
class AI_Response;

struct animevent_t;

typedef int AIConcept_t;

enum MoveToFailureType
{
	FAIL_NO_PATH_EXISTS,
	FAIL_STUCK,
	FAIL_FELL_OFF,
};

class INextBotEventResponder
{
public:
	virtual ~INextBotEventResponder() { }

	/**
	 * ці методи використовуються похідними класами для визначення того, як поширюються події
	 */
	virtual INextBotEventResponder *FirstContainedResponder( void ) const { return nullptr; }
	virtual INextBotEventResponder *NextContainedResponder( INextBotEventResponder *current ) const { return nullptr; }

	/**
	 * Події. Усі події мають бути «розширені» шляхом явного виклику похідного класу,
	 * щоб забезпечити поширення. Кожна подія повинна реалізувати своє поширення в цьому класі інтерфейсу.
	 */
	virtual void OnLeaveGround( CBaseEntity *ground );														// викликається, коли бот залишає землю з будь-якої причини
	virtual void OnLandOnGround( CBaseEntity *ground );														// викликається, коли бот приземляється на землю після того, як був у повітрі
	
	virtual void OnContact( CBaseEntity *other, CGameTrace *result = nullptr );								// викликається, коли бот торкається "іншого"
	
	virtual void OnMoveToSuccess( const Path *path );														// викликається, коли бот досягає кінця заданого шляху
	virtual void OnMoveToFailure( const Path *path, MoveToFailureType reason );								// викликається, коли бот не досягає кінця заданого шляху
	virtual void OnStuck( void );																			// викликається, коли бот застряє під час спроби рухатися
	virtual void OnUnStuck( void );																			// викликається, коли раніше застряглий бот відновлюється і може знову рухатися
	
	virtual void OnPostureChanged( void );																	// коли бот приймає нову позу (запит IBody для позиції)
	
	virtual void OnAnimationActivityComplete( int activity );												// коли анімаційна діяльність закінчилася
	virtual void OnAnimationActivityInterrupted( int activity );											// коли анімаційну діяльність було замінено іншою анімацією
	virtual void OnAnimationEvent( animevent_t *event );													// коли подія анімації QC-файлу запускається поточною послідовністю анімації
	
	virtual void OnIgnite( void );																			// коли бот починає горіти
	virtual void OnInjured( const CTakeDamageInfo &info );													// коли бот чимось пошкоджений
	virtual void OnKilled( const CTakeDamageInfo &info );													// коли здоров'я бота досягає нуля
	virtual void OnOtherKilled( CBaseCombatCharacter *victim, const CTakeDamageInfo &info );				// коли помре хтось інший
	
	virtual void OnSight( CBaseEntity *subject );															// коли суб’єкт спочатку входить у візуальне усвідомлення бота
	virtual void OnLostSight( CBaseEntity *subject );														// коли суб’єкт виходить, він потрапляє у візуальне усвідомлення бота
    
	virtual void OnThreatChanged(CBaseEntity *subject);
	
	virtual void OnSound( CBaseEntity *source, const Vector &pos, KeyValues *keys );						// коли суб'єкт видає звук. "pos" - це світові координати звуку. "ключі" взяті з GameData звуку
	virtual void OnSpokeConcept( CBaseCombatCharacter *who, AIConcept_t concepts, AI_Response *response);	// коли Актор говорить концепцію
	
	virtual void OnNavAreaChanged( CNavArea *newArea, CNavArea *oldArea );									// коли бот входить у нову область навігації
	
	virtual void OnModelChanged( void );																	// коли модель сутності була змінена
	
	virtual void OnPickUp( CBaseEntity *item, CBaseCombatCharacter *giver );								// коли щось додається до нашого інвентарю
	virtual void OnDrop( CBaseEntity *item );																// коли щось видаляється з нашого інвентарю
	virtual void OnShoved( CBaseEntity *pusher );															// "pusher" штовхнув мене
	virtual void OnBlinded( CBaseEntity *blinder );															// "blinder" засліпив мене спалахом світла
	
	virtual void OnCommandAttack( CBaseEntity *victim );													// атакувати дану сутність
	virtual void OnCommandApproach( const Vector &pos, float range = 0.0f );								// переміститися в межах даного положення
	virtual void OnCommandApproach( CBaseEntity *goal );													// слідувати за вказаним лідером
	virtual void OnCommandRetreat( CBaseEntity *threat, float range = 0.0f );								// відступити від загрози на відстані принаймні одиниць (0 == нескінченність)
	virtual void OnCommandPause( float duration = 0.0f );													// пауза на вказану тривалість (0 == назавжди)
	virtual void OnCommandResume( void );																	// відновити після паузи

};

inline void INextBotEventResponder::OnLeaveGround( CBaseEntity *ground )
{
	for(INextBotEventResponder* sub = FirstContainedResponder(); sub; sub = NextContainedResponder(sub))
	{
		sub->OnLeaveGround(ground);
	}
}

inline void INextBotEventResponder::OnLandOnGround( CBaseEntity *ground )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnLandOnGround( ground );
	}	
}

inline void INextBotEventResponder::OnContact( CBaseEntity *other, CGameTrace *result )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnContact( other, result );
	}
}

inline void INextBotEventResponder::OnMoveToSuccess( const Path *path )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnMoveToSuccess( path );
	}	
}

inline void INextBotEventResponder::OnMoveToFailure( const Path *path, MoveToFailureType reason )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnMoveToFailure( path, reason );
	}	
}

inline void INextBotEventResponder::OnStuck( void )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnStuck();
	}	
}

inline void INextBotEventResponder::OnUnStuck( void )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnUnStuck();
	}	
}

inline void INextBotEventResponder::OnPostureChanged( void )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnPostureChanged();
	}	
}

inline void INextBotEventResponder::OnAnimationActivityComplete( int activity )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnAnimationActivityComplete( activity );
	}	
}

inline void INextBotEventResponder::OnAnimationActivityInterrupted( int activity )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnAnimationActivityInterrupted( activity );
	}	
}

inline void INextBotEventResponder::OnAnimationEvent( animevent_t *event )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnAnimationEvent( event );
	}	
}

inline void INextBotEventResponder::OnIgnite( void )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnIgnite();
	}	
}

inline void INextBotEventResponder::OnInjured( const CTakeDamageInfo &info )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnInjured( info );
	}	
}

inline void INextBotEventResponder::OnKilled( const CTakeDamageInfo &info )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnKilled( info );
	}	
}

inline void INextBotEventResponder::OnOtherKilled( CBaseCombatCharacter *victim, const CTakeDamageInfo &info )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnOtherKilled( victim, info );
	}	
}

inline void INextBotEventResponder::OnSight( CBaseEntity *subject )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnSight( subject );
	}	
}

inline void INextBotEventResponder::OnLostSight( CBaseEntity *subject )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnLostSight( subject );
	}	
}

inline void INextBotEventResponder::OnThreatChanged(CBaseEntity *subject)
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnThreatChanged(subject);
	}
}

inline void INextBotEventResponder::OnSound( CBaseEntity *source, const Vector &pos, KeyValues *keys )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnSound( source, pos, keys );
	}	
}

inline void INextBotEventResponder::OnSpokeConcept( CBaseCombatCharacter *who, AIConcept_t concepts, AI_Response *response)
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnSpokeConcept( who, concepts, response );
	}	
}

inline void INextBotEventResponder::OnNavAreaChanged( CNavArea *newArea, CNavArea *oldArea )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnNavAreaChanged( newArea, oldArea );
	}	
}

inline void INextBotEventResponder::OnModelChanged( void )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnModelChanged();
	}	
}

inline void INextBotEventResponder::OnPickUp( CBaseEntity *item, CBaseCombatCharacter *giver )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnPickUp( item, giver );
	}	
}

inline void INextBotEventResponder::OnDrop( CBaseEntity *item )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnDrop( item );
	}
}

inline void INextBotEventResponder::OnShoved( CBaseEntity *pusher )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnShoved( pusher );
	}
}

inline void INextBotEventResponder::OnBlinded( CBaseEntity *blinder )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnBlinded( blinder );
	}	
}

inline void INextBotEventResponder::OnCommandAttack( CBaseEntity *victim )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnCommandAttack( victim );
	}	
}

inline void INextBotEventResponder::OnCommandApproach( const Vector &pos, float range )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnCommandApproach( pos, range );
	}	
}

inline void INextBotEventResponder::OnCommandApproach( CBaseEntity *goal )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnCommandApproach( goal );
	}	
}

inline void INextBotEventResponder::OnCommandRetreat( CBaseEntity *threat, float range )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnCommandRetreat( threat, range );
	}	
}

inline void INextBotEventResponder::OnCommandPause( float duration)
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnCommandPause( duration );
	}
}

inline void INextBotEventResponder::OnCommandResume( void )
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		sub->OnCommandResume();
	}
}


#endif