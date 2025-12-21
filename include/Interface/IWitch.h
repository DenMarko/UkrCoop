#ifndef _INCLUDE_WITCH_H_
#define _INCLUDE_WITCH_H_
#include "IInfected.h"

class WitchDying;
class ITerrorPlayer;

class IWitch : public IInfected
{
public:
	void SetHarasser(IBaseEntity *pEnt);
	void ChangeRageLevel(float flRale);
	bool IsHostileToMe(ITerrorPlayer* pPlayer);
	bool DoAttack(ITerrorPlayer *pVictim);

public:
    virtual ~IWitch() {};

	virtual ServerClass*			GetServerClass(void) = 0;
	virtual int						YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	bool					ShouldCollide( int collisionGroup, int contentsMask ) const = 0;
	virtual void					Spawn( void ) = 0;
	virtual void					Precache( void ) = 0;
	virtual const char				*GetFootstepSound(char const*,bool,float,bool) const = 0;
	virtual float     				GetFootstepRunThreshold(void)const = 0;
	virtual int						OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual ZombieClassType			GetClass(void) const = 0;
    virtual CBaseCombatCharacter	*GetEntity(void) const = 0;
    virtual void	           		Update(void) = 0;
	virtual	int 					DoBloodEffect(float, const CTakeDamageInfo &info, const Vector &vec3, trace_t *) = 0;
	virtual IWitchLocomotion		*GetLocomotionInterface(void) const = 0;
	virtual IWitchBody				*GetBodyInterface(void) const = 0;
	virtual IIntention				*GetIntentionInterface(void) const = 0;
	virtual IWitchVision			*GetVisionInterface(void) const = 0;
    virtual bool            		IsAbleToBlockMovementOf(INextBot const*)const = 0;
    virtual bool            		IsSacrificeFor(ZombieClassType)const = 0;
    virtual bool            		TryToCull(void) = 0;
    virtual float     				GetEyeOffsetUpdateInterval(void)const = 0;
    virtual void			 		CreateComponents(void) = 0;
    virtual void             		OnModelChanged(void) = 0;
    virtual WitchDying      		*CreateDeathAction(CTakeDamageInfo const&)const = 0;
};


#endif