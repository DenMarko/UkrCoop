#ifndef _HEADER_TERROR_WEAPON_INCLUDE_
#define _HEADER_TERROR_WEAPON_INCLUDE_
#include "IWeaponCSBase.h"

enum HelpingHandState
{
    HELPING_HAND_DISABLED = 0,
    HELPING_HAND_UNKNOWN_1 = 1,
    HELPING_HAND_READY = 2,
    HELPING_HAND_ACTIVE = 3,
    HELPING_HAND_PULLING = 4,
    HELPING_HAND_COMPLETE = 5
};

class ITerrorWeapon : public IWeaponCSBase
{
public:
    bool IsWeapon(int iWeapon)
    {
        return (GetWeaponID() == iWeapon);
    }

    inline IBaseCombatCharacter *GetPlayerOwner()
    {
        auto owner = GetOwner();
        if(owner && owner->IsPlayer())
            return owner;

        return nullptr;
    }

    void SetHelpingHandState(HelpingHandState state);
    void SuppressHelpingHands(float flValue);

public:
    virtual ~ITerrorWeapon() {};

	virtual ServerClass*	        GetServerClass(void) = 0;
	virtual int				        YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		        GetDataDescMap(void) = 0;
	virtual void			        Spawn( void ) = 0;
	virtual void			        Precache( void ) = 0;
	virtual void			        Drop( const Vector &vecVelocity ) = 0;
	virtual void 			        DefaultTouch( CBaseEntity *pOther ) = 0;
	virtual bool			        CanDeploy( void ) = 0;
	virtual bool			        Deploy( void ) = 0;
	virtual bool			        Holster( CBaseCombatWeapon *pSwitchingTo = nullptr ) = 0;
	virtual void			        ItemPostFrame( void ) = 0;
	virtual void			        ItemBusyFrame( void ) = 0;
	virtual void			        SecondaryAttack( void ) = 0;
	virtual const char		        *GetViewModel( int viewmodelindex = 0 ) const = 0;
	virtual	long double 	        GetReloadDurationModifier(void) = 0;
	virtual	long double 	        GetDeployDurationModifier(void) = 0;
	virtual void			        Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) = 0;
	virtual int				        GetDroppingPlayer(void)const = 0;
    virtual bool                    IsHelpingHandExtended(void) const = 0;

    virtual CCSPlayer               *CanPlayerTouch(CCSPlayer *) = 0;
    virtual bool                    IsAttacking(void) const = 0;
    virtual bool                    IsDualWielding(void)const = 0;
    virtual bool                    CanPlayerMove(void)const = 0;
    virtual bool                    HasSecondaryMelee(void) = 0;
    virtual bool                    CanSecondaryMeleeInterruptReload(void) = 0;
    virtual bool                    CanFidget(void) = 0;
    virtual bool                    IsDroppedWhenHolstered(void)const = 0;
    virtual bool                    WantsCSItemPostFrame(void)const = 0;
    virtual void                    OnStunned(float duration) = 0;
    virtual void                    OnPouncedUpon(void) = 0;
    virtual void                    OnOwnerTakeDamage(const CTakeDamageInfo &info) = 0;
    virtual Activity                TranslateViewmodelActivity(Activity)const = 0;
    virtual Activity                GetViewmodelMeleeActivity(void) = 0;
    virtual Activity                GetLayerForViewmodelActivity(Activity)const = 0;
    virtual int                     CanExtendHelpingHand(void)const = 0;
    virtual void                    *GetSwingForward(void) = 0;
    virtual int                     TrySwing(float,float,float) = 0;
    virtual int                     OnHit(CGameTrace &,Vector const&,bool) = 0;
    virtual void                    OnSwingStart(void) = 0;
    virtual void                    OnSwingEnd(bool) = 0;
    virtual int                     DoSwing(void) = 0;
    virtual float                   SwingYawStart(void)const = 0;
    virtual float                   SwingYawEnd(void)const = 0;
    virtual void                    CheckQueuedReload(void) = 0;
    virtual CBaseCombatCharacter    *CheckCancelledReload(void) = 0;
};

class ITerrorGun : public ITerrorWeapon
{
public:
    virtual ~ITerrorGun() {}

	virtual ServerClass*	        GetServerClass(void) = 0;
	virtual int				        YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		        GetDataDescMap(void) = 0;
	virtual void			        Spawn( void ) = 0;
	virtual void			        Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual bool			        SendWeaponAnim( int iActivity ) = 0;
	virtual int				        GetTracerAttachmentIndex(void) = 0;
	virtual bool			        Deploy( void ) = 0;
	virtual bool			        Holster( CBaseCombatWeapon *pSwitchingTo = nullptr ) = 0;
	virtual void			        ItemPostFrame( void ) = 0;
	virtual void			        ItemBusyFrame( void ) = 0;
	virtual void			        FinishReload( void ) = 0;
	virtual void			        AbortReload( void ) = 0;
	virtual bool			        Reload( void ) = 0;
	virtual void			        PrimaryAttack( void ) = 0;
	virtual float			        GetFireRate( void ) = 0;
	virtual float			        GetMaxAutoAimDeflection() = 0;
	virtual float			        WeaponAutoAimScale() = 0;
	virtual CTerrorGun		        *GetTerrorGun(void) = 0;
	virtual int				        GetMaxClip1( void ) const = 0;
	virtual void			        Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) = 0;
    virtual bool                    CanZoom(void)const = 0;
	virtual int                     HasScope(void)const = 0;
	virtual void                    CycleZoom(void) = 0;
    virtual bool                    IsDualWielding(void)const = 0;
    virtual bool                    HasSecondaryMelee(void) = 0;
    virtual bool                    WantsCSItemPostFrame(void)const = 0;
    virtual void                    OnSwingStart(void) = 0;

    virtual bool                    IsFullyAutomatic(void)const = 0;
    virtual bool                    IsSingleReload(void)const = 0;
    virtual float                   GetReloadEndDuration(bool)const = 0;
    virtual int                     CanBeDualWielded(void)const = 0;
    virtual int                     ViewMovementAffectsAccuracy(void)const = 0;
    virtual float                   GetRateOfFire(void)const = 0;
    virtual int                     EquipSecondWeapon(void) = 0;
    virtual int                     RemoveSecondWeapon(void) = 0;
    virtual int                     DropSecondWeapon(void) = 0;
    virtual int                     GetViewmodelTracerAttachmentIndex(void) = 0;
    virtual void                    UpdateDualWield(void) = 0;
    virtual float                   GetMaxAutoAimRange(void) = 0;
    virtual float                   GetMinInAirSpread(void)const = 0;
    virtual float                   GetMaxMovementSpread(void)const = 0;
    virtual float                   GetMinStandingSpread(void)const = 0;
    virtual float                   GetMinDuckingSpread(void)const = 0;


};

#endif // _HEADER_TERROR_WEAPON_INCLUDE_