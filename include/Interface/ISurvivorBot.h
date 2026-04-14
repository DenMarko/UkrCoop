#ifndef _INCLUDE_SURVIVOR_BOT_H_
#define _INCLUDE_SURVIVOR_BOT_H_
#include "INextBotPlayer.h"

class SurvivorBot;

class ISurvivorBot : public INextBotPlayer<ITerrorPlayer>
{
public:
	bool IsLineOfFireClear( const Vector& from, const Vector& to ) const;
	bool IsLineOfFireClear( const Vector &where ) const;
	bool IsLineOfFireClear( const Vector &from, IBaseEntity *who ) const;
	bool IsLineOfFireClear( IBaseEntity *who ) const;
	
public:
	~ISurvivorBot() {}

	virtual ServerClass     	*GetServerClass(void) = 0;
	virtual int					YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual datamap_t*			GetDataDescMap(void) = 0;
	virtual void				Spawn( void ) = 0;
	virtual void				Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual void				PhysicsSimulate( void ) = 0;
	virtual void				ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
	virtual	bool				Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 ) = 0;
	virtual void 				OnNavAreaChanged(CNavArea *,CNavArea *) = 0;
	virtual void				InitialSpawn( void ) = 0;
	virtual int					OnTeamChanged(int) = 0;
	virtual void				OnPlayerKilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual int 				OnBeginChangeLevel(char const*, KeyValues *) = 0;
	virtual unsigned int		OnAwardEarned(int, CBaseEntity *) = 0;
	virtual void				Update(void) = 0;
	virtual SurvivorBot 		*MySurvivorBotPointer(void) const = 0;
	virtual CBaseCombatCharacter*GetEntity(void) const = 0;
	virtual ILocomotion			*GetLocomotionInterface(void) const = 0;
	virtual IBody 				*GetBodyInterface(void) const = 0;
	virtual IIntention			*GetIntentionInterface(void) const = 0;
	virtual IVision				*GetVisionInterface(void) const = 0;
	virtual bool				IsAbleToBreak(const CBaseEntity *) const = 0;
	virtual const char			*GetDebugIdentifier(void) const = 0;
};

#endif