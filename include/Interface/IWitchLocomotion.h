#ifndef _HEADER_WITCH_LOCOMOTION_INCLUDE_
#define _HEADER_WITCH_LOCOMOTION_INCLUDE_
#include "IZombieBotLocomotion.h"

class IWitchLocomotion : public IZombieBotLocomotion
{
public:
    virtual ~IWitchLocomotion() {}

	virtual void SetSpeedLimit( float speed ) = 0;
	virtual float GetSpeedLimit( void ) const = 0;
	virtual float GetRunSpeed( void ) const = 0;
	virtual bool IsAreaTraversable( const CNavArea *baseArea ) const = 0;
    virtual void SetVelocity(Vector const&) = 0;
};

#endif //_HEADER_WITCH_LOCOMOTION_INCLUDE_