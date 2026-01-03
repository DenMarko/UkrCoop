#ifndef _INCLUDE_BASE_PLAYER_H_
#define _INCLUDE_BASE_PLAYER_H_
#include "IBaseCombatCharacter.h"

class CFogController;

class CPlayerCmdInfo
{
public:
	CPlayerCmdInfo() : 
	  m_flTime( 0.0f ), m_nNumCmds( 0 ), m_nDroppedPackets( 0 )
	{
	}

	float		m_flTime;
	int			m_nNumCmds;
	int			m_nDroppedPackets;
};

class CPlayerSimInfo
{
public:
	CPlayerSimInfo() : 
	  m_flTime( 0.0f ), m_nNumCmds( 0 ), m_nTicksCorrected( 0 ), m_flFinalSimulationTime( 0.0f ), m_flGameSimulationTime( 0.0f ), m_flServerFrameTime( 0.0f ), m_vecAbsOrigin( 0, 0, 0 )
	{
	}

	float		m_flTime;
	int			m_nNumCmds;
	int			m_nTicksCorrected; // +ve or -ve
	float		m_flFinalSimulationTime;
	float		m_flGameSimulationTime;
	float		m_flServerFrameTime;  
	Vector		m_vecAbsOrigin;
};


class CUserCmd
{
public:
	CUserCmd()
	{
		Reset();
	}

	virtual ~CUserCmd() { };


	CUserCmd& operator =( const CUserCmd& src )
	{
		if ( this == &src )
			return *this;

		command_number		= src.command_number;
		tick_count			= src.tick_count;
		viewangles			= src.viewangles;
		forwardmove			= src.forwardmove;
		sidemove			= src.sidemove;
		upmove				= src.upmove;
		buttons				= src.buttons;
		impulse				= src.impulse;
		weaponselect		= src.weaponselect;
		weaponsubtype		= src.weaponsubtype;
		random_seed			= src.random_seed;
		mousedx				= src.mousedx;
		mousedy				= src.mousedy;
		hasbeenpredicted	= src.hasbeenpredicted;
		headangles			= src.headangles;
		headoffset			= src.headoffset;

		return *this;
	}

	void Reset()
	{
		command_number = 0;
		tick_count = 0;
		viewangles.Init();
		forwardmove = 0.0f;
		sidemove = 0.0f;
		upmove = 0.0f;
		buttons = 0;
		impulse = 0;
		weaponselect = 0;
		weaponsubtype = 0;
		random_seed = 0;
		mousedx = 0;
		mousedy = 0;
		hasbeenpredicted = false;
		headangles.Init();
		headoffset.Init();
	}

	CUserCmd( const CUserCmd& src )
	{
		*this = src;
	}

	int		command_number;
	int		tick_count;
	QAngle	viewangles;
	float	forwardmove;   
	float	sidemove;      
	float	upmove;         
	int		buttons;		
	byte    impulse;        
	int		weaponselect;	
	int		weaponsubtype;
	int		random_seed;
	short	mousedx;
	short	mousedy;
	bool	hasbeenpredicted;
	QAngle 	headangles;
	Vector 	headoffset;
};


class CCommandContext
{
public:
	CUtlVector< CUserCmd > cmds;

	int				numcmds;
	int				totalcmds;
	int				dropped_packets;
	bool			paused;
};

struct fogparams_t
{
	DECLARE_CLASS_NOBASE( fogparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();

	bool operator !=( const fogparams_t& other ) const;

	CNetworkVector(dirPrimary);				//	4
	color32 colorPrimary;			//	16
	color32 colorSecondary;			//	20
	color32 colorPrimaryLerpTo;		//	24
	color32 colorSecondaryLerpTo;	//	28
	CNetworkVar(float, start);					//	32
	CNetworkVar(float, end);						//	36
	CNetworkVar(float, farz);						//	40
	CNetworkVar(float, maxdensity);				//	44
	CNetworkVar(float, startLerpTo);				//	48
	CNetworkVar(float, endLerpTo);				//	52
	CNetworkVar(float, maxdensityLerpTo);			//	56
	CNetworkVar(float, lerptime);					//	60
	CNetworkVar(float, duration);					//	64
	CNetworkVar(bool, enable);					//	68
	CNetworkVar(bool, blend);						//	69
	// float ZoomFogScale;
	CNetworkVar(float, HDRColorScale);			// 	72
};

struct sky3dparams_t
{
	DECLARE_CLASS_NOBASE( sky3dparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkVar(int, scale);		// 4
	CNetworkVector(origin);			// 8
	CNetworkVar(int, area);			// 20

	CNetworkVarEmbedded(fogparams_t, fog);				// 24
};

struct audioparams_t
{
	DECLARE_CLASS_NOBASE( audioparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkArray( Vector, localSound, 8 )			// 4
	CNetworkVar(int, soundscapeIndex);			// 100
	CNetworkVar(int, localBits);					// 104
	CNetworkVar(int, entIndex);					// 108 
};

struct fogplayerparams_t
{
	virtual void NetworkStateChanged() {}
	virtual void NetworkStateChanged( void *pProp ) {}

	CHandle< CFogController > 	m_hCtrl;				// 4
	float						m_flTransitionTime;		// 8
	color32						m_OldColor;				// 12
	float						m_flOldStart;			// 16
	float						m_flOldEnd;				// 20
	float						m_flOldMaxDensity;		// 24
	float						m_flOldHDRColorScale;	// 28
	float						m_flOldFarZ;			// 32

	color32						m_NewColor;				// 36
	float						m_flNewStart;			// 40
	float						m_flNewEnd;				// 44
	float						m_flNewMaxDensity;		// 48
	float						m_flNewHDRColorScale;	// 52
	float						m_flNewFarZ;			// 56

	fogplayerparams_t()
	{
		m_hCtrl.Set( NULL );
		m_flTransitionTime = -1.0f;
		m_OldColor.r = 0.f;
		m_OldColor.g = 0.f;
		m_OldColor.g = 0.f;
		m_OldColor.a = 0.0f;
		m_flOldStart = 0.0f;
		m_flOldEnd = 0.0f;
		m_flOldMaxDensity = 1.0f;
		m_flOldHDRColorScale = 1.0f;
		m_flOldFarZ = 0;
		m_NewColor.r = 0.f;
		m_NewColor.g = 0.f;
		m_NewColor.g = 0.f;
		m_NewColor.a = 0.0f;
		m_flNewStart = 0.0f;
		m_flNewEnd = 0.0f;
		m_flNewMaxDensity = 1.0f;
		m_flNewHDRColorScale = 1.0f;
		m_flNewFarZ = 0;
	}
};

class CPlayerLocalData
{
public:
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerLocalData();

	void UpdateAreaBits( class IBasePlayer *pl, unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES] );


public:

	CNetworkArray(unsigned char, m_chAreaBits, MAX_AREA_STATE_BYTES);					// 	4	Which areas are potentially visible to the client?
	CNetworkArray(unsigned char, m_chAreaPortalBits, MAX_AREA_PORTAL_STATE_BYTES);		// 	36	Which area portals are open?

	CNetworkVar(int, m_iHideHUD);														// 	60	bitfields containing sections of the HUD to hide
	CNetworkVar(float, m_flFOVRate);													// 	64	rate at which the FOV changes (defaults to 0)
		
	Vector				m_vecOverViewpoint;												// 	68	Viewpoint overriding the real player's viewpoint
	
	CNetworkVar(bool, m_bDucked);														//  80
	CNetworkVar(bool, m_bDucking);														//  81
	CNetworkVar(bool, m_bInDuckJump);													//  82
	CNetworkVar(float, m_flDucktime);													//  84
	CNetworkVar(float, m_flDuckJumpTime);												//  88
	CNetworkVar(float, m_flJumpTime);													//  92
	int 				m_nStepside;													//  96
	CNetworkVar(float, m_flFallVelocity);												//  100
	int 				m_nOldButtons;													//  104
	class CSkyCamera*	m_pOldSkyCamera;												//  108
	CNetworkQAngle(m_vecPunchAngle);													//  112
	CNetworkQAngle(m_vecPunchAngleVel);													//  124
	CNetworkVar(bool, m_bDrawViewmodel);												//  136
	CNetworkVar(bool, m_bWearingSuit);													//  137
	CNetworkVar(bool, m_bPoisoned);														//  138
	CNetworkVar(float, m_flStepSize);													//  140
	CNetworkVar(bool, m_bAllowAutoMovement);											//  144
	CNetworkVar(bool, m_bAutoAimTarget);												// 	145
	CNetworkVarEmbedded(sky3dparams_t, m_skybox3d);										//  148
	CNetworkVarEmbedded(fogparams_t, m_fog);											// 	248
	CNetworkVarEmbedded(audioparams_t, m_audio);										// 	324
	CNetworkVar(bool, m_bSlowMovement);													// 	436
};

class CPlayerState
{
public:
	DECLARE_CLASS_NOBASE( CPlayerState );
	virtual void NetworkStateChanged() { }
	virtual void NetworkStateChanged( void *pProp ) { }	
	virtual ~CPlayerState() {}

	CNetworkVar(bool, 	deadflag);		// 4
	QAngle				v_angle;		// 8
	string_t			netname;		// 20
	int					fixangle;		// 24
	QAngle				anglechange;	// 28
	bool				hltv;			// 40
	int					frags;			// 44
	int					deaths;			// 48
};

class CPlayerInfo : public IBotController, public IPlayerInfo
{
public:
	CPlayerInfo () { m_pParent = NULL; } 
	~CPlayerInfo () {}
	void SetParent( class IBasePlayer *parent ) { m_pParent = parent; } 

	// IPlayerInfo interface
	virtual const char *GetName();
	virtual int			GetUserID();
	virtual const char *GetNetworkIDString();
	virtual int			GetTeamIndex();
	virtual void		ChangeTeam( int iTeamNum );
	virtual int			GetFragCount();
	virtual int			GetDeathCount();
	virtual bool		IsConnected();
	virtual int			GetArmorValue();

	virtual bool IsHLTV();
	virtual bool IsPlayer();
	virtual bool IsFakeClient();
	virtual bool IsDead();
	virtual bool IsInAVehicle();
	virtual bool IsObserver();
	virtual const Vector GetAbsOrigin();
	virtual const QAngle GetAbsAngles();
	virtual const Vector GetPlayerMins();
	virtual const Vector GetPlayerMaxs();
	virtual const char *GetWeaponName();
	virtual const char *GetModelName();
	virtual const int GetHealth();
	virtual const int GetMaxHealth();

	// bot specific functions	
	virtual void SetAbsOrigin( Vector & vec );
	virtual void SetAbsAngles( QAngle & ang );
	virtual void RemoveAllItems( bool removeSuit );
	virtual void SetActiveWeapon( const char *WeaponName );
	virtual void SetLocalOrigin( const Vector& origin );
	virtual const Vector GetLocalOrigin( void );
	virtual void SetLocalAngles( const QAngle& angles );
	virtual const QAngle GetLocalAngles( void );
	virtual void PostClientMessagesSent( void );
	virtual bool IsEFlagSet( int nEFlagMask );

	virtual void RunPlayerMove( CBotCmd *ucmd );
	virtual void SetLastUserCommand( const CBotCmd &cmd );

	virtual CBotCmd GetLastUserCommand();

private:
	class IBasePlayer *m_pParent; 
};

enum stepsoundtimes_t
{
	STEPSOUNDTIME_NORMAL = 0,
	STEPSOUNDTIME_ON_LADDER,
	STEPSOUNDTIME_WATER_KNEE,
	STEPSOUNDTIME_WATER_FOOT,
};

enum PlayerConnectedState
{
	PlayerConnected,
	PlayerDisconnecting,
	PlayerDisconnected,
};

class CUserCmd;

typedef struct 
{
	Vector		m_vecAutoAimDir;
	Vector		m_vecAutoAimPoint;
	EHANDLE		m_hAutoAimEntity;
	bool		m_bAutoAimAssisting;
	bool		m_bOnTargetNatural;		
	float		m_fScale;
	float		m_fMaxDist;
} autoaim_params_t;

class CHintSystem;

class IBasePlayer : public IBaseCombatCharacter
{
	DECLARE_CLASS( IBasePlayer, IBaseCombatCharacter );
public:
	bool IsConnected() const
	{
		return m_iConnected != PlayerDisconnected;
	}

	bool IsDisconnecting() const
	{
		return m_iConnected == PlayerDisconnecting;
	}

	bool IsOnLadder( void )
	{ 
		return (GetMoveType() == MOVETYPE_LADDER);
	}

	void SetArmorValue( int value )
	{
		m_ArmorValue = value;
	}

	void EyeVectors( Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL );
	void CacheVehicleView( void );
	int GetUserID( void ) const { return engine->GetPlayerUserId( edict() ); }
	const char *			GetPlayerName() { return m_szNetname; }
	const char *			GetNetworkIDString();
	int						FragCount() const		{ return m_iFrags; }
	int						DeathCount() const		{ return m_iDeaths;}
	int						ArmorValue() const		{ return m_ArmorValue; }
	bool					IsHLTV( void ) const 	{ return pl.hltv; }
	bool 					IsDead() const;
	bool					IsObserver() const		{ return (m_afPhysicsFlags & ( 1<<3 )) != 0; }
	void					SetTimeBase( float flTimeBase ) { m_nTickBase = ( (int)( 0.5f + (float)(flTimeBase) / (g_pGlobals->interval_per_tick) ) ); }
	void 					SetLastUserCommand( const CUserCmd &cmd ) { m_LastCmd = cmd; }
	CUserCmd const 			*GetLastUserCommand( void ) { return &m_LastCmd; }
	const bool 				IsDucked( void ) const { return m_Local.m_bDucked; }
	void 					SetDucked(bool bDucked) { m_Local.m_bDucked = bDucked; }
	void					SetDucking(bool bDucking) { m_Local.m_bDucking = bDucking; }
	void					SetDucktime(float flTime) { m_Local.m_flDucktime = flTime; }
	float					GetFallVelocity() { return m_Local.m_flFallVelocity; }
	void					SetFallVelocity( float velocity ) { m_Local.m_flFallVelocity = velocity; }
	const IBaseEntity*		GetUseEntity() const { return m_hUseEntity; }
	bool					ClearUseEntity( void );
	void					SnapEyeAngles( const QAngle &viewAngles );


	IPlayerInfo *GetPlayerInfo() { return &m_PlayerInfo; }
	IBotController *GetBotController() { return &m_PlayerInfo; }
public:
	virtual ~IBasePlayer() {  }

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo ) = 0;
	virtual int				UpdateTransmitState() = 0;
	virtual const char*		GetTracerType( void ) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			SetModel( const char *szModelName ) = 0;
	virtual void			Activate( void ) = 0;
	virtual int				ObjectCaps( void ) = 0;
	virtual	void			DrawDebugGeometryOverlays(void) = 0;					
	virtual int				Save( ISave &save ) = 0;
	virtual int				Restore( IRestore &restore ) = 0;
	virtual bool			ShouldSavePhysics() = 0;
	virtual void			OnRestore() = 0;
	virtual int				RequiredEdictIndex( void ) = 0;
private:
	virtual	void			NetworkStateChanged_m_nNextThinkTick(void) = 0;
	virtual void			NetworkStateChanged_m_nNextThinkTick(void *) = 0;
public:
	virtual Class_T			Classify ( void ) = 0;
protected:
	virtual void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr ) = 0;
public:
	virtual int				OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual int				TakeHealth( float flHealth, int bitsDamageType ) = 0;
	virtual void			Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual void			Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual	bool			IsPlayer( void ) const = 0;
	virtual bool			IsNetClient( void ) const = 0;
	virtual void			ChangeTeam( int iTeamNum ) = 0;
	virtual void			PhysicsSimulate( void ) = 0;
	virtual void			UpdateOnRemove( void ) = 0;
	virtual void			MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType ) = 0;
	virtual void 			DoImpactEffect( trace_t &tr, int nDamageType ) = 0;
	virtual void			NetworkStateChanged_m_iHealth(void) = 0;
	virtual void			NetworkStateChanged_m_iHealth(void *) = 0;
	virtual void			NetworkStateChanged_m_lifeState(void) = 0;
	virtual void			NetworkStateChanged_m_lifeState(void *) = 0;
	virtual Vector			EyePosition( void ) = 0;
	virtual const QAngle 	&EyeAngles( void ) = 0;
	virtual const QAngle 	&LocalEyeAngles( void ) = 0;
	virtual Vector			BodyTarget( const Vector &posSrc, bool bNoisy = true) = 0;
	virtual Vector			GetSmoothedVelocity( void ) = 0;
	virtual void			VPhysicsDestroyObject( void ) = 0;
	virtual void			VPhysicsUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void			VPhysicsShadowUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void			VPhysicsCollision( int index, gamevcollisionevent_t *pEvent ) = 0;
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const = 0;
	virtual void			NetworkStateChanged_m_fFlags(void) = 0;
	virtual void			NetworkStateChanged_m_fFlags(void *) = 0;
	virtual void			NetworkStateChanged_m_nWaterLevel(void) = 0;
	virtual void			NetworkStateChanged_m_nWaterLevel(void *) = 0;
	virtual void			NetworkStateChanged_m_hGroundEntity(void) = 0;
	virtual void			NetworkStateChanged_m_hGroundEntity(void *) = 0;
	virtual void			NetworkStateChanged_m_vecBaseVelocity(void) = 0;
	virtual void			NetworkStateChanged_m_vecBaseVelocity(void *) = 0;
	virtual void			NetworkStateChanged_m_flFriction(void) = 0;
	virtual void			NetworkStateChanged_m_flFriction(void *) = 0;
	virtual void			NetworkStateChanged_m_vecVelocity(void) = 0;
	virtual void			NetworkStateChanged_m_vecVelocity(void *) = 0;
	virtual void			NetworkStateChanged_m_vecViewOffset(void) = 0;
	virtual void			NetworkStateChanged_m_vecViewOffset(void *) = 0;
	virtual void 			HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual const impactdamagetable_t	&GetPhysicsImpactDamageTable( void ) = 0;
	virtual QAngle			BodyAngles() = 0;
	virtual bool			Weapon_CanUse( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			Weapon_Equip( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = nullptr, const Vector *pVelocity = nullptr ) = 0;
	virtual	bool			Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 ) = 0;
	virtual	Vector			Weapon_ShootPosition( ) = 0;
	void					Weapon_DropSlot(int weaponSlot);
	IBaseCombatWeapon*		Weapon_GetLast(void) { return m_hLastWeapon.Get(); }
	virtual bool			RemovePlayerItem( CBaseCombatWeapon *pItem ) = 0;
	virtual int				OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual void			Event_Dying( void ) = 0;
	virtual bool 			IsInAVehicle( void ) const = 0;
	virtual IServerVehicle 	*GetVehicle( void ) = 0;
	virtual CBaseEntity 	*GetVehicleEntity( void ) = 0;
	virtual void			DoMuzzleFlash() = 0;

	virtual void			OnNavAreaChanged(CNavArea *,CNavArea *) = 0;
	virtual bool			IsGhost(void)const = 0;
private:
	virtual void			NetworkStateChanged_m_iAmmo(void) = 0;
	virtual void			NetworkStateChanged_m_iAmmo(void *) = 0;
public:
	virtual void			CreateViewModel( int viewmodelindex = 0 ) = 0;
	virtual void			SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize ) = 0;
	virtual bool			WantsLagCompensationOnEntity( const CBasePlayer	*pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const = 0;
	virtual void			SharedSpawn() = 0;
	virtual void			ForceRespawn( void ) = 0;
	virtual void			InitialSpawn( void ) = 0;
	virtual void			InitHUD( void ) = 0;
	virtual void			ShowViewPortPanel( const char * name, bool bShow = true, KeyValues *data = nullptr ) = 0;
	virtual void			PlayerDeathThink( void ) = 0;
	virtual void			Jump( void ) = 0;
	virtual void			Duck( void ) = 0;
	virtual void			Cough(CBasePlayer*) = 0;
	virtual void			PreThink( void ) = 0;
	virtual void			PostThink( void ) = 0;
	virtual void			DamageEffect(float flDamage, int fDamageType) = 0;
	virtual void			OnDamagedByExplosion( const CTakeDamageInfo &info ) = 0;
	virtual bool			ShouldFadeOnDeath( void ) = 0;
	virtual bool			IsFakeClient( void ) const = 0;
	virtual const char		*GetCharacterDisplayName(void) = 0;
	virtual const Vector	GetPlayerMins( void ) const = 0;
	virtual const Vector	GetPlayerMaxs( void ) const = 0;
	virtual void			UpdateCollisionBounds(void) = 0;
	virtual float			CalcRoll (const QAngle& angles, const Vector& velocity, float rollangle, float rollspeed) = 0;
	virtual void			PackDeadPlayerItems( void ) = 0;
	virtual void			RemoveAllItems( bool removeSuit ) = 0;
	virtual bool			IsRunning( void ) const = 0;
	virtual void			Weapon_SetLast( CBaseCombatWeapon *pWeapon ) = 0;
	virtual bool			Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon ) = 0;
	virtual bool			Weapon_ShouldSelectItem( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			UpdateClientData( void ) = 0;
	virtual void			ExitLadder() = 0;
	virtual surfacedata_t	*GetLadderSurface( const Vector &origin ) = 0;
	virtual bool			IsAbleToAutoCenterOnLadders(void)const = 0;
	virtual void			SetFlashlightEnabled( bool bState ) = 0;
	virtual int				FlashlightIsOn( void ) = 0;
	virtual void			FlashlightTurnOn( void ) = 0;
	virtual void			FlashlightTurnOff( void ) = 0;
	virtual bool			IsIlluminatedByFlashlight( CBaseEntity *pEntity, float *flReturnDot ) = 0;
	virtual void			UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity ) = 0;
	virtual void			PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force ) = 0;
	virtual void			GetStepSoundVelocities( float *velwalk, float *velrun ) = 0;
	virtual void			SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking ) = 0;
	virtual void			DeathSound( const CTakeDamageInfo &info ) = 0;
	virtual void			SetAnimation( int playerAnim ) = 0;
	virtual void			OnMainActivityComplete(Activity,Activity) = 0;
	virtual void			OnMainActivityInterrupted(Activity,Activity) = 0;
	virtual void			ImpulseCommands( void ) = 0;
	virtual void			CheatImpulseCommands( int iImpulse ) = 0;
	virtual bool			ClientCommand( const CCommand &args ) = 0;
	virtual bool			StartObserverMode(int mode) = 0;
	virtual void			StopObserverMode( void ) = 0;
	virtual bool			ModeWantsSpectatorGUI( int iMode ) = 0;
	virtual bool			SetObserverMode(int mode) = 0;
	virtual int				GetObserverMode( void ) = 0;
	virtual bool			SetObserverTarget(CBaseEntity * target) = 0;
	virtual void			ObserverUse( bool bIsPressed ) = 0;
	virtual CBaseEntity		*GetObserverTarget( void ) = 0;
	virtual CBaseEntity		*FindNextObserverTarget( bool bReverse ) = 0;
	virtual int				GetNextObserverSearchStartPoint( bool bReverse ) = 0;
	virtual int				PassesObserverFilter(CBaseEntity const*) = 0;
	virtual bool			IsValidObserverTarget(CBaseEntity * target) = 0;
	virtual void			CheckObserverSettings() = 0;
	virtual void			JumptoPosition(const Vector &origin, const QAngle &angles) = 0;
	virtual void			ForceObserverMode(int mode) = 0;
	virtual void			ResetObserverMode() = 0;
	virtual void			ValidateCurrentObserverTarget( void ) = 0;
	virtual void			AttemptToExitFreezeCam( void ) = 0;
	virtual int				WantsRoamingObserverMode(void)const = 0;
	virtual bool			StartReplayMode( float fDelay, float fDuration, int iEntity ) = 0;
	virtual void			StopReplayMode() = 0;
	virtual int				GetDelayTicks() = 0;
	virtual int				GetReplayEntity() = 0;
	virtual void			CreateCorpse( void ) = 0;
	virtual CBaseEntity		*EntSelectSpawnPoint( void ) = 0;
	virtual bool			GetInVehicle( IServerVehicle *pVehicle, int nRole ) = 0;
	virtual void			LeaveVehicle( const Vector &vecExitPoint/* = vec3_origin*/, const QAngle &vecExitAngles/* = vec3_angle*/ ) = 0;
	virtual void			OnVehicleStart() = 0;
	virtual void			OnVehicleEnd( Vector &playerDestPosition ) = 0;
	virtual bool			BumpWeapon( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			SelectLastItem(void) = 0;
	virtual void 			SelectItem( const char *pstr, int iSubType = 0 ) = 0;
	virtual int				SelectItem(CBaseCombatWeapon *) = 0;
	virtual void			ItemPostFrame( void ) = 0;
	virtual CBaseEntity		*GiveNamedItem( const char *szName, int iSubType, bool val) = 0;
	virtual void			CheckTrainUpdate( void ) = 0;
	virtual void			SetPlayerUnderwater( bool state ) = 0;
	virtual int				PlayWadeSound(void) = 0;
	virtual bool			CanBreatheUnderwater() const = 0;
	virtual int				CanRecoverCurrentDrowningDamage(void)const = 0;
	virtual void			PlayerUse( CBaseEntity* ) = 0;
	virtual void			PlayUseDenySound() = 0;
	virtual CBaseEntity		*FindUseEntity( float,float,bool * ) = 0;
	virtual bool			IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps ) = 0;
	virtual void			OnUseEntity(CBaseEntity *,USE_TYPE) = 0;
	virtual void			PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize = true ) = 0;
	virtual void			ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldindThis = nullptr ) = 0;
	virtual float			GetHeldObjectMass( IPhysicsObject *pHeldObject ) = 0;
	virtual int				IsHoldingEntity(CBaseEntity *) = 0;
	virtual void			UpdateGeigerCounter( void ) = 0;
	virtual Vector			GetAutoaimVector( float flScale ) = 0;
	virtual Vector			GetAutoaimVector( float flScale, float flMaxDist ) = 0;
	virtual Vector			GetAutoaimVector(float, float, float, unsigned int *) = 0;
	virtual Vector			GetAutoaimVector( autoaim_params_t &params ) = 0;
	virtual bool			ShouldAutoaim( void ) = 0;
	virtual void			ForceClientDllUpdate( void ) = 0;
	virtual void			ProcessUsercmds( CUserCmd *cmds, int numcmds, int totalcmds, int dropped_packets, bool paused ) = 0;
	virtual void			PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper) = 0;
	virtual bool			CanHearAndReadChatFrom( CBasePlayer *pPlayer ) = 0;
	virtual bool			CanSpeak( void ) = 0;
	virtual void 			ModifyOrAppendPlayerCriteria( AI_CriteriaSet& set ) = 0;
	virtual void			CheckChatText( char *p, int bufsize ) = 0;
	virtual void			CreateRagdollEntity( void ) = 0;
	virtual bool			ShouldAnnouceAchievement( void ) = 0;
	virtual int				EnsureSplitScreenTeam(void) = 0;
	virtual void			ForceChangeTeam(int) = 0;
	virtual bool			IsFollowingPhysics( void ) = 0;
	virtual void			InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity ) = 0;
	virtual void			UpdatePhysicsShadowToCurrentPosition() = 0;
	virtual CHintSystem		*Hints( void ) = 0;
	virtual bool			IsReadyToPlay( void ) = 0;
	virtual bool			IsReadyToSpawn( void ) = 0;
	virtual bool			ShouldGainInstantSpawn( void ) = 0;
	virtual void			ResetPerRoundStats( void ) = 0;
	virtual void			ResetScores( void ) = 0;
	virtual void 			EquipSuit( bool bPlayEffects = true ) = 0;
	virtual void 			RemoveSuit( void ) = 0;
	virtual void			OnUseEntityChanged(void) = 0;
	virtual void 			CommitSuicide( bool bExplode = false, bool bForce = false ) = 0;
	virtual void 			CommitSuicide( const Vector &vecForce, bool bExplode = false, bool bForce = false ) = 0;
	virtual bool 			IsBot() const = 0;
	virtual CAI_Expresser 	*GetExpresser() = 0;
protected:
	virtual int 			SpawnArmorValue(void)const = 0;
public:
	virtual unsigned int 	UpdateTonemapController(void) = 0;
private:
	int						m_StuckLast;									// 2224
	CNetworkVarEmbedded(CPlayerLocalData,		m_Local);					// 2228
	fogplayerparams_t		m_PlayerFog;									// 2668
	CUtlVector< CHandle< class CTonemapTrigger > > m_hTriggerTonemapList; 	// 2728
	CHandle< class CPostProcessController> m_hPostProcessCtrl;				// 2748
	CHandle< class CColorCorrection> m_hColorCorrectionCtrl;				// 2752
	CUtlVector<EHANDLE>		m_hTriggerSoundscapeList;						// 2756
public:
	CNetworkVarEmbedded(CPlayerState, 			pl);						// 2776
private:
	int						m_nButtons;										// 2828
	int						m_afButtonPressed;								// 2832
	int						m_afButtonReleased;								// 2836
	int						m_afButtonLast;									// 2840
	int						m_afButtonDisabled;								// 2844 A mask of input flags that are cleared automatically
	int						m_afButtonForced;								// 2848 These are forced onto the player's inputs
	CNetworkVar(bool,		m_fOnTarget);									// 2852
	char					m_szAnimExtension[32];							// 2553
	int						m_nUpdateRate;									// 2888 user snapshot rate cl_updaterate
	float					m_fLerpTime;									// 2892 users cl_interp
	bool					m_bLagCompensation;								// 2896 user wants lag compenstation
	bool					m_bPredictWeapons; 								// 2897 user has client side predicted weapons
	Activity				m_Activity;										// 2900
	Vector					m_vecAdditionalPVSOrigin; 						// 2904
	Vector					m_vecCameraPVSOrigin;							// 2916
	CNetworkHandle(IBaseEntity,	m_hUseEntity);								// 2928
	int						m_iTrain;										// 2932 Train control position
	float					m_iRespawnFrames;								// 2936 used in PlayerDeathThink() to make sure players can always respawn
 	unsigned int			m_afPhysicsFlags;								// 2940 physics flags - set when 'normal' physics should be revisited or overriden
	CNetworkHandle(IBaseEntity,	m_hVehicle);								// 2944
	int						m_iVehicleAnalogBias;							// 2948
	bool					m_bPauseBonusProgress;							// 2952
	CNetworkVar(int,		m_iBonusProgress);								// 2956
	CNetworkVar(int,		m_iBonusChallenge);								// 2960
	int						m_lastDamageAmount;								// 2964 Last damage taken
	float 					m_fTimeLastHurt;								// 2968
	Vector					m_DmgOrigin;									// 2972
	float					m_DmgTake;										// 2984
	float					m_DmgSave;										// 2988
	int						m_bitsDamageType;								// 2992 what types of damage has player taken
	int						m_bitsHUDDamage;								// 2996 Damage bits for the current fame. These get sent to the hud via gmsgDamage
	CNetworkVar(float,		m_flDeathTime);									// 3000 the time at which the player died  (used in PlayerDeathThink())
	float					m_flDeathAnimTime;								// 3004 the time at which the player finished their death anim (used in PlayerDeathThink() and ShouldTransmit())
	CNetworkVar(int,		m_iObserverMode);								// 3008 if in spectator mode != 0
	CNetworkVar(int,		m_iFOV);										// 3012 field of view
	CNetworkVar(int,		m_iDefaultFOV);									// 3016 default field of view
	CNetworkVar(int,		m_iFOVStart);									// 3020 What our FOV started at
	CNetworkVar(float,		m_flFOVTime);									// 3024 Time our FOV change started
	int						m_iObserverLastMode; 							// 3028
	CNetworkHandle(IBaseEntity,m_hObserverTarget);							// 3032
	bool					m_bForcedObserverMode; 							// 3036
	CNetworkHandle(IBaseEntity,m_hZoomOwner);								// 3040
	float					m_tbdPrev;										// 3044
	int						m_idrowndmg;									// 3048
	int						m_idrownrestored;								// 3052
	int						m_nPoisonDmg;									// 3056
	int						m_nPoisonRestored;								// 3060
	BYTE					m_rgbTimeBasedDamage[8];						// 3064
	int						m_vphysicsCollisionState;						// 3072
	float					m_fNextSuicideTime; 							// 3076
	int						m_iSuicideCustomKillFlags;						// 3080
	float					m_fDelay;										// 3084
	float					m_fReplayEnd;									// 3088
	int						m_iReplayEntity;								// 3092
	IHANDLES				m_hTonemapController;							// 3096
	CUtlVector< CCommandContext > m_CommandContext;							// 3100
	IPhysicsPlayerController	*m_pPhysicsController;						// 3120
	IPhysicsObject				*m_pShadowStand;							// 3124
	IPhysicsObject				*m_pShadowCrouch;							// 3128
	Vector						m_oldOrigin;								// 3132
	Vector						m_vecSmoothedVelocity;						// 3144
	bool						m_bTouchedPhysObject;						// 3156
	bool						m_bPhysicsWasFrozen;						// 3157
	int						m_iPlayerSound;									// 3160
	int						m_iTargetVolume;								// 3164
	int						m_rgItems[MAX_ITEMS];							// 3168
	IntervalTimers			m_lastHeldVoteTimer;							// 3188
	float					m_flSuitUpdate;									// 3196
	int						m_rgSuitPlayList[4];							// 3200
	int						m_iSuitPlayNext;								// 3216
	int						m_rgiSuitNoRepeat[32];							// 3220
	float					m_rgflSuitNoRepeatTime[32];						// 3348
	float					m_flgeigerRange;								// 3476
	float					m_flgeigerDelay;								// 3480
	int						m_igeigerRangePrev;								// 3484
	bool					m_fInitHUD;										// 3488
	bool					m_fGameHUDInitialized;							// 3489
	bool					m_fWeapon;										// 3490
	int						m_iUpdateTime;									// 3492
	int						m_iClientBattery;								// 3496
	QAngle					m_vecAutoAim;									// 3500
	int						m_iFrags;										// 3512
	int						m_iDeaths;										// 3516
	float					m_flNextDecalTime;								// 3520
	float					m_flNextTauntTime;								// 3524
	PlayerConnectedState	m_iConnected;									// 3528
	CNetworkVarForDerived(int,m_ArmorValue);								// 3532
public:
	virtual int 			IsAutoCrouched(void)const = 0;
	virtual int 			HasHaptics(void) = 0;
	virtual CBasePlayer*	SetHaptics(bool) = 0;
	virtual int 			GetAvailableSteadyStateSlots(void) = 0;
	virtual void 			OnSpeak(CBasePlayer*,char const*,float) = 0;
	virtual void 			OnVoiceTransmit(void) = 0;
	virtual int 			PlayerSolidMask(bool)const = 0;
private:
	float					m_AirFinished;									// 3536
	float					m_PainFinished;									// 3540
	int						m_iPlayerLocked;								// 3544
	float					m_flOffset_887;									// 3548
	CNetworkArray(CHandle<class CBaseViewModel>, m_hViewModel, MAX_VIEWMODELS);// 3552
	CUserCmd				m_LastCmd;										// 3560
	CUserCmd				*m_pCurrentCommand;								// 3648
	IntervalTimers			m_Offset_913;									// 3652
	float					m_flStepSoundTime;								// 3660 time to check for next footstep sound
	bool					m_bAllowInstantSpawn;							// 3664
	CNetworkVar(float,		m_flMaxspeed);									// 3668
	int						m_ladderSurfaceProps;							// 3672
	Vector					m_vecLadderNormal;								// 3676
	IHANDLES				m_hElevator;									// 3688
	float					m_flHeightAboveElevator;						// 3692
	float					m_flWaterJumpTime;  							// 3696 used to be called teleport_time
	Vector					m_vecWaterJumpVel;								// 3700
	int						m_nImpulse;										// 3712
	float					m_flSwimSoundTime;								// 3716
	float					m_flFlashTime;									// 3720
	int						m_nDrownDmgRate;								// 3724 Drowning damage in points per second without air.
	int						m_nNumCrouches;									// 3728 Number of times we've crouched (for hinting)
	bool					m_bDuckToggled;									// 3732 If true, the player is crouching via a toggle
	float					m_flForwardMove;								// 3736
	float					m_flSideMove;									// 3740
	int						m_nNumCrateHudHints;							// 3744
	Vector					m_vForcedOrigin;								// 3748
	bool					m_bForceOrigin;									// 3760
	CNetworkVar(int,		m_nTickBase);									// 3764
	bool					m_bGamePaused;									// 3768
	float					m_fLastPlayerTalkTime;							// 3772
	CNetworkVar(CHandle<IBaseCombatWeapon>, m_hLastWeapon);					// 3776
	CUtlVector< CHandle< IBaseEntity > > m_SimulatedByThisPlayer;			// 3780
	float					m_flOldPlayerZ;									// 3800
	float					m_flOldPlayerViewOffsetZ;						// 3804
	bool					m_bPlayerUnderwater;							// 3808
	IHANDLES				m_hViewEntity;									// 3812
	CNetworkHandle(IBaseEntity,m_hConstraintEntity);						// 3816
	CNetworkVector( 		m_vecConstraintCenter);							// 3820
	CNetworkVar(float, 		m_flConstraintRadius);							// 3832
	CNetworkVar(float, 		m_flConstraintWidth);							// 3836
	CNetworkVar(float, 		m_flConstraintSpeedFactor);						// 3840
	bool 					m_bConstraintPastRadius;						// 3844
	char					m_szNetname[32];								// 3845
	CNetworkVar(float,		m_flLaggedMovementValue);						// 3880
	Vector 					m_vNewVPhysicsPosition;							// 3884
	Vector 					m_vNewVPhysicsVelocity;							// 3896
	Vector					m_vecVehicleViewOrigin;							// 3908 Used to store the calculated view of the player while riding in a vehicle
	QAngle					m_vecVehicleViewAngles;							// 3920 Vehicle angles
	float					m_flVehicleViewFOV;								// 3932 FOV of the vehicle driver
	int						m_nVehicleViewSavedFrame;						// 3936 Used to mark which frame was the last one the view was calculated for
	Vector 					m_vecPreviouslyPredictedOrigin; 				// 3940 Used to determine if non-gamemovement game code has teleported, or tweaked the player's origin
	int						m_nBodyPitchPoseParam;							// 3952
	CNetworkString(			m_szLastPlaceName, MAX_PLACE_NAME_LENGTH);		// 3956
	char 					m_szNetworkIDString[MAX_NETWORKID_LENGTH];		// 3974
	CPlayerInfo 			m_PlayerInfo;									// 4040
	int						m_surfaceProps;									// 4052
	surfacedata_t*			m_pSurfaceData;									// 4056
	float					m_surfaceFriction;								// 4060
	char					m_chTextureType;								// 4064
	char					m_chPreviousTextureType;						// 4065
	bool					m_bSinglePlayerGameEnding;						// 4066
	CNetworkVar(int,		m_ubEFNoInterpParity);							// 4068
	bool					m_bhasHaptics;									// 4072
	bool					m_autoKickDisabled;								// 4073

	struct StepSoundCache_t
	{
		StepSoundCache_t() : m_usSoundNameIndex( 0 ) {}
		CSoundParameters	m_SoundParameters;
		unsigned short		m_usSoundNameIndex;
	};

	StepSoundCache_t		m_StepSoundCache[ 2 ];							// 4076
	CUtlLinkedList< CPlayerSimInfo >  m_vecPlayerSimInfo;					// 4420
	CUtlLinkedList< CPlayerCmdInfo >  m_vecPlayerCmdInfo;					// 4448
	Vector					m_movementCollisionNormal;						// 4476
	Vector					m_groundNormal;									// 4488
	CHandle< CBaseCombatCharacter > m_stuckCharacter;						// 4500

	char 					m_Unknown4504_4516[13];

	bool					m_bSplitScreenPlayer;							// 4517
	CHandle< CBasePlayer > 	m_hSplitOwner;									// 4520
	CUtlVector< CHandle< CBasePlayer > > m_hSplitScreenPlayers;				// 4524

};

#endif