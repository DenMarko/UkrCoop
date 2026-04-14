#ifndef _HEADER_BASE_TOGGLE_INCLUDE_
#define _HEADER_BASE_TOGGLE_INCLUDE_
#include "IBaseEntity.h"

class IBaseToggle : public IBaseEntity
{
public:
    virtual ~IBaseToggle() {};

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual bool		    KeyValue( const char *szKeyName, const char *szValue ) = 0;
	virtual bool		    KeyValue( const char *szKeyName, float flValue ) = 0;
	virtual void            MoveDone( void ) = 0;
	virtual float	        GetDelay( void ) = 0;
	virtual bool			IsLockedByMaster( void ) = 0;
	virtual Vector*			GetGroundVelocityToApply(Vector &) = 0;
	virtual bool			KeyValue( const char *szKeyName, Vector vecValue ) = 0;
};


#endif // _HEADER_BASE_TOGGLE_INCLUDE_