#ifndef _HEADER_PROP_INCLUDE_
#define _HEADER_PROP_INCLUDE_

#include "IBasePlayer.h"

enum PhysGunPickup_t
{
	PICKED_UP_BY_CANNON,
	PUNTED_BY_CANNON,
	PICKED_UP_BY_PLAYER, // Picked up by +USE, not physgun.
};

enum PhysGunDrop_t
{
	DROPPED_BY_PLAYER,
	THROWN_BY_PLAYER,
	DROPPED_BY_CANNON,
	LAUNCHED_BY_CANNON,
};

enum PhysGunForce_t
{
	PHYSGUN_FORCE_DROPPED,	// Dropped by +USE
	PHYSGUN_FORCE_THROWN,	// Thrown from +USE
	PHYSGUN_FORCE_PUNTED,	// Punted by cannon
	PHYSGUN_FORCE_LAUNCHED,	// Launched by cannon
};

class IBaseProp : public IBaseAnimating
{
	DECLARE_CLASS( IBaseProp, IBaseAnimating );
public:

	virtual void Spawn( void ) = 0;
	virtual void Precache( void ) = 0;
	virtual void Activate( void ) = 0;
	virtual bool KeyValue( const char *szKeyName, const char *szValue ) = 0;
	// void CalculateBlockLOS( void );
	// int  ParsePropData( void );
	
	virtual void DrawDebugGeometryOverlays( void ) = 0;

	// Don't treat as a live target
	virtual bool IsAlive( void ) = 0;
	virtual bool OverridePropdata() = 0;
};

enum propdata_interactions_t
{
	PROPINTER_PHYSGUN_WORLD_STICK,		// "onworldimpact"	"stick"
	PROPINTER_PHYSGUN_FIRST_BREAK,		// "onfirstimpact"	"break"
	PROPINTER_PHYSGUN_FIRST_PAINT,		// "onfirstimpact"	"paintsplat"
	PROPINTER_PHYSGUN_FIRST_IMPALE,		// "onfirstimpact"	"impale"
	PROPINTER_PHYSGUN_LAUNCH_SPIN_NONE,	// "onlaunch"		"spin_none"
	PROPINTER_PHYSGUN_LAUNCH_SPIN_Z,	// "onlaunch"		"spin_zaxis"
	PROPINTER_PHYSGUN_BREAK_EXPLODE,	// "onbreak"		"explode_fire"
	PROPINTER_PHYSGUN_BREAK_EXPLODE_ICE,	// "onbreak"	"explode_ice"
	PROPINTER_PHYSGUN_DAMAGE_NONE,		// "damage"			"none"

	PROPINTER_FIRE_FLAMMABLE,			// "flammable"			"yes"
	PROPINTER_FIRE_EXPLOSIVE_RESIST,	// "explosive_resist"	"yes"
	PROPINTER_FIRE_IGNITE_HALFHEALTH,	// "ignite"				"halfhealth"

	PROPINTER_PHYSGUN_CREATE_FLARE,		// "onpickup"		"create_flare"

	PROPINTER_PHYSGUN_ALLOW_OVERHEAD,	// "allow_overhead"	"yes"

	PROPINTER_WORLD_BLOODSPLAT,			// "onworldimpact", "bloodsplat"
	
	PROPINTER_PHYSGUN_NOTIFY_CHILDREN,	// "onfirstimpact" cause attached flechettes to explode
	PROPINTER_MELEE_IMMUNE,				// "melee_immune"	"yes"

	// If we get more than 32 of these, we'll need a different system

	PROPINTER_NUM_INTERACTIONS,
};

enum mp_break_t
{
	MULTIPLAYER_BREAK_DEFAULT,
	MULTIPLAYER_BREAK_SERVERSIDE,
	MULTIPLAYER_BREAK_CLIENTSIDE,
	MULTIPLAYER_BREAK_BOTH
};

class IBreakableWithPropData
{
public:
	// Damage modifiers
	virtual void		SetDmgModBullet( float flDmgMod ) = 0;
	virtual void		SetDmgModClub( float flDmgMod ) = 0;
	virtual void		SetDmgModExplosive( float flDmgMod ) = 0;
	virtual float		GetDmgModBullet( void ) = 0;
	virtual float		GetDmgModClub( void ) = 0;
	virtual float		GetDmgModExplosive( void ) = 0;
	virtual float		GetDmgModFire( void ) = 0;

	// Explosive
	virtual void		SetExplosiveRadius( float flRadius ) = 0;
	virtual void		SetExplosiveDamage( float flDamage ) = 0;
	virtual float		GetExplosiveRadius( void ) = 0;
	virtual float		GetExplosiveDamage( void ) = 0;
    virtual void        SetExplosionType(int iType) = 0;
    virtual int         GetExplosionType(void) = 0;
    virtual void        SetExplosionDelay(float flDelay) = 0;
    virtual float       GetExplosionDelay(void) = 0;
    virtual void        SetExplosionBuildupSound(string_t str) = 0;
    virtual string_t    GetExplosionBuildupSound(void) = 0;


	// Physics damage tables
	virtual void		SetPhysicsDamageTable( string_t iszTableName ) = 0;
	virtual string_t	GetPhysicsDamageTable( void ) = 0;

	// Breakable chunks
	virtual void		SetBreakableModel( string_t iszModel ) = 0;
	virtual string_t 	GetBreakableModel( void ) = 0;
	virtual void		SetBreakableSkin( int iSkin ) = 0;
	virtual int			GetBreakableSkin( void ) = 0;
	virtual void		SetBreakableCount( int iCount ) = 0;
	virtual int			GetBreakableCount( void ) = 0;
	virtual void		SetMaxBreakableSize( int iSize ) = 0;
	virtual int			GetMaxBreakableSize( void ) = 0;

	// LOS blocking
	virtual void		SetPropDataBlocksLOS( bool bBlocksLOS ) = 0;
	virtual void		SetPropDataIsAIWalkable( bool bBlocksLOS ) = 0;

	// Interactions
	virtual void		SetInteraction( propdata_interactions_t Interaction ) = 0;
	virtual bool		HasInteraction( propdata_interactions_t Interaction ) = 0;

	// Multiplayer physics mode
	virtual void		SetPhysicsMode(int iMode) = 0;
	virtual int			GetPhysicsMode() = 0;

	// Multiplayer breakable spawn behavior
	virtual void		SetMultiplayerBreakMode( mp_break_t mode ) = 0;
	virtual mp_break_t	GetMultiplayerBreakMode( void ) const = 0;

	// Used for debugging
	virtual void		SetBasePropData( string_t iszBase ) = 0;
	virtual string_t	GetBasePropData( void ) = 0;
};

Vector Pickup_DefaultPhysGunLaunchVelocity( const Vector &vecForward, float flMass );

class IPlayerPickupVPhysics
{
public:
	// Callbacks for the physgun/cannon picking up an entity
	virtual bool			OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) = 0;
	virtual CBaseEntity*    OnFailedPhysGunPickup( Vector vPhysgunPos ) = 0;
	virtual void			OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) = 0;
	virtual void			OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason ) = 0;
	virtual bool			HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer = NULL ) = 0;
	virtual QAngle			PreferredCarryAngles( void )  = 0;
	virtual bool			ForcePhysgunOpen( CBasePlayer *pPlayer ) = 0;
	virtual AngularImpulse	PhysGunLaunchAngularImpulse() = 0;
	virtual bool			ShouldPuntUseLaunchForces( PhysGunForce_t reason ) = 0;
	virtual Vector			PhysGunLaunchVelocity( const Vector &vecForward, float flMass ) = 0;
};

class CDefaultPlayerPickupVPhysics : public IPlayerPickupVPhysics
{
public:
	virtual bool			OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) { return true; }
	virtual CBaseEntity*    OnFailedPhysGunPickup( Vector vPhysgunPos ) { return NULL; }
	virtual void			OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) {}
	virtual void			OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason ) {}
	virtual bool			HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) { return false; }
	virtual QAngle			PreferredCarryAngles( void ) { return vec3_angle; }
	virtual bool			ForcePhysgunOpen( CBasePlayer *pPlayer ) { return false; }
	virtual AngularImpulse	PhysGunLaunchAngularImpulse() { return RandomAngularImpulse( -600, 600 ); }
	virtual bool			ShouldPuntUseLaunchForces( PhysGunForce_t reason ) { return false; }
	virtual Vector			PhysGunLaunchVelocity( const Vector &vecForward, float flMass )
	{
		return Pickup_DefaultPhysGunLaunchVelocity( vecForward, flMass );
	}
};

class CBreakableProp;
enum PerformanceMode_t
{
	PM_NORMAL,
	PM_NO_GIBS,
	PM_FULL_GIBS,
	PM_REDUCED_GIBS,
};

class IBreakableProp : public IBaseProp, public IBreakableWithPropData, public CDefaultPlayerPickupVPhysics
{
public:
	void DisableAutoFade();
	
public:
	DECLARE_CLASS( IBreakableProp, IBaseProp );

    virtual ~IBreakableProp() {}

	virtual ServerClass*                GetServerClass(void) = 0;
	virtual int							YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*                  GetDataDescMap(void) = 0;
	virtual void					    Spawn( void ) = 0;
	virtual void					    Precache( void ) = 0;
	virtual float			            GetAutoAimRadius() = 0;
	virtual int				            OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual void			            Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual void			            UpdateOnRemove( void ) = 0;
	virtual	CBasePlayer		            *HasPhysicsAttacker( float dt ) = 0;
	virtual unsigned int	            PhysicsSolidMaskForEntity( void ) const = 0;
	virtual void 			            Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false ) = 0;
	virtual bool                        OverridePropdata( void ) = 0;
	virtual void                        PlayPuntSound() = 0;
	virtual IPhysicsObject*             GetRootPhysicsObjectForBreak() = 0;
    virtual bool                        HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) = 0;
    virtual QAngle                      PreferredCarryAngles( void ) = 0;

    virtual void		                SetDmgModBullet( float flDmgMod ) = 0;
	virtual void		                SetDmgModClub( float flDmgMod ) = 0;
	virtual void		                SetDmgModExplosive( float flDmgMod ) = 0;
	virtual float		                GetDmgModBullet( void ) = 0;
	virtual float		                GetDmgModClub( void ) = 0;
	virtual float		                GetDmgModExplosive( void ) = 0;
	virtual float		                GetDmgModFire( void ) = 0;
	virtual void		                SetExplosiveRadius( float flRadius ) = 0;
	virtual void		                SetExplosiveDamage( float flDamage ) = 0;
	virtual float		                GetExplosiveRadius( void ) = 0;
	virtual float		                GetExplosiveDamage( void ) = 0;
    virtual void                        SetExplosionType(int iType) = 0;
    virtual int                         GetExplosionType(void) = 0;
    virtual void                        SetExplosionDelay(float flDelay) = 0;
    virtual float                       GetExplosionDelay(void) = 0;
    virtual void                        SetExplosionBuildupSound(string_t str) = 0;
    virtual string_t                    GetExplosionBuildupSound(void) = 0;
	virtual void		                SetPhysicsDamageTable( string_t iszTableName ) = 0;
	virtual string_t	                GetPhysicsDamageTable( void ) = 0;
	virtual void		                SetBreakableModel( string_t iszModel ) = 0;
	virtual string_t 	                GetBreakableModel( void ) = 0;
	virtual void		                SetBreakableSkin( int iSkin ) = 0;
	virtual int			                GetBreakableSkin( void ) = 0;
	virtual void		                SetBreakableCount( int iCount ) = 0;
	virtual int			                GetBreakableCount( void ) = 0;
	virtual void		                SetMaxBreakableSize( int iSize ) = 0;
	virtual int			                GetMaxBreakableSize( void ) = 0;
	virtual void		                SetPropDataBlocksLOS( bool bBlocksLOS ) = 0;
	virtual void		                SetPropDataIsAIWalkable( bool bBlocksLOS ) = 0;
	virtual void		                SetBasePropData( string_t iszBase ) = 0;
	virtual string_t	                GetBasePropData( void ) = 0;
	virtual void		                SetInteraction( propdata_interactions_t Interaction ) = 0;
	virtual bool		                HasInteraction( propdata_interactions_t Interaction ) = 0;
	virtual void		                SetMultiplayerBreakMode( mp_break_t mode ) = 0;
	virtual mp_break_t	                GetMultiplayerBreakMode( void ) const = 0;
	virtual void		                SetPhysicsMode(int iMode) = 0;
	virtual int			                GetPhysicsMode() = 0;
    virtual void                        OnSpawnBreakProp(CBreakableProp*) = 0;
    virtual void	                    OnBreak( const Vector &vecVelocity, const AngularImpulse &angVel, CBaseEntity *pBreaker ) = 0;
	virtual bool                        OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason ) = 0;
	virtual void                        OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason ) = 0;
	virtual void                        OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason ) = 0;
	virtual AngularImpulse              PhysGunLaunchAngularImpulse() = 0;

public:
	COutputEvent	m_OnBreak;
	COutputFloat	m_OnHealthChanged;
	COutputEvent	m_OnTakeDamage;

	float			m_impactEnergyScale;

	int				m_iMinHealthDmg;

	QAngle			m_preferredCarryAngles;
protected:

	int				m_iPhysicsMode;

	unsigned int	m_createTick;
	float			m_flPressureDelay;
	EHANDLE			m_hBreaker;

	PerformanceMode_t m_PerformanceMode;

	// Prop data storage
	float			m_flDmgModBullet;
	float			m_flDmgModClub;
	float			m_flDmgModExplosive;
	float			m_flDmgModFire;
	string_t		m_iszPhysicsDamageTableName;
	string_t		m_iszBreakableModel;
	int				m_iBreakableSkin;
	int				m_iBreakableCount;
	int				m_iMaxBreakableSize;
	string_t		m_iszBasePropData;	
	int				m_iInteractions;
	float			m_explodeDamage;
	float			m_explodeRadius;

	// Count of how many pieces we'll break into, custom or generic
	int				m_iNumBreakableChunks;
private:
	CHandle<IBasePlayer>	m_hPhysicsAttacker;
	float					m_flLastPhysicsInfluenceTime;
	bool					m_bBlockLOSSetByPropData;
	bool					m_bIsWalkableSetByPropData;
	bool					m_bOriginalBlockLOS;	// BlockLOS state before physgun pickup
	char					m_nPhysgunState;		// Ripped-off state
	COutputEvent			m_OnPhysCannonDetach;	// We've ripped it off!
	COutputEvent			m_OnPhysCannonAnimatePreStarted;	// Started playing the pre-pull animation
	COutputEvent			m_OnPhysCannonAnimatePullStarted;	// Player started the pull anim
	COutputEvent			m_OnPhysCannonAnimatePostStarted;	// Started playing the post-pull animation
	COutputEvent			m_OnPhysCannonPullAnimFinished; // We've had our pull anim finished, or the post-pull has finished if there is one
	float					m_flDefaultFadeScale;	// Things may temporarily change the fade scale, but this is its steady-state condition

	mp_break_t m_mpBreakMode;
	char m_Unknows_1484[12];						// виравнювання 12 байт, невідомі зміні

	EHANDLE					m_hLastAttacker;		// Last attacker that harmed me.
	EHANDLE					m_hFlareEnt;
	string_t				m_iszPuntSound;
	CNetworkVar( bool, m_noGhostCollision );
	bool					m_bUsePuntSound;
protected:
	// CNetworkQAngle( m_qPreferredPlayerCarryAngles );
	CNetworkVar( bool, m_bClientPhysics );
};

class IWatcherCallback
{
public:
	virtual ~IWatcherCallback() {}
};

class IPositionWatcher : public IWatcherCallback
{
public:
	virtual void NotifyPositionChanged( CBaseEntity *pEntity ) = 0;
};

class CBoneFollower;
struct physfollower_t
{
	int boneIndex;
	CHandle<CBoneFollower> hFollower;
};


class CBoneFollowerManager
{
private:
	int							m_iNumBones;
	CUtlVector<physfollower_t>	m_physBones;
};


class IDynamicProp : public IBreakableProp, public IPositionWatcher
{
	DECLARE_CLASS( IDynamicProp, IBreakableProp );

public:
    virtual ~IDynamicProp() {}

	virtual ServerClass*                GetServerClass(void) = 0;
	virtual int							YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*                  GetDataDescMap(void) = 0;
    virtual bool			            TestCollision( const Ray_t& ray, unsigned int mask, trace_t& trace ) = 0;
	virtual void 			            Spawn() = 0;
	virtual void			            SetParent( CBaseEntity* pNewParent, int iAttachment = -1 ) = 0;
	virtual void			            OnRestore() = 0;
	virtual void			            UpdateOnRemove( void ) = 0;
	virtual bool			            CreateVPhysics() = 0;
	virtual void 			            HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual bool                        OverridePropdata( void ) = 0;
	virtual IPhysicsObject              *GetRootPhysicsObjectForBreak() = 0;
	virtual void                        NotifyPositionChanged( CBaseEntity *pEntity ) = 0;

public:
	COutputEvent		m_pOutputAnimBegun;
	COutputEvent		m_pOutputAnimOver;

	string_t			m_iszDefaultAnim;

	int					m_iGoalSequence;
	int					m_iTransitionDirection;

	// Random animations
	bool				m_bAnimationDone;
	bool				m_bHoldAnimation;
	bool				m_bRandomAnimator;
	bool				m_bDisableBoneFollowers;
	float				m_flNextRandAnim;
	float				m_flMinRandAnimTime;
	float				m_flMaxRandAnimTime;
	short				m_nPendingSequence;

	bool				m_bStartDisabled;
	bool				m_bAnimateEveryFrame;

	CNetworkVar( bool, m_bUseHitboxesForRenderBox );

protected:
	CBoneFollowerManager	m_BoneFollowerManager;

};

class INavAvoidanceObstacle
{
public:
	virtual bool IsPotentiallyAbleToObstructNavAreas( void ) const = 0;	// could we at some future time obstruct nav?
	virtual float GetNavObstructionHeight( void ) const = 0;			// height at which to obstruct nav areas
	virtual bool CanObstructNavAreas( void ) const = 0;					// can we obstruct nav right this instant?
	virtual CBaseEntity *GetObstructingEntity( void ) = 0;
	virtual void OnNavMeshLoaded( void ) = 0;
};


class IPhysicsProp : public IBreakableProp, public INavAvoidanceObstacle
{
	DECLARE_CLASS( IPhysicsProp, IBreakableProp );
public:
    virtual ~IPhysicsProp() {}

	virtual ServerClass					*GetServerClass(void) = 0;
	virtual int							YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t					*GetDataDescMap(void) = 0;
	virtual void					    Spawn( void ) = 0;
	virtual void					    Precache( void ) = 0;
	virtual int				            ObjectCaps( void ) = 0;
	virtual int				            GetUsePriority(CBaseEntity*) = 0;
	virtual int				            DrawDebugTextOverlays(void) = 0;
	virtual int				            OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual void			            Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual void			            UpdateOnRemove( void ) = 0;
	virtual bool			            CreateVPhysics() = 0;
	virtual void			            VPhysicsUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void			            VPhysicsCollision( int index, gamevcollisionevent_t *pEvent ) = 0;
	virtual bool                        OverridePropdata( void ) = 0;
	virtual void						OnSpawnBreakProp(CBreakableProp *) = 0;
	virtual void                        OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason ) = 0;
	virtual void                        OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason ) = 0;
	virtual bool                        IsPotentiallyAbleToObstructNavAreas( void ) const = 0;
	virtual float                       GetNavObstructionHeight( void ) const = 0;
	virtual bool                        CanObstructNavAreas( void ) const = 0;
	virtual CBaseEntity*                GetObstructingEntity( void ) = 0;
	virtual void                        OnNavMeshLoaded( void ) = 0;

private:
	COutputEvent m_MotionEnabled;
	COutputEvent m_OnAwakened;
	COutputEvent m_OnPhysGunPickup;
	COutputEvent m_OnPhysGunPunt;
	COutputEvent m_OnPhysGunOnlyPickup;
	COutputEvent m_OnPhysGunDrop;
	COutputEvent m_OnPlayerUse;
	COutputEvent m_OnPlayerPickup;
	COutputEvent m_OnOutOfWorld;
	COutputEvent m_UnknownEvent_1732;

	float		m_massScale;
	float		m_inertiaScale;
	int			m_damageType;
	string_t	m_iszOverrideScript;
	int			m_damageToEnableMotion;
	float		m_flForceToEnableMotion;
	int			m_breakableType;
	bool		m_bThrownByPlayer;
	bool		m_bFirstCollisionAfterLaunch;;
	bool		m_bCanObstructNav;
	bool		m_bUnknown_1787;

protected:
	CNetworkVar( bool, m_bAwake );
	CNetworkVar( bool, m_hasTankGlow );
	CNetworkVar( bool, m_isCarryable );

	IntervalTimers m_UnknownInterval;
};

class IPropGlowingObject : public IDynamicProp
{
	DECLARE_CLASS( IPropGlowingObject, IDynamicProp );
public:
	virtual ~IPropGlowingObject() {}

	virtual ServerClass*                GetServerClass(void) = 0;
	virtual int							YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*                  GetDataDescMap(void) = 0;
	virtual int							UpdateTransmitState() = 0;
	virtual void						Spawn( void ) = 0;
	virtual void						Precache( void ) = 0;
	virtual void						Activate( void ) = 0;
	virtual void			            UpdateOnRemove( void ) = 0;


	CNetworkVar( bool, m_bIsGlowing );
	CNetworkVar( int, m_nGlowForTeam );
};

#endif