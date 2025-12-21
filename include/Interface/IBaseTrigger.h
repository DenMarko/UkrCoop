#ifndef _HEADER_BASE_TRIGGER_INCLUDE_
#define _HEADER_BASE_TRIGGER_INCLUDE_
#include "IBaseToggle.h"

class IBaseTrigger : public IBaseToggle
{
public:
    virtual ~IBaseTrigger() {};

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void            PostClientActive( void ) = 0;
	virtual void			Activate( void ) = 0;
	virtual int				DrawDebugTextOverlays(void) = 0;
	virtual void            StartTouch(CBaseEntity *pOther) = 0;
	virtual void            EndTouch(CBaseEntity *pOther) = 0;
	virtual void			UpdateOnRemove( void ) = 0;

	virtual void            InputEnable( inputdata_t &inputdata ) = 0;
	virtual void            InputDisable( inputdata_t &inputdata ) = 0;
	virtual void            InputToggle( inputdata_t &inputdata ) = 0;
	virtual void            InputTouchTest ( inputdata_t &inputdata ) = 0;
	virtual void            InputStartTouch( inputdata_t &inputdata ) = 0;
	virtual void            InputEndTouch( inputdata_t &inputdata ) = 0;

	virtual bool            UsesFilter( void ) = 0;
	virtual bool            PassesTriggerFilters(CBaseEntity *pOther) = 0;
};

#endif // _HEADER_BASE_TRIGGER_INCLUDE_