#ifndef _INCLUDE_TANK_ROCK_H_
#define _INCLUDE_TANK_ROCK_H_
#include "IBaseCSGrenadeProjectile.h"

class ITankRock : public IBaseCSGrenadeProjectile
{
public:
    virtual ~ITankRock() {}

	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity ) = 0;
    virtual	void            BounceTouch(CBaseEntity *pOther) = 0;
    virtual void	        Detonate(void) = 0;
};

#endif //_INCLUDE_TANK_ROCK_H_