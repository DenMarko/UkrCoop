#ifndef _INCLUDE_BASE_COMBAT_WEAPON_H_
#define _INCLUDE_BASE_COMBAT_WEAPON_H_
#include "IBaseAnimating.h"
#include "IBaseCombatCharacter.h"

class CBaseViewModel;
class CHudTexture;
class CTerrorGun;

typedef unsigned short WEAPON_FILE_INFO_HANDLE;

struct WeaponProficiencyInfo_t
{
	float	spreadscale;
	float	bias;
};

typedef enum {
	EMPTY,
	SINGLE,
	SINGLE_NPC,
	WPN_DOUBLE,
	DOUBLE_NPC,
	BURST,
	RELOAD,
	RELOAD_NPC,
	MELEE_MISS,
	MELEE_HIT,
	MELEE_HIT_WORLD,
	SPECIAL1,
	SPECIAL2,
	SPECIAL3,
	NUM_SHOOT_SOUND_TYPES,
} WeaponSound_t;

typedef struct
{
	int			baseAct;
	int			weaponAct;
	bool		required;
} acttable_t;

inline IBaseCombatCharacter *ToBaseCombatCharacter( IBaseEntity *pEntity )
{
	if ( !pEntity )
		return NULL;

	return (IBaseCombatCharacter *)pEntity->MyCombatCharacterPointer();
}

class IBaseCombatWeapon : public IBaseAnimating
{
	DECLARE_CLASS( IBaseCombatWeapon, IBaseAnimating );
public:
	void* GetWpnData( void ) { return g_CallHelper->GetFileWeaponInfoFromHandlet(m_hWeaponFileInfo); }
	const char *GetName( void )
	{
		void *ptr = GetWpnData();
		return access_array_member<char>(ptr, 6);
	}

	IBaseCombatCharacter *GetOwner()
	{
		return ToBaseCombatCharacter((m_hOwner.Get()));
	}

	bool UsesPrimaryAmmo( void );
	bool UsesSecondaryAmmo( void );

public:
	virtual ~IBaseCombatWeapon() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual int				UpdateTransmitState() = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			Activate( void ) = 0;
	virtual int				ObjectCaps( void ) = 0;
private:
	virtual	void			NetworkStateChanged_m_nNextThinkTick(void) = 0;
	virtual void			NetworkStateChanged_m_nNextThinkTick(void *) = 0;
public:
	virtual bool			IsBaseCombatWeapon(void) const = 0;
	virtual CBaseCombatWeapon *MyCombatWeaponPointer(void) = 0;
	virtual void			Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual void			MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType ) = 0;
	virtual CBaseEntity*	Respawn( void ) = 0;
	virtual void 			HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual bool			IsPredicted( void ) const = 0;
	virtual int				GetSubType( void ) = 0;
	virtual void			SetSubType( int iType ) = 0;
	virtual void			Equip( CBaseCombatCharacter *pOwner ) = 0;
	virtual void			Drop( const Vector &vecVelocity ) = 0;
	virtual	int				UpdateClientData( CBasePlayer *pPlayer ) = 0;
	virtual bool			IsAllowedToSwitch( void ) = 0;
	virtual bool			CanBeSelected( void ) = 0;
	virtual bool			VisibleInWeaponSelection( void ) = 0;
	virtual bool			HasAmmo( void ) = 0;
	virtual void			SetPickupTouch( void ) = 0;
	virtual void 			DefaultTouch( CBaseEntity *pOther ) = 0;
	virtual bool			ShouldDisplayAltFireHUDHint() = 0;
	virtual void			DisplayAltFireHudHint() = 0;	
	virtual void			RescindAltFireHudHint() = 0;
	virtual bool			ShouldDisplayReloadHUDHint() = 0;
	virtual void			DisplayReloadHudHint() = 0;
	virtual void			RescindReloadHudHint() = 0;
	virtual void			SetViewModelIndex( int index = 0 ) = 0;
	virtual bool			SendWeaponAnim( int iActivity ) = 0;
	virtual void			SendViewModelAnim( int nSequence ) = 0;
	virtual int				GetTracerAttachmentIndex(void) = 0;
	virtual void			SetViewModel() = 0;

	virtual bool			HasWeaponIdleTimeElapsed( void ) = 0;
	virtual void			SetWeaponIdleTime( float time ) = 0;
	virtual float			GetWeaponIdleTime( void ) = 0;
	virtual bool			HasAnyAmmo( void ) = 0;
	virtual bool			HasPrimaryAmmo( void ) = 0;
	virtual bool			HasSecondaryAmmo( void ) = 0;
	virtual bool			CanHolster( void ) = 0;
	virtual bool			DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt ) = 0;
	virtual bool			CanDeploy( void ) = 0;
	virtual bool			Deploy( void ) = 0;
	virtual bool			Holster( CBaseCombatWeapon *pSwitchingTo = nullptr ) = 0;
	virtual CBaseCombatWeapon *GetLastWeapon( void ) = 0;
	virtual void			SetWeaponVisible( bool visible ) = 0;
	virtual bool			IsWeaponVisible( void ) = 0;
	virtual bool			ReloadOrSwitchWeapons( void ) = 0;
	virtual void			ItemPreFrame( void ) = 0;
	virtual void			ItemPostFrame( void ) = 0;
	virtual void			ItemBusyFrame( void ) = 0;
	virtual void			ItemHolsterFrame( void ) = 0;
	virtual void			WeaponIdle( void ) = 0;
	virtual void			HandleFireOnEmpty() = 0;
	virtual bool			IsWeaponZoomed() = 0;
	virtual	void			CheckReload( void ) = 0;
	virtual void			FinishReload( void ) = 0;
	virtual void			AbortReload( void ) = 0;
	virtual bool			Reload( void ) = 0;
	virtual void			PrimaryAttack( void ) = 0;
	virtual void			SecondaryAttack( void ) = 0;
	virtual Vector&			ShootPosition(void) = 0;
	virtual Activity		GetPrimaryAttackActivity( void ) = 0;
	virtual Activity		GetSecondaryAttackActivity( void ) = 0;
	virtual Activity		GetDrawActivity( void ) = 0;
	virtual float			GetDefaultAnimSpeed( void ) = 0;
	virtual int				GetBulletType( void ) = 0;
	virtual const Vector&	GetBulletSpread( void ) = 0;
	virtual Vector			GetBulletSpread( WeaponProficiency_t proficiency ) = 0;
	virtual float			GetSpreadBias( WeaponProficiency_t proficiency ) = 0;
	virtual float			GetFireRate( void ) = 0;
	virtual int				GetMinBurst() = 0;
	virtual int				GetMaxBurst() = 0;
	virtual float			GetMinRestTime() = 0;
	virtual float			GetMaxRestTime() = 0;
	virtual int				GetRandomBurst() = 0;
	virtual void			WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f ) = 0;
	virtual void			StopWeaponSound( WeaponSound_t sound_type ) = 0;
	virtual const WeaponProficiencyInfo_t *GetProficiencyValues() = 0;
	virtual float			GetMaxAutoAimDeflection() = 0;
	virtual float			WeaponAutoAimScale() = 0;
	virtual bool			StartSprinting( void ) = 0;
	virtual bool			StopSprinting( void ) = 0;
	virtual float			GetDamage( float flDistance, int iLocation ) = 0;
	virtual void			SetActivity( Activity act, float duration ) = 0;
	virtual void			AddViewKick( void ) = 0;
	virtual char			*GetDeathNoticeName( void ) = 0;
	virtual void			OnPickedUp( CBaseCombatCharacter *pNewOwner ) = 0;
	virtual void			AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles ) = 0;
	virtual float			CalcViewmodelBob( void ) = 0;
	virtual void 			GetControlPanelInfo( int nPanelIndex, const char *&pPanelName ) = 0;
	virtual void			GetControlPanelClassName( int nPanelIndex, const char *&pPanelName ) = 0;
	virtual bool			ShouldShowControlPanels( void ) = 0;
	virtual bool			CanBePickedUpByNPCs( void ) = 0;
	virtual CTerrorGun		*GetTerrorGun(void) = 0;
	virtual const char		*GetViewModel( int viewmodelindex = 0 ) const = 0;
	virtual const char		*GetWorldModel( void ) const = 0;
	virtual const char		*GetAnimPrefix( void ) const = 0;
	virtual int				GetMaxClip1( void ) const = 0;
	virtual int				GetMaxClip2( void ) const = 0;
	virtual int				GetDefaultClip1( void ) const = 0;
	virtual int				GetDefaultClip2( void ) const = 0;
	virtual int				GetWeight( void ) const = 0;
	virtual bool			AllowsAutoSwitchTo( void ) const = 0;
	virtual bool			AllowsAutoSwitchFrom( void ) const = 0;
	virtual int				GetWeaponFlags( void ) const = 0;
	virtual int				GetSlot( void ) const = 0;
	virtual int				GetPosition( void ) const = 0;
	virtual char const		*GetName( void ) const = 0;
	virtual char const		*GetPrintName( void ) const = 0;
	virtual char const		*GetShootSound( int iIndex ) const = 0;
	virtual int				GetRumbleEffect() const = 0;
	virtual bool			UsesClipsForAmmo1( void ) const = 0;
	virtual bool			UsesClipsForAmmo2( void ) const = 0;
	virtual int				IsGrenade(void)const = 0;
	virtual const unsigned char *GetEncryptionKey( void ) = 0;
	virtual int				GetPrimaryAmmoType( void )  const = 0;
	virtual int				GetSecondaryAmmoType( void )  const = 0;

	virtual CHudTexture const	*GetSpriteActive( void ) const = 0;
	virtual CHudTexture const	*GetSpriteInactive( void ) const = 0;
	virtual CHudTexture const	*GetSpriteAmmo( void ) const = 0;
	virtual CHudTexture const	*GetSpriteAmmo2( void ) const = 0;
	virtual CHudTexture const	*GetSpriteCrosshair( void ) const = 0;
	virtual CHudTexture const	*GetSpriteAutoaim( void ) const = 0;
	virtual CHudTexture const	*GetSpriteZoomedCrosshair( void ) const = 0;
	virtual CHudTexture const	*GetSpriteZoomedAutoaim( void ) const = 0;
	virtual CHudTexture const	*GetSpriteDualActive(void)const = 0;
	virtual CHudTexture const	*GetSpriteDualInactive(void)const = 0;
	virtual Activity		ActivityOverride( Activity baseAct, bool *pRequired ) = 0;
	virtual	acttable_t*		ActivityList( void ) = 0;
	virtual	int				ActivityListCount( void ) = 0;
	virtual	long double 	GetReloadDurationModifier(void) = 0;
	virtual	long double 	GetDeployDurationModifier(void) = 0;
	virtual void			FallInit( void ) = 0;
	virtual void			FallThink( void ) = 0;
	virtual void			Materialize( void ) = 0;
	virtual void			CheckRespawn( void ) = 0;
	virtual void			Delete( void ) = 0;
	virtual void			Kill( void ) = 0;
	virtual int				CapabilitiesGet( void ) = 0;
	virtual bool			WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions ) = 0;
	virtual	int				WeaponRangeAttack1Condition( float flDot, float flDist ) = 0;
	virtual	int				WeaponRangeAttack2Condition( float flDot, float flDist ) = 0;
	virtual	int				WeaponMeleeAttack1Condition( float flDot, float flDist ) = 0;
	virtual	int				WeaponMeleeAttack2Condition( float flDot, float flDist ) = 0;

	virtual void			Operator_FrameUpdate( CBaseCombatCharacter  *pOperator ) = 0;
	virtual void			Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ) = 0;
	virtual void			Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary ) = 0;
	virtual int				GetDroppingPlayer(void)const = 0;
	virtual bool			CanLower( void ) = 0;
	virtual bool			Ready( void ) = 0;
	virtual bool			Lower( void ) = 0;
	virtual void			HideThink( void ) = 0;

private:
	typedef CHandle< IBaseCombatCharacter > IBaseCombatCharacterHandle;
	CNetworkVar( IBaseCombatCharacterHandle, m_hOwner );				// Player carrying this weapon
public:
	// Networked fields
	CNetworkVar( int, m_nViewModelIndex );
	// Weapon firing
	CNetworkVar( float, m_flNextPrimaryAttack );						// soonest time ItemPostFrame will call PrimaryAttack
	CNetworkVar( float, m_flNextSecondaryAttack );						// soonest time ItemPostFrame will call SecondaryAttack

	CNetworkVar( int, m_nQueuedAttack );
	CNetworkVar( float, m_flTimeAttackQueued );
	CNetworkVar( int, m_iViewModelIndex );
	CNetworkVar( int, m_iWorldModelIndex );
	// Weapon data
	CNetworkVar( int, m_iState );				// See WEAPON_* definition
	CNetworkVar( int, m_iPrimaryAmmoType );		// "primary" ammo index into the ammo info array 
	CNetworkVar( int, m_iSecondaryAmmoType );	// "secondary" ammo index into the ammo info array
	CNetworkVar( int, m_iClip1 );				// number of shots left in the primary weapon clip, -1 it not used
	CNetworkVar( int, m_iClip2 );				// number of shots left in the secondary weapon clip, -1 it not used
	CNetworkVar( bool, m_bOnlyPump );
	CNetworkVar( float, m_flTimeWeaponIdle );

	float					m_flNextEmptySoundTime;				// delay on empty sound playing
	float					m_fMinRange1;			// What's the closest this weapon can be used?
	float					m_fMinRange2;			// What's the closest this weapon can be used?
	float					m_fMaxRange1;			// What's the furthest this weapon can be used?
	float					m_fMaxRange2;			// What's the furthest this weapon can be used?
	float					m_fFireDuration;		// The amount of time that the weapon has sustained firing
	
private:
	Activity				m_Activity;
	int						m_iPrimaryAmmoCount;
	int						m_iSecondaryAmmoCount;

public:
	string_t				m_iszName;				// Classname of this weapon.

private:
	bool					m_bRemoveable;
public:
	// Weapon state
	CNetworkVar( bool, m_bInReload );									// Are we in the middle of a reload;
	bool					m_bFireOnEmpty;												// True when the gun is empty and the player is still holding down the attack key(s)
	bool					m_bFiresUnderwater;		// true if this weapon can fire underwater
	bool					m_bAltFiresUnderwater;		// true if this weapon can fire underwater
	bool					m_bReloadsSingly;		// Tryue if this weapon reloads 1 round at a time

public:
	Activity				GetIdealActivity( void ) { return m_IdealActivity; }
	int						GetIdealSequence( void ) { return m_nIdealSequence; }
	
private:
	int						m_nIdealSequence;
	Activity				m_IdealActivity;

public:
	int						WeaponState() const { return m_iState; }

	int						m_iSubType;
	float					m_flUnlockTime;
	EHANDLE					m_hLocker;				// Who locked this weapon.

private:
	WEAPON_FILE_INFO_HANDLE	m_hWeaponFileInfo;
	IPhysicsConstraint		*m_pConstraint;
	int						m_iAltFireHudHintCount;		// How many times has this weapon displayed its alt-fire HUD hint?
	int						m_iReloadHudHintCount;		// How many times has this weapon displayed its reload HUD hint?
	bool					m_bAltFireHudHintDisplayed;	// Have we displayed an alt-fire HUD hint since this weapon was deployed?
	bool					m_bReloadHudHintDisplayed;	// Have we displayed a reload HUD hint since this weapon was deployed?
	float					m_flHudHintPollTime;	// When to poll the weapon again for whether it should display a hud hint.
	float					m_flHudHintMinDisplayTime; // if the hint is squelched before this, reset my counter so we'll display it again.

protected:
	COutputEvent			m_OnPlayerUse;			// Fired when the player uses the weapon.
	COutputEvent			m_OnPlayerPickup;		// Fired when the player picks up the weapon.
	COutputEvent			m_OnNPCPickup;			// Fired when an NPC picks up the weapon.
	COutputEvent			m_OnCacheInteraction;	// For awarding lambda cache achievements in HL2 on 360. See .FGD file for details 

};

#endif