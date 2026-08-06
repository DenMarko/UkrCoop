#ifndef _INCLUDE_BASE_PROP_DOOR_H_
#define _INCLUDE_BASE_PROP_DOOR_H_
#include "IProps.h"
#include "../CGameEventListener.h"

struct opendata_t
{
	Vector vecStandPos;		// Where the NPC should stand.
	Vector vecFaceDir;		// What direction the NPC should face.
	Activity eActivity;		// What activity the NPC should play.
};

class IBasePropDoor : public IDynamicProp, public CGameEventListeners
{
public:
    virtual ~IBasePropDoor() {}

	virtual ServerClass*                GetServerClass(void) = 0;
	virtual int							YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*                  GetDataDescMap(void) = 0;
	virtual bool			            TestCollision( const Ray_t& ray, unsigned int mask, trace_t& trace ) = 0;
	virtual void                        Spawn( void ) = 0;
	virtual void                        Precache( void ) = 0;
	virtual void                        Activate( void ) = 0;
	virtual int				            ObjectCaps( void ) = 0;
	virtual void			            Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual void			            StartBlocked( CBaseEntity *pOther ) = 0;
	virtual void			            Blocked( CBaseEntity *pOther ) = 0;
	virtual void			            EndBlocked( void ) = 0;
	virtual void 			            HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual void                        FireGameEvent( IGameEvent *event ) = 0;

    virtual bool	IsCheckpointDoor(void)const = 0;
    virtual bool	IsCheckpointExitDoor(void)const = 0;
    virtual bool	IsAbleToCloseAreaPortals(void)const = 0;
    virtual bool	IsUsableByTeam(int)const = 0;
    virtual void	OnCheckpointDoorUnlocked(CBaseEntity *) = 0;
    virtual bool	DoorCanClose( bool bAutoClose ) = 0;
    virtual bool	DoorCanOpen(void) = 0;
	virtual void    GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata) = 0;
	virtual float   GetOpenInterval(void) = 0;
	virtual void    ComputeDoorExtent( void *extent, unsigned int extentType ) = 0;	// extent contains the volume encompassing by the door in the specified states
    virtual void	OnDoorClosed(void) = 0;
    virtual void	OnDoorOpened(void) = 0;
	virtual void    BeginOpening(CBaseEntity *pOpenAwayFrom) = 0;
	virtual void    BeginClosing( void ) = 0;
	virtual void    DoorStop( void ) = 0;
	virtual void    DoorResume( void ) = 0;
	virtual void    DoorTeleportToSpawnPosition() = 0;
    virtual bool	TryOpenClose(CBaseEntity *,CBaseEntity *,int) = 0;

};

#endif //_INCLUDE_BASE_PROP_DOOR_H_