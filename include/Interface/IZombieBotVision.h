#ifndef _INCLUDE_ZOMBIE_BOT_VISION_HEADER_
#define _INCLUDE_ZOMBIE_BOT_VISION_HEADER_
#include "IVision.h"

class IZombieBotVision : public IVision
{
public:
    virtual ~IZombieBotVision() {}

	virtual void        	Reset( void ) = 0;
	virtual void        	Update( void ) = 0;
    virtual CBaseEntity*	GetPrimaryRecognizedThreat(void) const = 0;
	virtual float       	GetTimeSinceVisible( int team ) const = 0;
    virtual CBaseEntity*	GetClosestRecognized(int) const = 0;
    virtual int         	GetRecognizedCount(int, float) const = 0;
    virtual CBaseEntity*	GetClosestRecognized(const INextBotEntityFilter&) const = 0;
	virtual float       	GetMaxVisionRange( void ) const = 0;
	virtual float       	GetMinRecognizeTime( void ) const = 0;
	virtual bool        	IsNoticed( CBaseEntity *subject ) const = 0;
	virtual bool        	IsIgnored( CBaseEntity *subject ) const = 0;
	virtual float       	GetDefaultFieldOfView( void ) const = 0;
    virtual float           GetNearNoticeRange( void )const = 0;
};

class IWitchVision : public IZombieBotVision
{
public:
    virtual ~IWitchVision() {}

	virtual bool        	IsIgnored( CBaseEntity *subject ) const = 0;
    virtual float           GetNearNoticeRange( void )const = 0;
};

#endif //_INCLUDE_ZOMBIE_BOT_VISION_HEADER_