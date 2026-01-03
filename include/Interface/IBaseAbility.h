#ifndef _INCLUDE_BASE_ABILITY_
#define _INCLUDE_BASE_ABILITY_
#include "IBaseEntity.h"
#include "ITerrorPlayer.h"

class CMoveData;

class IBaseAbility : public IBaseEntity
{
    DECLARE_CLASS( IBaseAbility, IBaseEntity );
public:
    virtual ~IBaseAbility() { }

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo ) = 0;
	virtual int				UpdateTransmitState() = 0;
	virtual void			UpdateOnRemove( void ) = 0;
    virtual void            OnOwnerLeaveActiveState(void) = 0;
    virtual void            OnOwnerChanged(CTerrorPlayer *) = 0;
    virtual bool            IsAbilityReadyToFire(void)const = 0;
    virtual void            ActivateAbility(void) = 0;
    virtual void            UpdateAbility(void) = 0;
    virtual void            OnCreate(CTerrorPlayer *) = 0;
    virtual bool            HasAbilityTarget(void)const = 0;
    virtual void            SetSupressionTimer(float) = 0;
    virtual float           GetActivationTimeRemaining(void)const = 0;
    virtual int             GetButton(void)const = 0;
    virtual int             OnButtonPressed(void) = 0;
    virtual void            OnButtonReleased(void) = 0;
    virtual bool            IsActive(void)const = 0;
    virtual bool            IsPredicted(void)const = 0;
    virtual Vector          GetJumpVector(bool) = 0;
    virtual void            OnTouch(CBaseEntity *) = 0;
    virtual void            OnCrouched(void) = 0;
    virtual void            OnCrouchStart(void) = 0;
    virtual float           GetSpeedOverride(void)const = 0;
    virtual float           GetFrictionMultiplier(void)const = 0;
    virtual int             CanPlayerMove(void)const  = 0;
    virtual void            OnOwnerTakeDamage(const CTakeDamageInfo &info) = 0;
    virtual void            Operator_HandleAnimEvent(animevent_t *, IBaseCombatCharacter *) = 0;
    virtual void            OnStunned(float duration) = 0;
    virtual int             HandleCustomCollision(CBaseEntity *,Vector const&,Vector const&,CGameTrace *,CMoveData *) = 0;
    virtual void            OnDestroy(void) = 0;
    virtual void            AbilityDebug(char const* msg, ...) = 0;
    virtual void            AbilityDebug(CTerrorPlayer *,char const* msh, ...) = 0;

    void                    StartActivationTimer(float flDelay, float flDuration);

protected:
    CNetworkVarEmbedded(CountdownTimers,    m_activationSupressedTimer);    // 892
    CNetworkHandle(IBaseEntity,             m_owner);                       // 904
    CNetworkVarEmbedded(CountdownTimers,    m_nextActivationTimer);         // 908
    CNetworkVar(bool,                       m_hasBeenUsed);                 // 920
};


class ITongue : public IBaseAbility
{
    DECLARE_CLASS( ITongue, IBaseAbility );
public:
    virtual ~ITongue() { }

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo ) = 0;
    virtual void            OnOwnerLeaveActiveState(void) = 0;
    virtual bool            IsAbilityReadyToFire(void)const = 0;
    virtual void            ActivateAbility(void) = 0;
    virtual void            UpdateAbility(void) = 0;
    virtual void            OnCreate(CTerrorPlayer *) = 0;
    virtual bool            HasAbilityTarget(void)const = 0;
    virtual int             GetButton(void)const = 0;
    virtual int             OnButtonPressed(void) = 0;
    virtual void            OnTouch(CBaseEntity *pEntity) = 0;
    virtual void            OnOwnerTakeDamage(CTakeDamageInfo const& info) = 0;
    virtual void            AbilityDebug(char const *msg, ...) = 0;
    virtual void            AbilityDebug(CTerrorPlayer *pPlayer, char const* msg, ...) = 0;
    virtual void            OnTongueShot(CTerrorPlayer *pPlayer, float, Vector const&, Vector const&) = 0;

    void                    ResetTongueTimer();
    float                   GetAbilityActivationDelay();
    void                    SnapTongueBackToMouth();

    const Vector            GetLastVictimPosition() const;

private:
    CNetworkVar(int, m_tongueGrabStartingHealth);                   //  924
    CNetworkVar(float, m_tongueHitRange);                           //  928
    CNetworkVar(float, m_tongueHitTimestamp);                       //  932
    CNetworkVar(int, m_tongueHitWasAmbush);                         //  936
    CNetworkVar(float, m_tongueVictimDistance);                     //  940
    CNetworkVar(float, m_tongueVictimPositionTime);                 //  944
    CNetworkVar(float, m_tongueVictimLastOnGroundTime);             //  948
    CNetworkHandle(IBaseEntity, m_potentialTarget);                 //  952
    CNetworkHandle(IBaseEntity, m_currentTipTarget);                //  956
    CountdownTimers m_attackRetryTimer;                             //  960 ?
    IntervalTimers m_groundChokeTimer;                              //  972 ?
    IntervalTimers m_airChokeTimer;                                 //  980 ?
    Vector m_smokerMouthPosition;                                   //  988 ?
    CNetworkVar(int, m_tongueState);                                //  1000
    void* m_pCurrentStateInfo;                                      //  1004 ?
    IntervalTimers m_stateEnterTimer;                               //  1008 ?
    CNetworkArray(Vector, m_bendPositions, 10);                     //  1016
    CNetworkVar(int, m_bendPointCount);                             //  1136
    CNetworkVector(m_tipPosition);                                  //  1140
    Vector m_currentTongueTargetPosition;                           //  1152 ?
    Vector m_targetPosition;                                        //  1164 ?
    Vector m_tongueVelocity;                                        //  1176 ?
    float m_tongueHealth;                                           //  1188 ?
    int m_pulledSurvivorCharacter;                                  //  1192 ?
    int m_chokedSurvivorCharacter;                                  //  1196 ?
    Vector m_cachedGroundPosition;                                  //  1200 ?
    Vector m_cachedPullDestination;                                 //  1212 ?
    Vector m_lastValidVictimPosition;                               //  1224 ?
};

inline const Vector ITongue::GetLastVictimPosition() const
{
    return m_lastValidVictimPosition;
}

class IThrow : public IBaseAbility
{
public:
    virtual ~IThrow() { }

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Precache( void ) = 0;
    virtual void            ActivateAbility(void) = 0;
    virtual void            UpdateAbility(void) = 0;
    virtual void            OnCreate(CTerrorPlayer *) = 0;
    virtual bool            HasAbilityTarget(void)const = 0;
    virtual bool            IsActive(void)const = 0;
    virtual void            OnStunned(float) = 0;

private:
    CountdownTimers m_throwDurationTimer;
    CountdownTimers m_pickupAnimationTimer;
    CountdownTimers m_createRockTimer;
    CountdownTimers m_throwRockTimer;
    CHandle<IBaseEntity> m_hRock;
};

class IVomit : public IBaseAbility
{
    DECLARE_CLASS( IVomit, IBaseAbility );
public:
    virtual ~IVomit() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void			Precache( void ) = 0;
    virtual bool            IsAbilityReadyToFire(void)const = 0;
    virtual void            ActivateAbility(void) = 0;
    virtual void            UpdateAbility(void) = 0;
    virtual void            OnCreate(CTerrorPlayer *) = 0;
    virtual bool            HasAbilityTarget(void)const = 0;
    virtual int             GetButton(void)const = 0;
    virtual bool            IsActive(void)const = 0;
    virtual void            Operator_HandleAnimEvent(animevent_t *, IBaseCombatCharacter *) = 0;
    virtual void            OnStunned(float) = 0;

private:
    CNetworkVar(bool, m_isSpraying);                                // 921
    CNetworkVarEmbedded(CountdownTimers,    m_attackDuration);      // 924
    CNetworkVarEmbedded(CountdownTimers,    m_nextSpray);           // 936
};

class ILunge : public IBaseAbility
{
    DECLARE_CLASS( ILunge, IBaseAbility );
public:
    virtual ~ILunge() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
    virtual bool            IsAbilityReadyToFire(void)const = 0;
    virtual void            ActivateAbility(void) = 0;
    virtual void            UpdateAbility(void) = 0;
    virtual void            OnCreate(CTerrorPlayer *) = 0;
    virtual bool            HasAbilityTarget(void)const = 0;
    virtual int             GetButton(void)const = 0;
    virtual bool            IsActive(void)const = 0;
    virtual bool            IsPredicted(void)const = 0;
    virtual Vector          GetJumpVector(bool) = 0;
    virtual void            OnTouch(CBaseEntity *) = 0;
    virtual void            OnCrouched(void) = 0;
    virtual void            OnCrouchStart(void) = 0;
    virtual void            OnOwnerTakeDamage( const CTakeDamageInfo &info ) = 0;
    virtual void            OnStunned(float) = 0;
    virtual int             HandleCustomCollision(CBaseEntity *,Vector const&,Vector const&,CGameTrace *,CMoveData *) = 0;

private:
    CNetworkVarEmbedded(CountdownTimers, m_lungeAgainTimer);        // 924
    CNetworkVar(float, m_lungeStartTime);                           // 936
    CNetworkVector(m_queuedLunge);                                  // 940
    CNetworkVar(bool, m_isLunging);                                 // 952

    CountdownTimers m_timer_956;                                    // 956 ?
};

#endif