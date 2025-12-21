#ifndef _INCLUDE_BOSS_ZOMBIE_PLAYER_BOT_H_
#define _INCLUDE_BOSS_ZOMBIE_PLAYER_BOT_H_
#include "INextBotPlayer.h"

class IBossZombiePlayerBot : public INextBotPlayer<ITerrorPlayer>
{
public:
	ITerrorPlayer* ChooseVictim(ITerrorPlayer* pVictim);
	bool IsBehind(ITerrorPlayer* pVictim);
public:
    ~IBossZombiePlayerBot() {}

    virtual void                Spawn( void ) = 0;
    virtual void                PhysicsSimulate( void ) = 0;
	virtual int 			    OnLeaveActiveState(void) = 0;
	virtual int				    OnTeamChanged(int) = 0;
	virtual void 			    OnPreThinkObserverMode(void) = 0;
	virtual void				Update(void) = 0;
	virtual CBaseCombatCharacter*GetEntity(void) const = 0;
	virtual const char			*GetDebugIdentifier(void) const = 0;
    virtual bool                IsDebugFilterMatch( const char *filter ) const = 0;
    virtual bool                ShouldTouch( const CBaseEntity *object ) const = 0;
};

#endif