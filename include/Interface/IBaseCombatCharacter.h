#ifndef _INCLUDE_BASE_COMBAT_CHARACTER_H_
#define _INCLUDE_BASE_COMBAT_CHARACTER_H_

#include "IBaseFlex.h"

class impactdamagetable_t;
class fogparams_t;
class INextBot;
class CAI_Expresser;
class CNavArea;
class CBaseCombatWeapon;
class IBaseCombatWeapon;

typedef int Disposition_t;

enum LineOfSightCheckType
{
	IGNORE_NOTHING,
	IGNORE_ACTORS
};

enum Hull_t
{
	HULL_HUMAN,				// Combine, Stalker, Zombie...
	HULL_SMALL_CENTERED,	// Scanner
	HULL_WIDE_HUMAN,		// Vortigaunt
	HULL_TINY,				// Headcrab
	HULL_WIDE_SHORT,		// Bullsquid
	HULL_MEDIUM,			// Cremator
	HULL_TINY_CENTERED,		// Manhack 
	HULL_LARGE,				// Antlion Guard
	HULL_LARGE_CENTERED,	// Mortar Synth
	HULL_MEDIUM_TALL,		// Hunter
	NUM_HULLS,
	HULL_NONE				// No Hull (appears after num hulls as we don't want to count it)
};

enum ZombieClassType
{
	ZombieClassCommon = 0,
	ZombieClassSmoker,
	ZombieClassBommer,
	ZombieClassHunter,
	ZombieClassWitch,
	ZombieClassTank,
	ZombieClassNotInfected
};

struct Relationship_t
{
	EHANDLE			entity;			// Relationship to a particular entity
	Class_T			classType;		// Relationship to a class  CLASS_NONE = not class based (Def. in baseentity.h)
	Disposition_t	disposition;	// D_HT (Hate), D_FR (Fear), D_LI (Like), D_NT (Neutral)
	int				priority;		// Relative importance of this relationship (higher numbers mean more important)
};

class CAI_MoveMonitor
{
public:
	CAI_MoveMonitor()
	 : m_vMark( 0, 0, 0 ),
	   m_flMarkTolerance( NO_MARK )
	{
	}
	
	void SetMark( IBaseEntity *pEntity, float tolerance )
	{
		if ( pEntity )
		{
			m_vMark = pEntity->GetAbsOrigin();
			m_flMarkTolerance = tolerance;
		}
	}
	
	void ClearMark()
	{
	   m_flMarkTolerance = NO_MARK;
	}

	bool IsMarkSet()
	{
		return ( m_flMarkTolerance != NO_MARK );
	}

	bool TargetMoved( IBaseEntity *pEntity )
	{
		if ( IsMarkSet() && pEntity != NULL )
		{
			float distance = ( m_vMark - pEntity->GetAbsOrigin() ).Length();
			if ( distance > m_flMarkTolerance )
				return true;
		}
		return false;
	}

	bool TargetMoved2D( IBaseEntity *pEntity )
	{
		if ( IsMarkSet() && pEntity != NULL )
		{
			float distance = ( m_vMark.AsVector2D() - pEntity->GetAbsOrigin().AsVector2D() ).Length();
			if ( distance > m_flMarkTolerance )
				return true;
		}
		return false;
	}

	Vector GetMarkPos() { return m_vMark; }
	
private:
	enum
	{
		NO_MARK = -1
	};
	
	Vector			   m_vMark;
	float			   m_flMarkTolerance;

	DECLARE_SIMPLE_DATADESC();
};

class IBaseCombatCharacter : public IBaseFlex
{
	DECLARE_CLASS( IBaseCombatCharacter, IBaseFlex );
public:
	IBaseCombatWeapon *GetActiveWeapon() const;
	IBaseCombatWeapon *GetWeapon(int i) const;
	int GiveAmmo(int iCount, const char* szName, bool bSuppressSound = false);
	int	LastHitGroup() const;
	void SetImpactEnergyScale( float fScale ) { m_impactEnergyScale = fScale; }
	void GiveAmmoBox(int iCount, int iAmmoIndex);
	
public:
	virtual ~IBaseCombatCharacter() { }

	virtual ServerClass					*GetServerClass(void) = 0;
	virtual int							YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t					*GetDataDescMap(void) = 0;
	virtual void						SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways ) = 0;
	virtual void 						Spawn() = 0;
	virtual void 						Precache() = 0;
	virtual int							Restore( IRestore &restore ) = 0;
	virtual int							OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual int							TakeHealth( float flHealth, int bitsDamageType ) = 0;
	virtual void						Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual int							BloodColor( void ) = 0;
	virtual CBaseCombatCharacter 		*MyCombatCharacterPointer( void ) = 0;
	virtual void						ChangeTeam( int iTeamNum ) = 0;
	virtual void						UpdateOnRemove( void ) = 0;
	virtual	bool 						FVisible ( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = nullptr ) = 0;
	virtual bool 						FVisible( const Vector &vecTarget, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = nullptr ) = 0;
	virtual void						VPhysicsUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void						VPhysicsShadowCollision( int index, gamevcollisionevent_t *pEvent ) = 0;

	virtual const impactdamagetable_t	&GetPhysicsImpactDamageTable( void ) = 0;
	virtual bool						FInViewCone( CBaseEntity *pEntity ) = 0;
	virtual bool						FInViewCone( const Vector &vecSpot ) = 0;
	virtual bool						FInAimCone( CBaseEntity *pEntity ) = 0;
	virtual bool						FInAimCone( const Vector &vecSpot ) = 0;
	virtual bool						ShouldShootMissTarget( CBaseCombatCharacter *pAttacker ) = 0;
	virtual CBaseEntity 				*FindMissTarget( void ) = 0;
	virtual bool						HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt ) = 0;

	virtual QAngle						BodyAngles() = 0;
	virtual Vector						BodyDirection2D( void ) = 0;
	virtual Vector						BodyDirection3D( void ) = 0;
	virtual Vector						HeadDirection2D( void ) = 0;
	virtual Vector						HeadDirection3D( void ) = 0;
	virtual Vector						EyeDirection2D( void ) = 0;
	virtual Vector						EyeDirection3D( void ) = 0;

	virtual bool             			IsHiddenByFog(const Vector &target)const = 0;
	virtual bool             			IsHiddenByFog(CBaseEntity *target)const = 0;
	virtual bool            			IsHiddenByFog(float range)const = 0;
	virtual float             			GetFogObscuredRatio(const Vector &target)const = 0;
	virtual float		     			GetFogObscuredRatio(CBaseEntity *target)const = 0;
	virtual float		     			GetFogObscuredRatio(float range)const = 0;
	virtual bool             			GetFogParams(fogparams_t *fog)const = 0;
	virtual bool            			IsLookingTowards(CBaseEntity const*,float)const = 0;
	virtual bool            			IsLookingTowards(Vector const&,float)const = 0;
	virtual bool            			IsInFieldOfView(CBaseEntity *entity)const = 0;
	virtual bool            			IsInFieldOfView(const Vector &pos)const = 0;
	virtual bool            			IsLineOfSightClear(CBaseEntity *entity, LineOfSightCheckType checkType = IGNORE_NOTHING)const = 0;
	virtual bool            			IsLineOfSightClear(const Vector &pos, LineOfSightCheckType checkType = IGNORE_NOTHING, CBaseEntity *entityToIgnore = nullptr)const = 0;
	virtual void            			OnFootstep(Vector const&,bool,bool,bool,bool) = 0;
	virtual int             			GetGroundSurface(void)const = 0;
	virtual const char      			*GetFootstepSound(char const*,bool,float,bool)const = 0;
	virtual int             			AreFootstepsAudible(void)const = 0;
	virtual int             			IsFootstepAudible(float,bool)const = 0;
	virtual float		     			GetFootstepRunThreshold(void)const = 0;

	virtual int							GiveAmmo( int iCount, int iAmmoIndex, bool bSuppressSound = false ) = 0;
	CBaseCombatWeapon*					Weapon_Create( const char *pWeaponName );
	virtual Activity					NPC_TranslateActivity( Activity baseAct ) = 0;
	virtual Activity					Weapon_TranslateActivity( Activity baseAct, bool *pRequired = nullptr ) = 0;
	virtual void						Weapon_FrameUpdate( void ) = 0;
	virtual void						Weapon_HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual bool						Weapon_CanUse( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void						Weapon_Equip( CBaseCombatWeapon *pWeapon ) = 0;
	virtual bool						Weapon_EquipAmmoOnly( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void						Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = nullptr, const Vector *pVelocity = nullptr ) = 0;
	virtual	bool						Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 ) = 0;
	virtual	Vector						Weapon_ShootPosition( ) = 0;
	virtual	bool						Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon) = 0;
	virtual bool						Weapon_SlotOccupied( CBaseCombatWeapon *pWeapon ) = 0;
	virtual CBaseCombatWeapon 			*Weapon_GetSlot( int slot ) const = 0;
	virtual bool						AddPlayerItem( CBaseCombatWeapon *pItem ) = 0;
	virtual bool						RemovePlayerItem( CBaseCombatWeapon *pItem ) = 0;

	virtual bool						CanBecomeServerRagdoll( void ) = 0;
	virtual int							OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual int							OnTakeDamage_Dying( const CTakeDamageInfo &info ) = 0;
	virtual int							OnTakeDamage_Dead( const CTakeDamageInfo &info ) = 0;
	virtual float						GetAliveDuration(void)const = 0;
	virtual void 						OnFriendDamaged( CBaseCombatCharacter *pSquadmate, CBaseEntity *pAttacker ) = 0;
	virtual void 						NotifyFriendsOfDamage( CBaseEntity *pAttackerEntity ) = 0;
	virtual bool						HasEverBeenInjured(int) const = 0;
	virtual float						GetTimeSinceLastInjury(int) const = 0;
	virtual void						OnPlayerKilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual Activity					GetDeathActivity( void ) = 0;
	virtual bool						CorpseGib( const CTakeDamageInfo &info ) = 0;
	virtual void						CorpseFade( void ) = 0;
	virtual bool						HasHumanGibs( void ) = 0;
	virtual bool						HasAlienGibs( void ) = 0;
	virtual bool						ShouldGib( const CTakeDamageInfo &info ) = 0;
	virtual void 						OnKilledNPC( CBaseCombatCharacter *pKilled ) = 0;
	virtual bool						Event_Gibbed( const CTakeDamageInfo &info ) = 0;
	virtual void						Event_Dying( void ) = 0;
	virtual bool						BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector ) = 0;
	virtual void						FixupBurningServerRagdoll( CBaseEntity *pRagdoll ) = 0;
	virtual bool						BecomeRagdollBoogie( CBaseEntity *pKiller, const Vector &forceVector, float duration, int flags ) = 0;
	virtual CBaseEntity					*CheckTraceHullAttack( float flDist, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float forceScale = 1.0f, bool bDamageAnyNPC = false ) = 0;
	virtual CBaseEntity					*CheckTraceHullAttack( const Vector &vStart, const Vector &vEnd, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float flForceScale = 1.0f, bool bDamageAnyNPC = false ) = 0;
	virtual void						PushawayTouch( CBaseEntity *pOther ) = 0;
	virtual int							IRelationType( CBaseEntity *pTarget ) = 0;
	virtual int							IRelationPriority( CBaseEntity *pTarget ) = 0;
	virtual bool 						IsInAVehicle( void ) const = 0;
	virtual IServerVehicle 				*GetVehicle( void ) = 0;
	virtual CBaseEntity 				*GetVehicleEntity( void ) = 0;
	virtual bool 						ExitVehicle( void ) = 0;
	virtual int 						CalcWeaponProficiency( CBaseCombatWeapon *pWeapon ) = 0;
	virtual	Vector						GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = nullptr ) = 0;
	virtual	float						GetSpreadBias(  CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget ) = 0;
	virtual void						DoMuzzleFlash() = 0;
	virtual void						AddEntityRelationship( CBaseEntity *pEntity, Disposition_t nDisposition, int nPriority ) = 0;
	virtual bool						RemoveEntityRelationship( CBaseEntity *pEntity ) = 0;
	virtual void						AddClassRelationship( Class_T nClass, Disposition_t nDisposition, int nPriority ) = 0;

	virtual void            			OnChangeActiveWeapon(CBaseCombatWeapon *, CBaseCombatWeapon *) = 0;
	virtual CNavArea 					*GetLastKnownArea(void)const = 0;
	virtual bool            			IsAreaTraversable(CNavArea const*)const = 0;
	virtual int             			ClearLastKnownArea(void) = 0;
	virtual void            			UpdateLastKnownArea(void) = 0;
	virtual void            			OnNavAreaChanged(CNavArea *,CNavArea *) = 0;
	virtual void            			OnNavAreaRemoved(CNavArea *) = 0;
	virtual ZombieClassType 			GetClass(void)const = 0;
	virtual int             			CanBeA(ZombieClassType eClass)const = 0;
	virtual void            			OnPursuedBy(INextBot *) = 0;
	virtual bool             			IsGhost(void)const = 0;

private:
	bool				m_bForceServerRagdoll;						// 1668
	bool				m_bPreventWeaponPickup;						// 1669
	
public:
	CNetworkVar( float, m_flNextAttack );							// 1672
private:
	Hull_t				m_eHull;									// 1676
	int					m_bloodColor;								// 1680
	float				m_flFieldOfView;							// 1684
	Vector				m_HackedGunPos;								// 1688
	string_t			m_RelationshipString;						// 1700
	float				m_impactEnergyScale;						// 1704
	byte				m_weaponIDToIndex[30];						// 1708
	int					m_LastHitGroup;								// 1740
	float				m_flDamageAccumulator;						// 1744
	int					m_iDamageCount;								// 1748
	WeaponProficiency_t m_CurrentWeaponProficiency;
	CUtlVector<Relationship_t>		m_Relationship;					// Array of relationships
	CUtlVector<EHANDLE> m_hTriggerFogList;
	EHANDLE 			m_hLastFogTrigger;
	CNetworkArrayForDerived(int, m_iAmmo, MAX_AMMO_SLOTS);
	CNetworkArray(CHandle<IBaseCombatWeapon>, m_hMyWeapons, 48);	// 1928
	CNetworkHandle(IBaseCombatWeapon, m_hActiveWeapon);
	IntervalTimers 		m_aliveTimer;								// 2124
	unsigned int 		m_hasBeenInjured;							// 2132	

	enum { MAX_DAMAGE_TEAMS = 4 };
	struct DamageHistory
	{
		int team;
		IntervalTimers interval;
	};
	DamageHistory m_damageHistory[ MAX_DAMAGE_TEAMS ];				// 2136
	CNavArea*			m_lastNavArea;								// 2184
	CAI_MoveMonitor 	m_NavAreaUpdateMonitor;						// 2188
	int 				m_registeredNavTeam;						// 2204

	CountdownTimers		m_time_Unknown;								// 2208
	int					m_i_Unknown;								// 2220
};

inline int IBaseCombatCharacter::LastHitGroup() const
{
	return m_LastHitGroup;
}

#endif