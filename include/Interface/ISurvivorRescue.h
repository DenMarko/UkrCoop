#ifndef _INCLUDE_BASE_SURVIVOR_RESCUE_H_
#define _INCLUDE_BASE_SURVIVOR_RESCUE_H_
#include "IBaseMultiplayerPlayer.h"
#include "../CGameEventListener.h"

enum DoorState_t
{
    DOOR_STATE_CLOSED = 0,
    DOOR_STATE_OPENING,
    DOOR_STATE_OPEN,
    DOOR_STATE_CLOSING,
    DOOR_STATE_AJAR,
};

class ISurvivorRescue : public IFlexExpresser, public CGameEventListeners
{
public:
	bool IsBehindClosedDoor(void);
	bool CloseDoors(void);
	inline const int GetSurvivorHere(void) const { return m_SurvivorHere; }
	IBaseEntity* GetSurvivor( void ) const;
	INavArea* GetRescueNavArea( void );
public:
    ~ISurvivorRescue() { }

    virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual int				UpdateTransmitState() = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual int				DrawDebugTextOverlays(void) = 0;
	virtual void			ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
	virtual Vector			EyePosition( void ) = 0;
    virtual void            FireGameEvent( IGameEvent *event ) = 0;

private:
	IHANDLES				m_survivor;					// 1992
	CountdownTimers 		m_Unknown_499;				// 1996
	float					m_fDistance_502;			// 2008
	int						m_SurvivorHere;				// 2012
	Vector					m_rescueEyePos;				// 2016
	CUtlVector<IHANDLES> 	m_hDoorList;				// 2028
};

#endif