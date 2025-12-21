#ifndef _HEADER_WEAPON_CS_BASE_INCLUDE_
#define _HEADER_WEAPON_CS_BASE_INCLUDE_
#include "IBaseCombatWeapon.h"
#include "IBaseMultiplayerPlayer.h"

class CCSPlayer;

enum CSWeaponID
{
	ID_WEAPON_NONE = 0,
	ID_WEAPON_PISTO,
	ID_WEAPON_SUB_MACHINEGUN,
	ID_WEAPON_PUMP_SHOTGUN,
	ID_WEAPON_AUTO_SHOTGUN,
	ID_WEAPON_ASSAULT_RIFLE,
	ID_WEAPON_SNIPER_RIFLE,
	ID_WEAPON_FIRST_AID_KIT = 8,
	ID_WEAPON_MOLOTOV,
	ID_WEAPON_PIPE_BOMB,
	ID_WEAPON_PAIN_PILS = 12,
	ID_WEAPON_GAS_CAN = 14,
	ID_WEAPON_PROPAN_TANK,
	ID_WEAPON_OXYGEN_TANK,
	ID_WEAPON_TANK_CLAW,
	ID_WEAPON_HUNTER_CLAW,
	ID_WEAPON_BOOMER_CLAW,
	ID_WEAPON_SMOKE_CLAWN,
};

class IWeaponCSBase : public IBaseCombatWeapon
{
public:
    virtual ~IWeaponCSBase() {};

	virtual ServerClass*	        GetServerClass(void) = 0;
	virtual int				        YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		        GetDataDescMap(void) = 0;
	virtual void			        Spawn( void ) = 0;
	virtual void			        Precache( void ) = 0;
	virtual bool			        KeyValue( const char *szKeyName, const char *szValue ) = 0;
	virtual void			        Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual CBaseEntity*	        Respawn( void ) = 0;
	virtual int				        PhysicsSplash(Vector const&,Vector const&,float,float) = 0;
	virtual bool			        IsPredicted( void ) const = 0;
	virtual void			        Drop( const Vector &vecVelocity ) = 0;
	virtual bool			        CanBeSelected( void ) = 0;
	virtual void 			        DefaultTouch( CBaseEntity *pOther ) = 0;
	virtual bool			        SendWeaponAnim( int iActivity ) = 0;
	virtual bool			        DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt ) = 0;
	virtual bool			        CanDeploy( void ) = 0;
	virtual bool			        Deploy( void ) = 0;
	virtual bool			        Holster( CBaseCombatWeapon *pSwitchingTo = nullptr ) = 0;
	virtual void			        ItemPostFrame( void ) = 0;
	virtual bool			        Reload( void ) = 0;
	virtual void			        SecondaryAttack( void ) = 0;
	virtual float			        GetDefaultAnimSpeed( void ) = 0;
	virtual const Vector&	        GetBulletSpread( void ) = 0;
	virtual void			        OnPickedUp( CBaseCombatCharacter *pNewOwner ) = 0;
	virtual void			        AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles ) = 0;
	virtual float			        CalcViewmodelBob( void ) = 0;
	virtual const char		        *GetViewModel( int viewmodelindex = 0 ) const = 0;
	virtual void			        Materialize( void ) = 0;
	virtual void			        CheckRespawn( void ) = 0;
    virtual void	                BulletWasFired( const Vector &vecStart, const Vector &vecEnd ) = 0;
    virtual bool	                ShouldRemoveOnRoundRestart() = 0;
    virtual bool	                DefaultReload( int iClipSize1, int iClipSize2, int iActivity ) = 0;
    virtual bool                    IsRemoveable() = 0;
    virtual bool                    IsHelpingHandExtended(void) const = 0;

    virtual CCSPlayer               *CanPlayerTouch(CCSPlayer *) = 0;
    virtual int                     ArePlayerTouchesAllowed(void) const = 0;
    virtual bool                    IsAttacking(void) const = 0;
    virtual int                     WeaponTranslateMainActivity(Activity) = 0;
    virtual Activity                GetWeaponFireActivity(PlayerAnimEvent_t, Activity) = 0;
    virtual Activity                GetWeaponReloadActivity(PlayerAnimEvent_t, Activity) = 0;
    virtual Activity                GetWeaponDeployActivity(PlayerAnimEvent_t, Activity) = 0;

	virtual bool	                IsAwp() const = 0;
	
    virtual bool                    CanZoom(void)const = 0;
	virtual int                     HasScope(void)const = 0;
	virtual void                    CycleZoom(void) = 0;
	virtual float                   GetMaxSpeed() const = 0;	// What's the player's max speed while holding this weapon.
	virtual CSWeaponID              GetWeaponID( void ) const = 0;
	virtual bool                    IsSilenced(void)const = 0;
	virtual void                    SetWeaponModelIndex( const char *pName ) = 0;
	virtual void*                   UpdateShieldState(void) = 0;
	virtual bool                    HasSecondaryAttack(void) = 0;
	virtual Activity                GetDeployActivity( void ) = 0;
	virtual bool	                DefaultPistolReload() = 0;
	virtual int                     DeployResetsAttackTime(void) = 0;
	virtual bool                    CanBeDropped(void)const = 0;
};

#endif //_HEADER_WEAPON_CS_BASE_INCLUDE_