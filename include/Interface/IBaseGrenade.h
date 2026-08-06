#ifndef _INCLUDE_BASE_GRENADE_H_
#define _INCLUDE_BASE_GRENADE_H_
#include "IBaseCombatCharacter.h"

class IBaseGrenade : public IBaseCombatCharacter
{
public:
    virtual ~IBaseGrenade() {}

    virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void 			Precache() = 0;
	virtual void			Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual int				BloodColor( void ) = 0;
	virtual void			Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual float			GetDamage() = 0;
	virtual void			SetDamage(float flDamage)  = 0;
	virtual void			NetworkStateChanged_m_fFlags(void) = 0;
	virtual void			NetworkStateChanged_m_fFlags(void *) = 0;
	virtual void			NetworkStateChanged_m_vecVelocity(void) = 0;
	virtual void			NetworkStateChanged_m_vecVelocity(void *) = 0;
    virtual void	        Explode(trace_t *pTrace, int bitsDamageType) = 0;
    virtual	void            BounceTouch(CBaseEntity *pOther) = 0;
    virtual void	        Detonate(void) = 0;
    virtual Vector          GetBlastForce(void) = 0;
    virtual void	        BounceSound(void) = 0;
    virtual float	        GetShakeAmplitude(void) = 0;
    virtual float	        GetShakeRadius(void) = 0;
    virtual float	        GetDamageRadius(void) = 0;
    virtual float	        SetDamageRadius(float) = 0;
};

#endif // _INCLUDE_BASE_GRENADE_H_