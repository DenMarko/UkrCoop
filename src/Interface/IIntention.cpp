#include "IIntention.h"
#include "INextBot.h"

//------------------------------------------------------------------------------------------------------------------------
/**
 * Given a subject, return the world space position we should aim at
 */
Vector IIntention::SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			Vector result = query->SelectTargetPoint( me, subject );
			if ( result != vec3_origin )
			{
				return result;
			}
		}
	}	

	// no answer, use a reasonable position
	Vector threatMins, threatMaxs;
    IBaseEntity *pEnt = (IBaseEntity*)subject;

	pEnt->CollisionProp()->WorldSpaceAABB( &threatMins, &threatMaxs );
	Vector targetPoint = pEnt->GetAbsOrigin();
	targetPoint.z += 0.7f * ( threatMaxs.z - threatMins.z );

	return targetPoint;
}


//------------------------------------------------------------------------------------------------------------------------
/**
 * Given two threats, decide which one is more dangerous
 */
const CBaseCombatCharacter *IIntention::SelectMoreDangerousThreat( const INextBot *me, const CBaseCombatCharacter *subject, const CBaseCombatCharacter *threat1, const CBaseCombatCharacter *threat2 ) const
{
	for ( INextBotEventResponder *sub = FirstContainedResponder(); sub; sub = NextContainedResponder( sub ) )
	{
		const IContextualQuery *query = dynamic_cast< const IContextualQuery * >( sub );
		if ( query )
		{
			// return the response of the first responder that gives a definitive answer
			const CBaseCombatCharacter *result = query->SelectMoreDangerousThreat( me, subject, threat1, threat2 );
			if ( result )
			{
				return result;
			}
		}
	}

	// no specific decision was made - return closest threat as most dangerous
    IBaseEntity *pEnt = (IBaseEntity*)subject;
	IBaseEntity *pThreat1 = (IBaseEntity*)threat1;
	IBaseEntity *pThreat2 = (IBaseEntity*)threat2;

	float range1 = ( pEnt->GetAbsOrigin() - pThreat1->GetAbsOrigin() ).LengthSqr();
	float range2 = ( pEnt->GetAbsOrigin() - pThreat2->GetAbsOrigin() ).LengthSqr();

	if ( range1 < range2 )
	{
		return threat1;
	}

	return threat2;
}
