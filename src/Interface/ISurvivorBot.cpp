#include "ISurvivorBot.h"
#include "CTrace.h"

class CTraceFilterIgnoreFriendlyCombatItems : public CTraceFilterSimples
{
public:

	CTraceFilterIgnoreFriendlyCombatItems( const IHandleEntity *passentity, int collisionGroup, int iIgnoreTeam, bool bIsProjectile = false )
		: CTraceFilterSimples( (IHandleEntity*)passentity, collisionGroup ), m_iIgnoreTeam( iIgnoreTeam )
	{
		m_bCallerIsProjectile = bIsProjectile;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		return CTraceFilterSimples::ShouldHitEntity( pServerEntity, contentsMask );
	}

	int m_iIgnoreTeam;
	bool m_bCallerIsProjectile;
};

bool ISurvivorBot::IsLineOfFireClear(const Vector &from, const Vector &to) const
{
    trace_t trace;
    NextBotTraceFilterIgnoreActors botFilter(NULL, COLLISION_GROUP_NONE);
    CTraceFilterIgnoreFriendlyCombatItems ignoreFriendlyCombatFilter(this, COLLISION_GROUP_NONE, GetTeamNumber());
    CTraceFilterChain filter(&botFilter, &ignoreFriendlyCombatFilter);

    util_TraceLine(from, to, MASK_SOLID_BRUSHONLY, &filter, &trace);

    return !trace.DidHit();
}

bool ISurvivorBot::IsLineOfFireClear(const Vector &where) const
{
    return IsLineOfFireClear(const_cast<ISurvivorBot*>(this)->EyePosition(), where);
}

bool ISurvivorBot::IsLineOfFireClear(const Vector &from, IBaseEntity *who) const
{
	trace_t trace;
	NextBotTraceFilterIgnoreActors botFilter( NULL, COLLISION_GROUP_NONE );
	CTraceFilterIgnoreFriendlyCombatItems ignoreFriendlyCombatFilter( this, COLLISION_GROUP_NONE, GetTeamNumber() );
	CTraceFilterChain filter( &botFilter, &ignoreFriendlyCombatFilter );

	util_TraceLine( from, who->WorldSpaceCenter(), MASK_SOLID_BRUSHONLY, &filter, &trace );

	return !trace.DidHit() || (trace.m_pEnt == (CBaseEntity*)who);
}

bool ISurvivorBot::IsLineOfFireClear(IBaseEntity *who) const
{
    return IsLineOfFireClear(const_cast<ISurvivorBot*>(this)->EyePosition(), who);
}
