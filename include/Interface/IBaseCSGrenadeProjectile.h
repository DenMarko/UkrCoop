#ifndef _INCLUDE_BASE_CS_GRENADE_PROJECTILE_H_
#define _INCLUDE_BASE_CS_GRENADE_PROJECTILE_H_
#include "IBaseGrenade.h"

class IBaseCSGrenadeProjectile : public IBaseGrenade
{
public:
    virtual ~IBaseCSGrenadeProjectile() {}

    virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			PostConstructor( const char *szClassname ) = 0;
	virtual void			Splash(void) = 0;
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const = 0;
	virtual void			ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity ) = 0;
    virtual float	        GetShakeAmplitude(void) = 0;
};

#endif //_INCLUDE_BASE_CS_GRENADE_PROJECTILE_H_