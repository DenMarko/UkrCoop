#ifndef _INCLUDE_BASE_ANIMATING_H_
#define _INCLUDE_BASE_ANIMATING_H_

#include "IBaseEntity.h"
#include "CStudio.h"

class IBaseAnimating;
class CIKContext;
typedef struct memhandle_t__ *memhandle_t;

enum WeaponProficiency_t
{
	WEAPON_PROFICIENCY_POOR = 0,
	WEAPON_PROFICIENCY_AVERAGE,
	WEAPON_PROFICIENCY_GOOD,
	WEAPON_PROFICIENCY_VERY_GOOD,
	WEAPON_PROFICIENCY_PERFECT,
};

class CThreadFastMutex_
{
public:
	CThreadFastMutex_()
	  :	m_ownerID( 0 ),
	  	m_depth( 0 )
	{
	}

private:
	FORCEINLINE bool TryLockInline( const uint32 threadId ) volatile
	{
		if ( threadId != m_ownerID && !ThreadInterlockedAssignIf( (volatile long *)&m_ownerID, (long)threadId, 0 ) )
			return false;

		++m_depth;
		return true;
	}

	bool TryLock( const uint32 threadId ) volatile
	{
		return TryLockInline( threadId );
	}

	TT_CLASS void Lock( const uint32 threadId, unsigned nSpinSleepTime ) volatile;

public:
	bool TryLock() volatile
	{
#ifdef _DEBUG
		if ( m_depth == INT_MAX )
			DebuggerBreak();

		if ( m_depth < 0 )
			DebuggerBreak();
#endif
		return TryLockInline( ThreadGetCurrentId() );
	}

#ifndef _DEBUG
	FORCEINLINE 
#endif
	void Lock( unsigned nSpinSleepTime = 0 ) volatile
	{
		const uint32 threadId = ThreadGetCurrentId();

		if ( !TryLockInline( threadId ) )
		{
			ThreadPause();
			Lock( threadId, nSpinSleepTime );
		}
#ifdef _DEBUG
		if ( m_ownerID != ThreadGetCurrentId() )
			DebuggerBreak();

		if ( m_depth == INT_MAX )
			DebuggerBreak();

		if ( m_depth < 0 )
			DebuggerBreak();
#endif
	}

#ifndef _DEBUG
	FORCEINLINE 
#endif
	void Unlock() volatile
	{
#ifdef _DEBUG
		if ( m_ownerID != ThreadGetCurrentId() )
			DebuggerBreak();

		if ( m_depth <= 0 )
			DebuggerBreak();
#endif

		--m_depth;
		if ( !m_depth )
			ThreadInterlockedExchange( &m_ownerID, 0 );
	}

	bool TryLock() const volatile							{ return (const_cast<CThreadFastMutex_ *>(this))->TryLock(); }
	void Lock(unsigned nSpinSleepTime = 1 ) const volatile	{ (const_cast<CThreadFastMutex_ *>(this))->Lock( nSpinSleepTime ); }
	void Unlock() const	volatile							{ (const_cast<CThreadFastMutex_ *>(this))->Unlock(); }

	// To match regular CThreadMutex:
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}

	uint32 GetOwnerId() const			{ return m_ownerID;	}
	int	GetDepth() const				{ return m_depth; }
private:
	volatile uint32	m_ownerID;
	int				m_depth;
};

struct animevent_t
{
	int				event;		// 0
	const char		*options;	// 4
	float			cycle;		// 8
	float			eventtime;	// 12
	int				type;		// 16
	IBaseAnimating	*pSource;	// 20
};

class IBoneCache;

class IBaseAnimating : public IBaseEntity
{
public:
	DECLARE_CLASS( IBaseAnimating, IBaseEntity );

	enum
	{
		NUM_POSEPAREMETERS = 24,
		NUM_BONECTRLS = 4
	};


	int 					LookupAttachment(const char* szName);
	int 					LookupBone(const char *szName);
	CStudioHdr 				*GetModelPtr();
	int						LookupPoseParameter( CStudioHdr *pStudioHdr, const char *szName );
	inline int				LookupPoseParameter( const char *szName )	{ return LookupPoseParameter(GetModelPtr(), szName); }
	inline void				LockStudioHdr()								{ g_CallHelper->LockStudioHdr(this); }
	inline int				GetSequence()								{ return m_nSequence; }
	inline bool				SequenceLoops()								{ return m_bSequenceLoops; }
	inline void				SetCycle(float flCycle)						{ m_flCycle = flCycle; }
	inline float			GetCycle()									{ return m_flCycle; }
	void					SetSequence(int nSequence);
	void					ResetSequence(int nSequence);
	void					ResetSequenceInfo();
	bool					ComputeHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	bool					ComputeEntitySpaceHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	IBoneCache				*GetBoneCache();
	float					GetModelScale() const;
	int						SelectWeightedSequence ( Activity activity );
	int						SelectWeightedSequence ( Activity activity, int curSequence );
	float					SequenceDuration( CStudioHdr *pStudioHdr, int iSequence );
	inline float 			SequenceDuration( int iSequence ) { return SequenceDuration(GetModelPtr(), iSequence); }
	const float*			GetPoseParameterArray();
	bool					IsOnFire();
	LocalFlexController_t	GetNumFlexControllers(void);
	const char				*GetFlexControllerName(LocalFlexController_t iFlexController);

	void					GetBonePosition(int iBone, Vector& origin, QAngle& angles);
	bool					GetAttachment( const char *szName, Vector &absOrigin, Vector *forward, Vector *right, Vector *up );
	bool					GetAttachment( int iAttachment, Vector &absOrigin, Vector *forward, Vector *right, Vector *up );
	bool					GetAttachment( const char *szName, Vector &absOrigin, QAngle &absAngles );
	bool					GetAttachment( int iAttachment, Vector &absOrigin, QAngle &absAngles );

	float					GetSequenceCycleRate(CStudioHdr* pStudioHdr, int iSequence);
	inline float			GetSequenceCycleRate( int iSequence ) 		{ return GetSequenceCycleRate(GetModelPtr(), iSequence); }
	float					GetAnimTimeInterval( void ) const;

	bool 					Dissolve(const char *pMaterialName, float flStartTime, bool bNPCOnly = true, int nDissolveType = 0, Vector vDissolverOrigin = vec3_origin, int iMagnitude = 0);
	bool 					IsDissolving() { return ( (GetFlags() & FL_DISSOLVING) != 0 ); }
	
	float					SetPoseParameter( CStudioHdr *pStudioHdr, const char *szName, float flValue );
	inline float 			SetPoseParameter( const char *szName, float flValue ) { return SetPoseParameter( GetModelPtr(), szName, flValue ); }
	float					SetPoseParameter( CStudioHdr *pStudioHdr, int iParameter, float flValue );
	inline float 			SetPoseParameter( int iParameter, float flValue ) { return SetPoseParameter( GetModelPtr(), iParameter, flValue ); }

	float					GetPoseParameter( const char *szName );
	float					GetPoseParameter( int iParameter );

	int						FindBodygroupByName( const char *szName );
	int 					GetNumBodyGroups( void );
	int 					GetBodygroup( int iGroup );
	const char*				GetBodygroupPartName( int iGroup, int iPart );
	int						GetBodygroupCount( int iGroup );
	void					SetBodygroup( int iGroup, int iValue );
	Activity 				GetSequenceActivity( int iSequence );
	void					SetFadeDistance( float minFadeDist, float maxFadeDist );

public:
	virtual ~IBaseAnimating() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual bool			TestCollision( const Ray_t& ray, unsigned int mask, trace_t& trace ) = 0;
	virtual	bool			TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr ) = 0;
	virtual void			SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways ) = 0;
	virtual void 			Spawn() = 0;
	virtual void 			Precache() = 0;
	virtual void 			SetModel( const char *szModelName ) = 0;
	virtual void 			Activate() = 0;
	virtual int				DrawDebugTextOverlays(void) = 0;
	virtual int	 			Restore( IRestore &restore ) = 0;
	virtual void 			OnRestore() = 0;
	virtual CBaseAnimating*	GetBaseAnimating() = 0;
	virtual void			Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity ) = 0;
	virtual void			ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
	virtual void			GetVelocity(Vector *vVelocity, Vector *vAngVelocity = nullptr) = 0;
	virtual	Vector			GetStepOrigin( void ) const = 0;
	virtual	QAngle			GetStepAngles( void ) const = 0;
	virtual float			GetIdealSpeed( ) const = 0;
	virtual float			GetIdealAccel( ) const = 0;
	virtual void			StudioFrameAdvance() = 0;
	virtual int				OnSequenceSet(int) = 0;
	virtual bool			IsActivityFinished( void ) = 0;
	virtual float			GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence ) = 0;
	virtual void 			ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut ) = 0;
	virtual bool 			BecomeRagdollOnClient( const Vector &force ) = 0;
	virtual bool 			IsRagdoll() = 0;
	virtual bool 			CanBecomeRagdoll( void ) = 0;
	virtual	void 			GetSkeleton( CStudioHdr *pStudioHdr, Vector pos[], Quaternion q[], int boneMask ) = 0;
	virtual void 			GetBoneTransform( int iBone, matrix3x4_t &pBoneToWorld ) = 0;
	virtual void 			SetupBones( matrix3x4_t *pBoneToWorld, int boneMask ) = 0;
	virtual void 			CalculateIKLocks( float currentTime ) = 0;
	virtual	void 			DispatchAnimEvents ( CBaseAnimating *eventHandler ) = 0;
	virtual void 			HandleAnimEvent( animevent_t *pEvent ) = 0;
protected:
	virtual void			PopulatePoseParameters( void ) = 0;
public:
	virtual bool 			GetAttachment( int iAttachment, matrix3x4_t &attachmentToWorld ) = 0;
	virtual	void			InitBoneControllers ( void ) = 0;
	virtual	Vector 			GetGroundSpeedVelocity( void ) = 0;
	virtual bool 			IsViewModel() const = 0;
	virtual void 			Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false ) = 0;
	virtual void 			IgniteLifetime( float flFlameLifetime ) = 0;
	virtual void 			IgniteNumHitboxFires( int iNumHitBoxFires ) = 0;
	virtual void 			IgniteHitboxFireScale( float flHitboxFireScale ) = 0;
	virtual void 			Extinguish() = 0;
	virtual void			SetLightingOrigin( CBaseEntity *pLightingOrigin ) = 0;

private:
	float					m_flGroundSpeed;			// 892 computed linear movement rate for current sequence
	float					m_flLastEventCheck;			// 896 cycle index of when events were last checked

public:
	CNetworkVar(int,		m_nForceBone);				// 900
	CNetworkVector(			m_vecForce);				// 904
	CNetworkVar(int,		m_nSkin);					// 916
	CNetworkVar(int,		m_nBody);					// 920
	CNetworkVar(int,		m_nHitboxSet);				// 924
	CNetworkVar(float,		m_flModelWidthScale);		// 928
	CNetworkVar(float,		m_flPlaybackRate);			// 932

private:
	float					m_flIKGroundContactTime;	// 936
	float					m_flIKGroundMinHeight;		// 940
	float					m_flIKGroundMaxHeight;		// 944
	float					m_flEstIkFloor; 			// 948 debounced
	float					m_flEstIkOffset;			// 952

  	CIKContext				*m_pIk;						// 956
	int						m_iIKCounter;				// 960
	bool					m_bSequenceFinished;		// 964 flag set when StudioAdvanceFrame moves across a frame boundry
	bool					m_bSequenceLoops;			// 965 true if the sequence loops
	float					m_flDissolveStartTime;		// 968
	CNetworkVar(float,		m_flCycle);					// 972
	CNetworkVar(int,		m_nSequence);				// 976
	CNetworkArray(float,	m_flPoseParameter, NUM_POSEPAREMETERS);			// 980 must be private so manual mode works!
	CNetworkArray(float,	m_flEncodedController, NUM_BONECTRLS);			// 1076 bone controller setting (0..1)
	CNetworkVar(bool,		m_bClientSideAnimation);	// 1092
	CNetworkVar(bool,		m_bClientSideFrameReset);	// 1093
	CNetworkVar(int,		m_nNewSequenceParity);		// 1096
	CNetworkVar(int,		m_nResetEventsParity);		// 1100
	CNetworkVar(unsigned char, m_nMuzzleFlashParity);	// 1104
	CNetworkHandle(IBaseEntity,m_hLightingOrigin);		// 1108
	string_t 				m_iszLightingOrigin;		// 1112 for reading from the file only
	memhandle_t				m_boneCacheHandle;			// 1116
	unsigned short			m_fBoneCacheFlags;			// 1120 Used for bone cache state on model
	COutputEvent			m_OnIgnite;					// 1122
	CStudioHdr				*m_pStudioHdr;				// 1148
	CThreadFastMutex_		m_StudioHdrInitLock;		// 1152
	CThreadFastMutex_		m_BoneSetupMutex;			// 1160
};

inline bool IBaseAnimating::IsOnFire()
{
    return ((GetFlags() & FL_ONFIRE) != 0);
}

#endif