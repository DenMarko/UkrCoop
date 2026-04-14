#ifndef IPLAYERANIMSTATE_H
#define IPLAYERANIMSTATE_H

#include "ICSPlayer.h"

class IPlayerAnimState
{
public:

    virtual void    OnNewModel() = 0;
	virtual void    Update( float eyeYaw, float eyePitch ) = 0;
	virtual void    ClearAnimationState() = 0;
	virtual const QAngle& GetRenderAngles() = 0;

    virtual void    DoAnimationEvent( PlayerAnimEvent_t event, int nData, int*, int* ) = 0;
    virtual bool    IsThrowingGrenade( void ) = 0;
    virtual bool    IsPlayingGrenadeThrowAnim( void ) = 0;
    virtual bool    IsHealing( void ) = 0;
    virtual bool    IsJumping( void ) = 0;
    virtual bool    IsIncapAnimFinished( void ) = 0;
    virtual bool    IsClimbingLedge( void ) = 0;
    virtual void    GetAccumulatedMotion( Vector *pVec ) = 0;
    virtual void    ClearAccumulatedMotion( void ) = 0;     // 56
    virtual bool    ShouldHideWeapon( void ) = 0;
    virtual void    FireEvent( const Vector &vecOrigin, const QAngle &angAngles, int iEvent, const char *pszOptions ) = 0;
    virtual void    OnStaggerStart( void ) = 0; // 68
    virtual void    OnMainSequenceChanged( int iNewSequence ) = 0;
};

#endif // IPLAYERANIMSTATE_H