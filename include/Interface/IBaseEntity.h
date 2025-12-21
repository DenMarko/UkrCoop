#ifndef _INCLUDE_BASE_ENTITY_H_
#define _INCLUDE_BASE_ENTITY_H_

#include <extension.h>
#include <CTrace.h>
#include <vphysics/vehicles.h>
#include "IServerNetworkPropperty.h"
#include "ICollisionProperty.h"

template <typename T>
T *GetVirtualClass(CBaseEntity *pEntity)
{
	return reinterpret_cast<T*>(pEntity);
}

template <typename T>
T *GetVirtualClass(const CBaseEntity *pEntity)
{
	return reinterpret_cast<T*>((CBaseEntity *)pEntity);
}

template <typename T, typename N>
T *GetVirtualClass(N *pEntity)
{
	return reinterpret_cast<T*>(pEntity);
}

template <typename T>
T *GetVirtualClass(int id)
{
	CBaseEntity *ptr = gamehelpers->ReferenceToEntity(id);
	return reinterpret_cast<T*>(ptr);
}

enum notify_system_event_t
{
	NOTIFY_EVENT_TELEPORT = 0,
	NOTIFY_EVENT_DESTROY,
};

enum Class_T
{
	CLASS_NONE = 0,
	CLASS_PLAYER,
	CLASS_PLAYER_ALLY,
	NUM_AI_CLASSES
};

typedef int EntityEvent_t;
typedef unsigned int UtlHashHandle_t;

struct notify_system_event_params_t;
struct gamevcollisionevent_t;

class CBaseCombatWeapon;
class CEntityMapData;
class IEntitySaveUtils;
class CBaseAnimating;
class IResponseSystem;
class CBaseCombatCharacter;
class AI_CriteriaSet;
class CAI_BaseNPC;
class CBasePlayer;
class CMoveData;
class CUserCmd;
class CNPC_VehicleDriver;

class ITeam;
class IInfected;
class INextBot;
class CDamageModifier;

class IntervalTimers
{
public:
	DECLARE_CLASS_NOBASE( IntervalTimers );

	virtual void NetworkStateChanged() {} 
	virtual void NetworkStateChanged( void *pProp ) {}	
	
	IntervalTimers( void )
	{
		m_timestamp = -1.0f;
	}

	void Reset( void )
	{
		m_timestamp = Now();
	}		

	void Start( void )
	{
		m_timestamp = Now();
	}

	void StartFromTime( float startTime )
	{
		m_timestamp = startTime;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}		

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	/// if not started, elapsed time is very large
	float GetElapsedTime( void ) const
	{
		return (HasStarted()) ? (Now() - m_timestamp) : 99999.9f;
	}

	bool IsLessThen( float duration ) const
	{
		return (Now() - m_timestamp < duration) ? true : false;
	}

	bool IsGreaterThen( float duration ) const
	{
		return (Now() - m_timestamp > duration) ? true : false;
	}

	float GetStartTime( void ) const
	{
		return m_timestamp;
	}

protected:
	CNetworkVar( float, m_timestamp ); // 4

	float Now( void ) const;		// work-around since client header doesn't like inlined gpGlobals->curtime
};

class CountdownTimers
{
public:
	DECLARE_CLASS_NOBASE( CountdownTimers );

	virtual void NetworkStateChanged() {} 
	virtual void NetworkStateChanged( void *pProp ) {}

	CountdownTimers( void )
	{
		m_timestamp = -1.0f;
		m_duration = 0.0f;
	}

	void Reset( void )
	{
		m_timestamp = Now() + m_duration;
	}		

	void Start( float duration )
	{
		m_timestamp = Now() + duration;
		m_duration = duration;
	}
	
	void StartFromTime( float startTime, float duration )
	{
		m_timestamp = startTime + duration;
		m_duration = duration;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}		

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	// Минув
	bool IsElapsed( void ) const
	{
		return (Now() > m_timestamp);
	}

	float GetElapsedTime( void ) const
	{
		return Now() - m_timestamp + m_duration;
	}

	float GetRemainingTime( void ) const
	{
		return (m_timestamp - Now());
	}

	float GetTargetTime() const
	{
		return m_timestamp;
	}

	float GetCountdownDuration( void ) const
	{
		return (m_timestamp > 0.0f) ? m_duration : 0.0f;
	}

	float GetRemainingRatio( void ) const
	{
		if ( HasStarted() )
		{
			float left = GetRemainingTime() / m_duration;
			if ( left < 0.0f )
				return 0.0f;
			if ( left > 1.0f )
				return 1.0f;
			return left;
		}
		
		return 0.0f;
	}

	float GetElapsedRatio() const
	{
		if ( HasStarted() )
		{
			float elapsed = GetElapsedTime() / m_duration;
			if ( elapsed < 0.0f )
				return 0.0f;
			if ( elapsed > 1.0f )
				return 1.0f;
			return elapsed;
		}

		return 1.0f;
	}

	bool RunEvery( float amount = -1.0f )
	{
		// First call starts the timer
		if(!HasStarted())
		{
			if(amount > 0.0f)
				Start( amount );

			return false;
		}

		if( IsElapsed() )
		{
			if ( amount > 0.0f )
				m_duration = amount;

			m_timestamp += m_duration;
			return true;
		}

		return false;
	}

	bool Interval( float amount = -1.0f )
	{
		// First call starts the timer
		if ( !HasStarted() )
		{
			if ( amount > 0.0f )
				Start( amount );

			return false;
		}

		if ( IsElapsed() )
		{
			if ( amount > 0.0f )
				m_duration = amount;

			m_timestamp += m_duration;

			// If we are still expired, add a multiple of the interval 
			// until we become non-elapsed
			float remaining = GetRemainingTime();
			if ( remaining < 0.0f)
			{
				float numIntervalsRequired = -floorf( remaining / m_duration );
				m_timestamp += m_duration * numIntervalsRequired;
			}

			// We should no longer be elapsed
			Assert( !IsElapsed() );

			return true;
		}

		return false;
	}

private:
	CNetworkVar( float, m_duration );	// 4
	CNetworkVar( float, m_timestamp );	// 8
	float Now( void ) const;
};

class IVehicle
{
public:
	virtual CBaseCombatCharacter*	GetPassenger( int nRole = VEHICLE_ROLE_DRIVER ) = 0;
	virtual int						GetPassengerRole( CBaseCombatCharacter *pPassenger ) = 0;
	
	virtual void			GetVehicleViewPosition( int nRole, Vector *pOrigin, QAngle *pAngles, float *pFOV = NULL ) = 0;

	virtual bool			IsPassengerUsingStandardWeapons( int nRole = VEHICLE_ROLE_DRIVER ) = 0;

	virtual void			SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move ) = 0;
	virtual void			ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData ) = 0;
	virtual void			FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move ) = 0;

	virtual void			ItemPostFrame( CBasePlayer *pPlayer ) = 0;
};


abstract_class IServerVehicle : public IVehicle
{
public:
	virtual CBaseEntity*	GetVehicleEnt() = 0;

	virtual void			SetPassenger( int nRole, CBaseCombatCharacter *pPassenger ) = 0;
	
	virtual bool			IsPassengerVisible( int nRole = 0 ) = 0;

	virtual bool			IsPassengerDamagable( int nRole  = 0 ) = 0;
	virtual bool			PassengerShouldReceiveDamage( CTakeDamageInfo &info ) = 0;

	virtual bool			IsVehicleUpright( void ) = 0;

	virtual bool			IsPassengerEntering( void ) = 0;
	virtual bool			IsPassengerExiting( void ) = 0;

	virtual void			GetPassengerSeatPoint( int nRole, Vector *pPoint, QAngle *pAngles ) = 0;

	virtual void			HandlePassengerEntry( CBaseCombatCharacter *pPassenger, bool bAllowEntryOutsideZone = false ) = 0;
	virtual bool			HandlePassengerExit( CBaseCombatCharacter *pPassenger ) = 0;

	virtual bool			GetPassengerExitPoint( int nRole, Vector *pPoint, QAngle *pAngles ) = 0;
	virtual int				GetEntryAnimForPoint( const Vector &vecPoint ) = 0;
	virtual int				GetExitAnimToUse( Vector &vecEyeExitEndpoint, bool &bAllPointsBlocked ) = 0;
	virtual void			HandleEntryExitFinish( bool bExitAnimOn, bool bResetAnim ) = 0;

	virtual Class_T			ClassifyPassenger( CBaseCombatCharacter *pPassenger, Class_T defaultClassification ) = 0;
	virtual float			PassengerDamageModifier( const CTakeDamageInfo &info ) = 0;

	virtual const vehicleparams_t	*GetVehicleParams( void ) = 0;
	virtual IPhysicsVehicleController *GetVehicleController() = 0;

	virtual int				NPC_GetAvailableSeat( CBaseCombatCharacter *pPassenger, string_t strRoleName, VehicleSeatQuery_e nQueryType ) = 0;
	virtual bool			NPC_AddPassenger( CBaseCombatCharacter *pPassenger, string_t strRoleName, int nSeat ) = 0;
	virtual bool			NPC_RemovePassenger( CBaseCombatCharacter *pPassenger ) = 0;
	virtual bool			NPC_GetPassengerSeatPosition( CBaseCombatCharacter *pPassenger, Vector *vecResultPos, QAngle *vecResultAngle ) = 0;
	virtual bool			NPC_GetPassengerSeatPositionLocal( CBaseCombatCharacter *pPassenger, Vector *vecResultPos, QAngle *vecResultAngle ) = 0;
	virtual int				NPC_GetPassengerSeatAttachment( CBaseCombatCharacter *pPassenger ) = 0;
	virtual bool			NPC_HasAvailableSeat( string_t strRoleName ) = 0;
	
	virtual const PassengerSeatAnims_t	*NPC_GetPassengerSeatAnims( CBaseCombatCharacter *pPassenger, PassengerSeatAnimType_t nType ) = 0;
	virtual CBaseCombatCharacter		*NPC_GetPassengerInSeat( int nRoleID, int nSeatID ) = 0;

	virtual void			RestorePassengerInfo( void ) = 0;

	virtual bool			NPC_CanDrive( void ) = 0;
	virtual void			NPC_SetDriver( CNPC_VehicleDriver *pDriver ) = 0;
  	virtual void			NPC_DriveVehicle( void ) = 0;
	virtual void			NPC_ThrottleCenter( void ) = 0;
	virtual void			NPC_ThrottleReverse( void ) = 0;
	virtual void			NPC_ThrottleForward( void ) = 0;
	virtual void			NPC_Brake( void ) = 0;
	virtual void			NPC_TurnLeft( float flDegrees ) = 0;
	virtual void			NPC_TurnRight( float flDegrees ) = 0;
	virtual void			NPC_TurnCenter( void ) = 0;
	virtual void			NPC_PrimaryFire( void ) = 0;
	virtual void			NPC_SecondaryFire( void ) = 0;
	virtual bool			NPC_HasPrimaryWeapon( void ) = 0;
	virtual bool			NPC_HasSecondaryWeapon( void ) = 0;
	virtual void			NPC_AimPrimaryWeapon( Vector vecTarget ) = 0;
	virtual void			NPC_AimSecondaryWeapon( Vector vecTarget ) = 0;

	virtual void			Weapon_PrimaryRanges( float *flMinRange, float *flMaxRange ) = 0;	
	virtual void			Weapon_SecondaryRanges( float *flMinRange, float *flMaxRange ) = 0;	
	virtual float			Weapon_PrimaryCanFireAt( void ) = 0;
	virtual float			Weapon_SecondaryCanFireAt( void ) = 0;

	virtual void			ReloadScript() = 0;
};


/**
 * Дії які можна робити тільки визов не створення!!!
 */
class CEventAction
{
public:
	// CEventAction( const char *ActionData = NULL );
	// CEventAction( const CEventAction &p_EventAction );

	string_t m_iTarget;
	string_t m_iTargetInput;
	string_t m_iParameter;
	float m_flDelay;
	int m_nTimesToFire;

	int m_iIDStamp;

	// static int s_iNextIDStamp;

	CEventAction *m_pNext; 
};

#define EVENT_FIRE_ALWAYS	-1
#define ADD_DEBUG_HISTORY( category, line )		((void)0)

class CBaseEntityOutput
{
public:
	~CBaseEntityOutput();

	// void ParseEventAction( const char *EventData );
	// void AddEventAction( CEventAction *pEventAction );
	void RemoveEventAction( CEventAction *pEventAction );

	// int Save( ISave &save );
	// int Restore( IRestore &restore, int elementCount );

	int NumberOfElements( void );

	float GetMaxDelay( void );

	fieldtype_t ValueFieldType() { return m_Value.FieldType(); }

	void FireOutput( variant_t Value, IBaseEntity *pActivator, IBaseEntity *pCaller, float fDelay = 0 );

	/// Delete every single action in the action list. 
	void DeleteAllElements( void ) ;

	CEventAction *GetFirstAction() { return m_ActionList; }

	const CEventAction *GetActionForTarget( string_t iSearchTarget ) const;
protected:
	variant_t m_Value;
	CEventAction *m_ActionList;

	CBaseEntityOutput() {} // this class cannot be created, only it's children

private:
	CBaseEntityOutput( CBaseEntityOutput& ); // protect from accidental copying
};

template< class Type, fieldtype_t fieldType >
class CEntityOutputTemplate : public CBaseEntityOutput
{
public:
	void Init( Type value ) 
	{
		m_Value.Set( fieldType, &value );
	}

	void Set( Type value, IBaseEntity *pActivator, IBaseEntity *pCaller ) 
	{
		m_Value.Set( fieldType, &value );
		FireOutput( m_Value, pActivator, pCaller );
	}

	Type Get( void )
	{
		return *((Type*)&m_Value);
	}
};


class COutputEvent : public CBaseEntityOutput
{
public:
	void FireOutput(IBaseEntity* pActivator, IBaseEntity* pCaller, float fDelay = 0);
};

typedef CEntityOutputTemplate<float, FIELD_FLOAT>			COutputFloat;


struct groundlink_t
{
	EHANDLE					entity;		// offset byte 0
	groundlink_t			*nextLink;	// offset byte 8
	groundlink_t			*prevLink;	// offset byte 16
};

typedef void (IBaseEntity::*BASEPTR)(void);

struct thinkfunc_t
{
	BASEPTR		m_pfnThink;
	string_t	m_iszContext;
	int			m_nNextThinkTick;
	int			m_nLastThinkTick;

	DECLARE_SIMPLE_DATADESC();
};

struct TimedOverlay_t
{
	char 			*msg;
	int				msgEndTime;
	int				msgStartTime;
	TimedOverlay_t	*pNextTimedOverlay; 
};

struct ResponseContext_t
{
	DECLARE_SIMPLE_DATADESC();

	string_t		m_iszName;
	string_t		m_iszValue;
	float			m_fExpirationTime;		// when to expire context (0 == never)
};

#define SetThink( a ) ThinkSet( static_cast <void (IBaseEntity::*)(void)> (a), 0, NULL )
#define SetContextThink( a, b, context ) ThinkSet( static_cast <void (IBaseEntity::*)(void)> (a), (b), context )

class IBaseEntity : public IServerEntity
{
public:
	typedef IBaseEntity ThisClass;

	IBaseEntity(bool bServerOnly = false);

	const char *GetDebugName() const;
	const int GetHealth();
	const int GetMaxHealth();
	const Vector &GetAbsOrigin() const;
	void SetAbsOrigin(const Vector &absOrigin);
	const QAngle &GetAbsAngles() const;
	void SetAbsAngles(const QAngle &absAngles);
	const Vector &GetAbsVelocity() const;
	void SetAbsVelocity(const Vector &vecAbsVelocity);
	const Vector &GetLocalOrigin() const;
	const QAngle &GetLocalAngles() const;
	void CalcAbsolutePosition();
	void CalcAbsoluteVelocity();
	matrix3x4_t& GetParentToWorldTransform( matrix3x4_t &tempMatrix );
	void DispatchTraceAttack(const CTakeDamageInfo &info, const Vector& vecDir, trace_t *tr);
	void TraceAttackToTriggers(const CTakeDamageInfo &info, const Vector& vecStart, const Vector& vecEnd, const Vector& vecDir);
	void TakeDamage(const CTakeDamageInfo& info);
	string_t GetClassname() const;
	bool ClassMatches(const char *strClassOrWildcard);
	IPhysicsObject *VPhysicsGetObject() const;
	bool			VPhysicsInitSetup();
	IPhysicsObject *VPhysicsInitNormal( SolidType_t solidType, int nSolidFlags, bool createAsleep, struct solid_t *pSolid = NULL );
	IPhysicsObject *VPhysicsInitShadow( bool allowPhysicsMovement, bool allowPhysicsRotation, solid_t *pSolid = NULL );
	void VPhysicsSetObject( IPhysicsObject *pPhysics );
	matrix3x4_t &EntityToWorldTransform();
	const matrix3x4_t &EntityToWorldTransform() const;
	void PhysicsStepRecheckGround();
	void SetMoveType(MoveType_t val, MoveCollide_t moveCollide = MOVECOLLIDE_DEFAULT);
	void AddEFlags(int nEFlags);
	void RemoveEFlags(int nEFlags);
	bool IsEFlagsSet(int nEFlags) const;
	int DispatchUpdateTransmitState();
	void CollisionRulesChanged();
	void SetSimulatedEveryTick(bool bSim);
	bool IsSimulatedEveryTick() const;
	void SetAnimatedEveryTick(bool bAnim);
	void UpdateWaterState();
	void CheckStepSimulationChanged();
	void CheckHasGamePhysicsSimulation();
	bool WillSimulateGamePhysics();
	bool HasDataObjectType(int type) const;
	void AddDataObjectType(int type);
	void RemoveDataObjectType(int type);
	float GetMoveDoneTime();
	void *CreateDataObject(int type);
	void DestroyDataObject(int type);
	void AddEffects(int nEffects);
	bool IsCurrentlyTouching(void);
	int GetParentAttachment();
	IBaseEntity *FirstMoveChild();
	IBaseEntity *NextMovePeer( void );
	void InvalidatePhysicsRecursive(int nChangeFlags);
	bool IsEffectActive(int nEffects);
	IBaseEntity* GetMoveParent();
	void SetModelName(string_t name);
	model_t *GetModel();
	int GetEFlags() const;
	MoveType_t GetMoveType() const;
	int GetTeamNumber() const;
	ITeam *GetTeam(void) const;
	const char* TeamID(void) const;
	bool InSameTeam(IBaseEntity *pEnt) const;
	bool IsInTeam(ITeam *pTeam) const;
	bool IsInAnyTeam(void) const;
	void RemoveEffects(int nEffencts);
	int GetCollisionGroup() const;
	IBaseEntity *GetRootMoveParent();

	bool		 BlocksLOS( void );
	groundlink_t *AddEntityToGroundList(IBaseEntity *);
	void PhysicsNotifyOtherOfGroundRemoval(IBaseEntity* ent, IBaseEntity *other);

	const edict_t *edict() const;
	edict_t *edict();
	int entindex();

	void FollowEntity(IBaseEntity *pBaseEntity, bool bBoneMerge = true);
	void StopFollowingEntity();
	bool IsFollowingEntity();
	IBaseEntity *GetFollowedEntity();
	void SetEffectEntity(IBaseEntity* pEffectEnt);

	void NetworkStateChanged();
	void NetworkStateChanged(unsigned short varOffset);
	void NetworkStateChanged(void *pVar);

	void SetParent(string_t newParent, IBaseEntity *pActivator, int iAttachment = -1);
	void SetSimulationTime(float st);
	const float GetSimulationTime() const;
	void SetLocalOrigin(const Vector& vec);
	void SetLocalAngles(const QAngle& vec);
	void SetLocalVelocity(const Vector&);
	void SetCheckUntouch(bool check);
	void SetWaterLevel(int nLevel);
	int GetWaterLevel() const;
	void SetWaterType(int nType);
	void SetRenderColor(byte r, byte g, byte b);
	void SetGroundEntity(IBaseEntity *);

	void AddFlag(int flags);
	void RemoveFlag(int flags);
	void ClearFlags();
	int GetFlags(void) const;
	bool IsMarkedForDeletion(void);

	IServerNetworkProperty *NetworkProp();
	ICollisionProperty *CollisionProp();

	const IServerNetworkProperty* NetworkProp() const;
	const ICollisionProperty *CollisionProp() const;

	void ClearSolidFlags(void);
	void RemoveSolidFlags(int flags);
	void AddSolidFlags(int iFlags);
	int GetSolidFlags(void) const;
	bool IsSolidFlagSet(int flagMask) const;
	bool IsSolid(void) const;
	void SetSolid(SolidType_t val);
	void SetSolidFlags(int flags);
	SolidType_t GetSolid(void) const;

	const Vector &WorldAlignMins() const;
	const Vector &WorldAlignMaxs() const;
	const Vector &WorldAlignSize() const;
	float BoundingRadius() const;

	bool IsPointSized() const;
	bool IsTransparent() const;
	bool IsInWorld(void) const;

	IBaseEntity *GetOwnerEntity(void) const;
	IBaseEntity *GetEffectEntity(void) const;
	IBaseEntity *GetGroundEntity(void) const;
	char GetTakedamage();

	void EmitSound(const char* szSoundName, float soundTime = 0.0f, float *duration = nullptr);
	void EmitSound(const char* szSoundName, HSOUNDSCRIPTHANDLE& handle, float soundtime = 0.f, float* duration = nullptr);
	void EmitSound(IRecipientFilter& filter, int iEntIndex, const char* szSoundName, const Vector* pOrigin = nullptr, float soundtime = 0.0f, float* duration = nullptr);
	void EmitSound(IRecipientFilter& filter, int iEntIndex, const char* szSoundName, HSOUNDSCRIPTHANDLE& handle, const Vector *pOrigin = nullptr, float soundtime = 0.f, float *duration = nullptr);

	int			GetSpawnFlags( void ) const;
	void		AddSpawnFlags( int nFlags );
	void		RemoveSpawnFlags( int nFlags );
	void		ClearSpawnFlags( void );
	bool		HasSpawnFlags( int nFlags ) const;

	IBaseEntity *GetParent();
	void 		PostClientMessagesSent( void );

	bool 		IsEFlagSet( int nEFlagMask ) const;
	void		SetName( string_t newTarget );
	string_t	GetEntityName();
	void		RecalcHasPlayerChildBit();
	void		SetCollisionGroup( int collisionGroup );
	bool		IsWorld() { return entindex() == 0; }
	void		SetNextThink(float thinkTime, const char *szContext = NULL);
	void		CheckHasThinkFunction( bool isThinkingHint = false );
	bool		WillThink();
	int			GetIndexForThinkContext( const char *pszContext );
	int			RegisterThinkContext( const char *szContext );
	const char	GetLifeState( void ) const;

	BASEPTR		ThinkSet( BASEPTR func, float flNextThinkTime = 0, const char *szContext = NULL );

	static int						PrecacheModel( const char *name ); 

	void DrawOutputOverlay(CEventAction* ev);
	void AddTimedOverlay( const char* msg, int endTime);
	void				SetRenderFX( RenderFx_t nRenderFX );
	RenderFx_t			GetRenderFX() const;

	void	SetFadeDistance( float minFadeDist, float maxFadeDist );
	void	SetGlobalFadeScale( float flFadeScale );
	float	GetGlobalFadeScale() const;

public:
	virtual ~IBaseEntity() { }

	virtual void			SetRefEHandle( const CBaseHandle &handle ) = 0;
	virtual const			CBaseHandle& GetRefEHandle() const = 0;

	virtual ICollideable	*GetCollideable() = 0;
	virtual IServerNetworkable *GetNetworkable() = 0;
	virtual CBaseEntity		*GetBaseEntity() = 0;

public:
	virtual void			SetModelIndex( int index ) = 0;
	virtual int				GetModelIndex( void ) const = 0;
 	virtual string_t		GetModelName( void ) const = 0;

public:
	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual bool			TestCollision( const Ray_t& ray, unsigned int mask, trace_t& trace ) = 0;
	virtual	bool			TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr ) = 0;
	virtual void			ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs ) = 0;
	virtual	bool			ShouldCollide( int collisionGroup, int contentsMask ) const = 0;
	virtual void			SetOwnerEntity( CBaseEntity* pOwner ) = 0;
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo ) = 0;
	virtual int				UpdateTransmitState() = 0;
	virtual void			SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways ) = 0;
	virtual const char*		GetTracerType( void ) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			SetModel( const char *szModelName ) = 0;
	virtual void			PostConstructor( const char *szClassname ) = 0;
	virtual void			PostClientActive( void ) = 0;
	virtual void			ParseMapData( CEntityMapData *mapData ) = 0;
	virtual bool			KeyValue( const char *szKeyName, const char *szValue ) = 0;
	virtual bool			KeyValue( const char *szKeyName, float flValue ) = 0;
	virtual bool			KeyValue( const char *szKeyName, const Vector &vecValue ) = 0;
	virtual bool			GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen ) = 0;
	virtual void			Activate( void ) = 0;
	virtual void			SetParent( CBaseEntity* pNewParent, int iAttachment = -1 ) = 0;
	virtual int				ObjectCaps( void ) = 0;
	virtual int				GetUsePriority(CBaseEntity*) = 0;
	virtual CBaseEntity		*GetGlowEntity(void) = 0;
	virtual int				GetUseType(CBaseEntity*) = 0;
	virtual bool			AcceptInput( const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID ) = 0;
	virtual	void			DrawDebugGeometryOverlays(void) = 0;					
	virtual int				DrawDebugTextOverlays(void) = 0;
	virtual int				Save( ISave &save ) = 0;
	virtual int				Restore( IRestore &restore ) = 0;
	virtual bool			ShouldSavePhysics() = 0; //172
	virtual void			OnSave( IEntitySaveUtils *pSaveUtils ) = 0;
	virtual void			OnRestore() = 0;
	virtual int				RequiredEdictIndex( void ) = 0;

	void (IBaseEntity::*m_pfnMoveDone)(void);									// 4
	virtual void MoveDone( void );

	void (IBaseEntity::*m_pfnThink)(void);										// 12
	virtual void Think( void );
	void SetLastProgressBarUpdateTime() { m_flLastProgressBarUpdateTime = g_pGlobals->curtime; }
	
private:
	IServerNetworkProperty m_Network;											// 20

	float m_flLastProgressBarUpdateTime;										// 100	?
	char m_Unknown[4];															// 104	?
public:
	string_t 			m_iClassname;											// 108
private:
	string_t 			m_iGlobalname;											// 112
	string_t 			m_iParent;												// 116

	int					m_iHammerID;											// 120
public:
	float				m_flPrevAnimTime;										// 124
	CNetworkVar(float,	m_flAnimTime);											// 128
	CNetworkVar(float,	m_flSimulationTime);									// 132
private:
	CNetworkVar(float,	m_flCreateTime);										// 136
	int					m_nLastThinkTick;										// 140
	int					touchStamp;												// 144
	CUtlVector< thinkfunc_t >		m_aThinkFunctions;							// 148
	CUtlVector< ResponseContext_t > m_ResponseContexts;							// 168
	string_t			m_iszResponseContext;									// 188

	CNetworkVarForDerived(int, m_nNextThinkTick);								// 192

public:
	virtual CBaseAnimating*	GetBaseAnimating() = 0;
	virtual IResponseSystem *GetResponseSystem() = 0;
	virtual void			DispatchResponse( const char *conceptName ) = 0;
	virtual Class_T			Classify ( void ) = 0;
	virtual void			DeathNotice ( CBaseEntity *pVictim ) = 0;
	virtual bool			ShouldAttractAutoAim( CBaseEntity *pAimingEnt ) = 0;
	virtual float			GetAutoAimRadius() = 0;
	virtual Vector			GetAutoAimCenter() = 0;
	virtual ITraceFilter*	GetBeamTraceFilter( void ) = 0;
	virtual bool			PassesDamageFilter( const CTakeDamageInfo &info ) = 0;
protected:
	virtual void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr ) = 0;
public:
	virtual bool			CanBeHitByMeleeAttack( CBaseEntity *pAttacker ) = 0;
	virtual int				OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual int				TakeHealth( float flHealth, int bitsDamageType ) = 0;
	virtual bool			IsAlive( void ) = 0;
	virtual void			Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual void			Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual int				BloodColor( void ) = 0;
	virtual bool			IsTriggered( CBaseEntity *pActivator ) = 0;
	virtual bool			IsNPC( void ) const = 0;
	virtual CBaseCombatCharacter *MyCombatCharacterPointer( void ) = 0;
	virtual INextBot		*MyNextBotPointer(void) = 0;
	virtual IInfected		*MyInfectedPointer(void) = 0;
	virtual float			GetDelay( void ) = 0;
	virtual bool			IsMoving( void ) = 0;
	virtual char const*		DamageDecal( int bitsDamageType, int gameMaterial ) = 0;
	virtual void			DecalTrace( trace_t *pTrace, char const *decalName ) = 0;
	virtual void			ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName = nullptr ) = 0;
	virtual bool			OnControls( CBaseEntity *pControls ) = 0;
	virtual bool			HasTarget( string_t targetname ) = 0;
	virtual	bool			IsPlayer( void ) const = 0;
	virtual bool			IsNetClient( void ) const = 0;
	virtual bool			IsTemplate( void ) = 0;
	virtual bool			IsBaseObject( void ) const = 0;
	virtual bool			IsBaseCombatWeapon(void) const = 0;
	virtual CBaseCombatWeapon* MyCombatWeaponPointer(void) = 0;
	virtual IServerVehicle*	GetServerVehicle() = 0;
	virtual bool			IsViewable( void ) = 0;
	virtual void			ChangeTeam( int iTeamNum ) = 0;
	virtual void 			OnEntityEvent( EntityEvent_t event, void *pEventData ) = 0;
	virtual bool			CanStandOn( CBaseEntity *pSurface ) const = 0;
	virtual bool			CanStandOn( edict_t	*ent ) const = 0;
	virtual CBaseEntity		*GetEnemy( void ) = 0;
	virtual CBaseEntity		*GetEnemy( void ) const = 0;
	virtual void			Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) = 0;
	virtual void			StartTouch( CBaseEntity *pOther ) = 0;
	virtual void			Touch( CBaseEntity *pOther ) = 0;
	virtual void			EndTouch( CBaseEntity *pOther ) = 0;
	virtual void			StartBlocked( CBaseEntity *pOther ) = 0;
	virtual void			Blocked( CBaseEntity *pOther ) = 0;
	virtual void			EndBlocked( void ) = 0;
	virtual void			PhysicsSimulate( void ) = 0;
	virtual void			UpdateOnRemove( void ) = 0;
	virtual void			StopLoopingSounds( void ) = 0;
	virtual	bool			SUB_AllowedToFade( void ) = 0;
	virtual void			Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity ) = 0;
	virtual void			NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params ) = 0;
	virtual void			MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType ) = 0;
	virtual int				GetTracerAttachment( void ) = 0;
	virtual void 			FireBullets( const FireBulletsInfo_t &info ) = 0;
	virtual void 			DoImpactEffect( trace_t &tr, int nDamageType ) = 0;
	virtual CBaseEntity*	Respawn( void ) = 0;
	virtual bool			IsLockedByMaster( void ) = 0;
	virtual void			ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
	virtual void			ModifyOrAppendDerivedCriteria(AI_CriteriaSet &) = 0;

	CNetworkVar(int, m_fEffects);													// 196
	void (IBaseEntity ::*m_pfnTouch)( IBaseEntity *pOther );						// 200
	void (IBaseEntity ::*m_pfnUse)( IBaseEntity *pActivator, IBaseEntity *pCaller, USE_TYPE useType, float value ); // 208
	void (IBaseEntity ::*m_pfnBlocked)( IBaseEntity *pOther );						// 216

	IBaseEntity*			m_pLink;												// 224
	string_t				m_target;												// 228
	CNetworkVarForDerived(int, m_iMaxHealth);										// 232
	CNetworkVarForDerived(int, m_iHealth);											// 236
	CNetworkVarForDerived(char, m_lifeState);										// 240
	CNetworkVarForDerived(char, m_takedamage);										// 241

	virtual int				GetDamageType() const = 0;
	virtual float			GetDamage() = 0;
	virtual void			SetDamage(float flDamage)  = 0;
	virtual Vector			EyePosition( void ) = 0;
	virtual const QAngle 	&EyeAngles( void ) = 0;
	virtual const QAngle 	&LocalEyeAngles( void ) = 0;
	virtual Vector			EarPosition( void ) = 0;
	virtual Vector			BodyTarget( const Vector &posSrc, bool bNoisy = true) = 0;
	virtual Vector			HeadTarget( const Vector &posSrc ) = 0;
	virtual void			GetVectors(Vector* forward, Vector* right, Vector* up) const = 0;
	virtual const Vector 	&GetViewOffset() = 0;
	virtual void			SetViewOffset( const Vector &vecOffset ) = 0;
	virtual Vector			GetSmoothedVelocity( void ) = 0;
	virtual void			GetVelocity(Vector *vVelocity, Vector *vAngVelocity = nullptr) = 0;
	virtual float			GetFriction( void ) const = 0;
	virtual	bool 			FVisible ( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = nullptr ) = 0;
	virtual bool 			FVisible( const Vector &vecTarget, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = nullptr ) = 0;
	virtual bool 			CanBeSeenBy( CAI_BaseNPC *pNPC ) = 0;
	virtual float			GetAttackDamageScale( CBaseEntity *pVictim ) = 0;
	virtual float			GetReceivedDamageScale( CBaseEntity *pAttacker ) = 0;
	virtual void			OnGroundChanged(CBaseEntity*, CBaseEntity*) = 0;
	virtual Vector*			GetGroundVelocityToApply(Vector &) = 0;
	virtual int				PhysicsSplash(Vector const&,Vector const&,float,float) = 0;
	virtual void			Splash(void) = 0;
	virtual const Vector&	WorldSpaceCenter( ) const = 0;
	virtual Vector			GetSoundEmissionOrigin() const = 0;
	virtual bool			CreateVPhysics() = 0;
	virtual bool			ForceVPhysicsCollide( CBaseEntity *pEntity ) = 0;
	virtual void			VPhysicsDestroyObject( void ) = 0;
	virtual void			VPhysicsUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual int				VPhysicsTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual void			VPhysicsShadowCollision( int index, gamevcollisionevent_t *pEvent ) = 0;
	virtual void			VPhysicsShadowUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void			VPhysicsCollision( int index, gamevcollisionevent_t *pEvent ) = 0;
	virtual void			VPhysicsFriction( IPhysicsObject *pObject, float energy, int surfaceProps, int surfacePropsHit ) = 0;
	virtual void			UpdatePhysicsShadowToCurrentPosition( float deltaTime ) = 0;
	virtual int				VPhysicsGetObjectList( IPhysicsObject **pList, int listMax ) = 0;
	virtual bool			VPhysicsIsFlesh( void ) = 0;
	virtual int				CanPushEntity(CBaseEntity*)const = 0;
	virtual	CBasePlayer		*HasPhysicsAttacker( float dt ) = 0;
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const = 0;
protected:
	virtual void			ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity ) = 0;
private:
	virtual void			PerformCustomPhysics( Vector *pNewPosition, Vector *pNewVelocity, QAngle *pNewAngles, QAngle *pNewAngVelocity ) = 0;
	virtual	Vector			GetStepOrigin( void ) const = 0;
	virtual	QAngle			GetStepAngles( void ) const = 0;
	virtual bool 			ShouldDrawWaterImpacts() = 0;


	string_t			m_iszDamageFilterName;									// 244
	IHANDLES			m_hDamageFilter;										// 248
public:
	int					m_debugOverlays;										// 252
private:
	TimedOverlay_t*		m_pTimedOverlay;										// 256

	float				m_flSpeed;												// 260
public:
	CNetworkVar(unsigned char, m_nRenderFX);									// 264
	CNetworkVar(unsigned char,	m_nRenderMode);									// 265
	CNetworkVar(short, m_nModelIndex);											// 266
	CNetworkColor32(m_clrRender);												// 268
private:
	int					m_nSimulationTick;										// 272

	char m_nUnknown[24];

	int 				m_spawnflags;											// 300
	int					m_iEFlags;												// 304
	CNetworkVarForDerived(int, m_fFlags);										// 308

	CNetworkVar(string_t, m_iName);												// 312

	friend class CDamageModifier;
	CUtlLinkedList<CDamageModifier*,int>	m_DamageModifiers;
	IHANDLES m_pParent;  														// 356

	byte	m_nTransmitStateOwnedCounter;										// 360
public:
	CNetworkVar(unsigned char, m_iParentAttachment);							// 361
	CNetworkVar(unsigned char, m_MoveType);										// 362
	CNetworkVar(unsigned char, m_MoveCollide);									// 363
	CNetworkHandle(IBaseEntity, m_hMoveParent);									// 364
	IHANDLES m_hMoveChild;														// 368
	IHANDLES m_hMovePeer;														// 372
private:
	CNetworkVarEmbedded(ICollisionProperty, m_Collision);						// 376

	CNetworkHandle(IBaseEntity, m_hOwnerEntity);								// 468
	CNetworkHandle(IBaseEntity, m_hEffectEntity);								// 472

public:
	CNetworkVar(float, m_fadeMinDist);											// 476
	CNetworkVar(float, m_fadeMaxDist);											// 480
	CNetworkVar(float, m_flFadeScale);											// 484
private:

	CNetworkVar(int, m_CollisionGroup);											// 488
	IPhysicsObject	*m_pPhysicsObject;											// 492
	float m_flNonShadowMass;													// 496 cached mass (shadow controllers set mass to VPHYSICS_MAX_MASS, or 50000)

	CNetworkVar(float, m_flShadowCastDistance);									// 500
	float		m_flDesiredShadowCastDistance;									// 504
	int			m_iInitialTeamNum;												// 508 Team number of this entity's team read from file
	CNetworkVar(int, m_iTeamNum);												// 512 Team number of this entity's team. 

	unsigned char	m_nWaterTouch;												// 516
	unsigned char	m_nSlimeTouch;												// 517
	unsigned char	m_nWaterType;												// 518
	CNetworkVarForDerived(unsigned char, m_nWaterLevel);						// 519

	float			m_flNavIgnoreUntilTime;										// 520

	CNetworkHandleForDerived(IBaseEntity, m_hGroundEntity);						// 524

	float			m_flGroundChangeTime; 										// 528 Time that the ground entity changed

	string_t		m_ModelName;												// 532

	// Velocity of the thing we're standing on (world space)
	CNetworkVectorForDerived(m_vecBaseVelocity);								// 536

public:
	Vector			m_vecAbsVelocity;											// 548
private:

	// Local angular velocity
	QAngle			m_vecAngVelocity;											// 560

	matrix3x4_t		m_rgflCoordinateFrame;										// 572
	EHANDLE			m_pBlocker;													// 620
	float			m_flGravity;  												// 624
	CNetworkVarForDerived(float, m_flFriction);									// 628

	CNetworkVar(float, m_flElasticity);											// 632
	float 			m_flOverriddenFriction;										// 636

	// was pev->ltime
	float			m_flLocalTime;												// 640
	// local time at the beginning of this frame
	float			m_flVPhysicsUpdateLocalTime;								// 644
	// local time the movement has ended
	float			m_flMoveDoneTime;											// 648

	// A counter to help quickly build a list of potentially pushed objects for physics
	int				m_nPushEnumCount;											// 652
public:
	Vector			m_vecAbsOrigin;												// 656
private:
	CNetworkVectorForDerived(m_vecVelocity);									// 668

	CNetworkVar(unsigned char, m_iTextureFrameIndex);							// 680

	CNetworkVar(bool, m_bSimulatedEveryTick);									// 681
	CNetworkVar(bool, m_bAnimatedEveryTick);									// 682
	CNetworkVar(bool, m_bAlternateSorting);										// 683

	CNetworkVar(unsigned char, m_nMinCPULevel);									// 684
	CNetworkVar(unsigned char, m_nMaxCPULevel);									// 685
	CNetworkVar(unsigned char, m_nMinGPULevel);									// 686
	CNetworkVar(unsigned char, m_nMaxGPULevel);									// 687
	CNetworkVarForDerived(bool, m_bClientSideRagdoll);							// 688

	COutputEvent m_OnUser1;
	COutputEvent m_OnUser2;
	COutputEvent m_OnUser3;
	COutputEvent m_OnUser4;
	COutputEvent m_OnKilled;

public:
	QAngle			m_angAbsRotation;											// 812
private:

	CNetworkVector(m_vecOrigin);												// 824
	CNetworkQAngle(m_angRotation);												// 836
	CBaseHandle m_RefEHandle;													// 848
	CNetworkVectorForDerived(m_vecViewOffset);									// 852

	bool m_bIsPlayerSimulated;													// 864
	CHandle< CBasePlayer >			m_hPlayerSimulationOwner;					// 868
	int								m_fDataObjectTypes;							// 872

	UtlHashHandle_t m_ListByClass;												// 876
	IBaseEntity* m_pPrevByClass;												// 880
	IBaseEntity* m_pNextByClass;												// 884
	bool m_bNetworkQuantizeOriginAndAngles;										// 888
};

inline bool IBaseEntity::IsEFlagSet( int nEFlagMask ) const
{
	return (m_iEFlags & nEFlagMask) != 0;
}

inline int IBaseEntity::GetFlags(void) const
{
    return m_fFlags;
}

inline IBaseEntity *IBaseEntity::GetEffectEntity(void) const
{
    return m_hEffectEntity.Get();
}

template<typename R>
R* access_dynamic_cast(IBaseEntity* source, const char* target)
{
    if(!source || target == NULL)
    {
        return nullptr;
    }
    
	datamap_t *pData = source->GetDataDescMap();
	if(pData) {
		while (pData) {
			if(g_Sample.my_bStrcmp(target, pData->dataClassName)) {
				return reinterpret_cast<R*>(source);
			}
			pData = pData->baseMap;
		}
	}

    return nullptr;
}

inline const int IBaseEntity::GetHealth()
{
    return m_iHealth;
}

inline const int IBaseEntity::GetMaxHealth()
{
    return m_iMaxHealth;
}

inline int IBaseEntity::GetSpawnFlags( void ) const
{
	return m_spawnflags; 
}

inline void IBaseEntity::AddSpawnFlags( int nFlags ) 
{ 
	m_spawnflags |= nFlags; 
}
inline void IBaseEntity::RemoveSpawnFlags( int nFlags ) 
{ 
	m_spawnflags &= ~nFlags; 
}

inline void IBaseEntity::ClearSpawnFlags( void ) 
{ 
	m_spawnflags = 0; 
}

inline bool IBaseEntity::HasSpawnFlags( int nFlags ) const
{ 
	return (m_spawnflags & nFlags) != 0; 
}

inline IBaseEntity *IBaseEntity::GetParent()
{
	return m_pParent.Get();
}

inline int IBaseEntity::GetParentAttachment()
{
	return m_iParentAttachment;
}

inline IBaseEntity *IBaseEntity::FirstMoveChild()
{
    return m_hMoveChild.Get();
}

inline IBaseEntity *IBaseEntity::NextMovePeer( void )
{
	return m_hMovePeer.Get();
}

inline IServerNetworkProperty *IBaseEntity::NetworkProp()
{
    return &m_Network;
}

inline ICollisionProperty *IBaseEntity::CollisionProp()
{
    return &m_Collision;
}

inline const IServerNetworkProperty *IBaseEntity::NetworkProp() const
{
    return &m_Network;
}

inline const ICollisionProperty *IBaseEntity::CollisionProp() const
{
    return &m_Collision;
}

inline const Vector &IBaseEntity::GetLocalOrigin() const
{
    return m_vecOrigin;
}

inline const QAngle &IBaseEntity::GetLocalAngles() const
{
    return m_angRotation;
}

inline bool IBaseEntity::IsEffectActive(int nEffects)
{
    return (m_fEffects & nEffects) != 0;
}

inline IBaseEntity *IBaseEntity::GetMoveParent()
{
    return m_hMoveParent.Get();
}

inline string_t IBaseEntity::GetClassname() const
{
    return m_iClassname;
}

inline IPhysicsObject *IBaseEntity::VPhysicsGetObject() const
{
    return m_pPhysicsObject;
}

inline bool IBaseEntity::IsEFlagsSet(int nEFlags) const
{
    return (m_iEFlags & nEFlags) != 0;
}

inline void IBaseEntity::SetWaterLevel(int nLevel)
{
    m_nWaterLevel = nLevel;
}

inline int IBaseEntity::GetWaterLevel() const
{
	return m_nWaterLevel;
}

inline void IBaseEntity::AddSolidFlags(int iFlags)
{
    CollisionProp()->AddSolidFlags(iFlags);
}

inline int IBaseEntity::GetSolidFlags() const
{
    return CollisionProp()->GetSolidFlags();
}

inline bool IBaseEntity::IsSolidFlagSet(int flagMask) const
{
    return CollisionProp()->IsSolidFlagSet(flagMask);
}

inline bool IBaseEntity::IsSolid(void) const
{
    return CollisionProp()->IsSolid();
}

inline void IBaseEntity::SetSolid(SolidType_t val)
{
    CollisionProp()->SetSolid(val);
}

inline void IBaseEntity::SetSolidFlags(int flags)
{
    CollisionProp()->SetSolidFlags(flags);
}

inline void IBaseEntity::RemoveSolidFlags(int flags)
{
    CollisionProp()->RemoveSolidFlags(flags);
}

#define CHANGE_FLAG(flags, newFlags) \
    { \
        unsigned int old = flags; \
        flags = (newFlags); \
        g_CallHelper->ReportEntityFlagsChanged((CBaseEntity *)this, old, flags); \
    }

inline void IBaseEntity::AddFlag(int flags)
{
    CHANGE_FLAG(m_fFlags, m_fFlags | flags)
}

inline void IBaseEntity::RemoveFlag(int flags)
{
    CHANGE_FLAG(m_fFlags, m_fFlags & ~flags)
}

inline void IBaseEntity::ClearFlags()
{
    CHANGE_FLAG(m_fFlags, 0)
}

inline void IBaseEntity::ClearSolidFlags()
{
    CollisionProp()->ClearSolidFlags();
}

inline bool IBaseEntity::IsSimulatedEveryTick() const
{
    return m_bSimulatedEveryTick;
}

inline bool IBaseEntity::HasDataObjectType(int type) const
{
    return (m_fDataObjectTypes & (1 << type)) ? true : false;
}

inline void IBaseEntity::AddDataObjectType(int type)
{
    m_fDataObjectTypes |= (1<<type);
}

inline void IBaseEntity::RemoveDataObjectType(int type)
{
    m_fDataObjectTypes &= ~(1<<type);
}

inline float IBaseEntity::GetMoveDoneTime()
{
    return (m_flMoveDoneTime >= 0) ? m_flMoveDoneTime - m_flLocalTime : -1;
}

inline model_t *IBaseEntity::GetModel()
{
    return (model_t *)g_pModelInfo->GetModel(GetModelIndex());
}

inline int IBaseEntity::GetEFlags() const
{
    return m_iEFlags;
}

inline MoveType_t IBaseEntity::GetMoveType() const
{
    return static_cast<MoveType_t>(m_MoveType.Get());
}

inline int IBaseEntity::GetTeamNumber() const
{
    return m_iTeamNum;
}

inline ITeam *IBaseEntity::GetTeam(void) const
{
    return g_HL2->GetGlobalTeam(m_iTeamNum);
}

inline bool IBaseEntity::InSameTeam(IBaseEntity *pEnt) const
{
    if(!pEnt)
        return false;

    return (pEnt->GetTeam() == GetTeam());
}

inline bool IBaseEntity::IsInTeam(ITeam *pTeam) const
{
    return (GetTeam() == pTeam);
}

inline bool IBaseEntity::IsInAnyTeam(void) const
{
    return (GetTeam() != nullptr);
}

inline int IBaseEntity::GetCollisionGroup() const
{
    return m_CollisionGroup;
}

inline SolidType_t IBaseEntity::GetSolid() const
{
    return CollisionProp()->GetSolid();
}

inline const Vector &IBaseEntity::WorldAlignMins() const
{
    return CollisionProp()->OBBMins();
}

inline const Vector &IBaseEntity::WorldAlignMaxs() const
{
    return CollisionProp()->OBBMaxs();
}

inline const Vector &IBaseEntity::WorldAlignSize() const
{
    return CollisionProp()->OBBSize();
}

inline float IBaseEntity::BoundingRadius() const
{
    return CollisionProp()->BoundingRadius();
}

inline bool IBaseEntity::IsPointSized() const
{
    return CollisionProp()->BoundingRadius() == 0.0f;
}

inline bool IBaseEntity::IsTransparent() const
{
    return m_nRenderMode != 0;
}

inline IBaseEntity *IBaseEntity::GetOwnerEntity(void) const
{
    return m_hOwnerEntity.Get();
}

inline IBaseEntity *IBaseEntity::GetGroundEntity(void) const
{
    return m_hGroundEntity.Get();
}

inline char IBaseEntity::GetTakedamage()
{
    return m_takedamage;
}

inline const edict_t *IBaseEntity::edict() const
{
    return NetworkProp()->edict();
}

inline edict_t *IBaseEntity::edict()
{
    return NetworkProp()->edict();
}

inline int IBaseEntity::entindex()
{
    return NetworkProp()->entindex();
}

inline const float IBaseEntity::GetSimulationTime() const
{
    return m_flSimulationTime;
}

inline bool IBaseEntity::IsMarkedForDeletion( void ) 
{ 
	return (m_iEFlags & EFL_KILLME); 
}

inline void IBaseEntity::SetName( string_t newName )
{
	m_iName = newName;
}

inline string_t IBaseEntity::GetEntityName() 
{ 
	return m_iName; 
}

inline const char IBaseEntity::GetLifeState(void) const
{
    return m_lifeState;
}

inline void IBaseEntity::SetRenderFX( RenderFx_t nRenderFX )
{
	m_nRenderFX = nRenderFX;
}

inline RenderFx_t IBaseEntity::GetRenderFX() const
{
	return (RenderFx_t)m_nRenderFX.Get();
}


#define SetTouch( a ) m_pfnTouch = static_cast <void (IBaseEntity::*)(IBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (IBaseEntity::*)( IBaseEntity *pActivator, IBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (IBaseEntity::*)(IBaseEntity *)> (a)

IBaseEntity* CreateNoSpawn(const char* szName, const Vector &vecOrigion, const QAngle &vecAngles, IBaseEntity* pOwner);
IBaseEntity* CreateSpawn(const char* szName, const Vector &vecOrigion, const QAngle &vecAngles, IBaseEntity* pOwner);

#endif