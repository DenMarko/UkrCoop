#ifndef _INCLUDE_NEXT_BOT_COMBAT_CHARACTER_H_
#define _INCLUDE_NEXT_BOT_COMBAT_CHARACTER_H_
#include "INextBot.h"

class INextBotCombatCharacter : 
	public IBaseCombatCharacter,
	public INextBot
{
public:
    virtual ~INextBotCombatCharacter() { }

	virtual ServerClass						*GetServerClass(void) = 0;
	virtual int								YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t						*GetDataDescMap(void) = 0;
	virtual void 							Spawn() = 0;
	virtual void 							SetModel( const char *szModelName ) = 0;
	virtual void							Event_Killed( const CTakeDamageInfo &info ) = 0;
    virtual INextBot						*MyNextBotPointer(void);
	virtual void							Touch( CBaseEntity *pOther ) = 0;
	virtual Vector							EyePosition( void ) = 0;
	virtual void							PerformCustomPhysics( Vector *pNewPosition, Vector *pNewVelocity, QAngle *pNewAngles, QAngle *pNewAngVelocity ) = 0;
	virtual void 							HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual void 							Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false ) = 0;
	virtual int								OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual int								OnTakeDamage_Dying( const CTakeDamageInfo &info ) = 0;
	virtual bool							BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector ) = 0;
	virtual bool            				IsAreaTraversable(const CNavArea *area)const = 0;
	virtual void            				OnNavAreaChanged(CNavArea *enteredArea, CNavArea *leftArea) = 0;
    virtual void            				Ignite(float flFlameLifetime, CBaseEntity *pAttacker) = 0;
    virtual bool            				IsUseableEntity(CBaseEntity *entity, unsigned int requiredCaps = 0) = 0;
    virtual CBaseCombatCharacter			*GetLastAttacker(void) const = 0;
    virtual CBaseCombatCharacter			*GetEntity(void) const = 0;
    virtual class NextBotCombatCharacter	*GetNextBotCombatCharacter(void) const = 0;

	int GetLastHitGroup( void ) const;
private:
	int m_UnknownAD4_D60[163];				// 2772 - 3420 невідомі зміні
	bool m_UnknownD60;						// 3424

	EHANDLE m_lastAttacker;
	bool m_didModelChange;
};

inline int INextBotCombatCharacter::GetLastHitGroup( void ) const
{
	return LastHitGroup();
}

#endif