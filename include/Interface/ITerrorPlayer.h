#ifndef _INCLUDE_TERROR_PLAYER_H_
#define _INCLUDE_TERROR_PLAYER_H_
#include "ICSPlayer.h"
#include "ITerrorWeapon.h"

class CBaseAnimatingOverlay;
class CTerrorPlayer;
class IBaseAbility;

class ITerrorPlayer : public ICSPlayer
{
public:
	float GetProgressBarPercent(void);
	bool IsStaggering(void);
	bool IsPlayingDeathAnim() const;
	const Vector &GetStaggerDir(void);
	float GetTimeSinceLastFiredWeapon(void);
	bool IsFiringWeapon(void);
	ITerrorPlayer* GetOtherResponsibleForMovement();
	ITerrorPlayer* GetRecentPusher();
	bool IsStill(bool val) const;
	float GetFlowDistance(int typeFlow) const;
	IBaseEntity *GetMinigun();
	ITerrorWeapon* GetActiveTerrorWeapon() const;
	void SetPushEntity(IBaseEntity* pEntity);
	void StopRevivingSomeone(bool palySound);
	void StopHealingSomeone( void );
	void ReleaseTongueVictim( void );
	void SetMainActivity(Activity activity, bool bVal = false);
	void CancelStagger( void );
	void CancelTug( void );

	void OnStaggered(IBaseEntity* pAttacker, const Vector *dist = nullptr);
	void OnPounceEnded( void );
	void OnReleasedByTongue( void );
	void OnReleasingWithTongue( void );
	void OnStopHangingFromTongue(int);

	bool IsMotionControlledXY(Activity activity);
	bool IsMotionControlledZ(Activity activity);

	void AbilityDebug(ITerrorPlayer *pVictim, const char* msg, ...);

	ITerrorPlayer* GetPounceVictim();
	ITerrorPlayer* GetTongueVictim();
	ITerrorPlayer* GetHealTarget();
	IBaseAbility* GetCustomAbility();

	static ITerrorPlayer* GetPlayerByCharacter(ITerrorPlayer* pPlayer, int charact);
public:
	virtual ~ITerrorPlayer() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual datamap_t*		GetDataDescMap(void) = 0;
	virtual bool			TestHitboxes(Ray_t const&,unsigned int,trace_t &) = 0;
	virtual	bool			ShouldCollide( int collisionGroup, int contentsMask ) const = 0;
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo ) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual int				ObjectCaps( void ) = 0;
	virtual int				GetUsePriority(CBaseEntity*) = 0;
	virtual IResponseSystem *GetResponseSystem() = 0;
	virtual bool			ShouldAttractAutoAim( CBaseEntity *pAimingEnt ) = 0;
	virtual int				OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual int				TakeHealth( float flHealth, int bitsDamageType ) = 0;
	virtual void			Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual void			ChangeTeam( int iTeamNum ) = 0;
	virtual void			Touch( CBaseEntity *pOther ) = 0;
	virtual void			Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity ) = 0;
	virtual void			ModifyOrAppendCriteria( AI_CriteriaSet& set ) = 0;
	virtual void			ModifyOrAppendDerivedCriteria(AI_CriteriaSet &) = 0;
private:
	virtual void			NetworkStateChanged_m_iMaxHealth(void) = 0;
	virtual void			NetworkStateChanged_m_iMaxHealth(void *) = 0;
public:
	virtual float			GetFriction( void ) const = 0;
	virtual void			OnGroundChanged(CBaseEntity*,CBaseEntity*) = 0;
	virtual void			VPhysicsShadowUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void 			HandleAnimEvent( animevent_t *pEvent ) = 0;
	virtual void 			Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false ) = 0;
	virtual void 			Extinguish() = 0;
	virtual float 			PlayScene(char const*,float,AI_Response *,IRecipientFilter *) = 0;
	virtual void 			OnFootstep(Vector const&,bool,bool,bool,bool) = 0;
	virtual const char 		*GetFootstepSound(char const*,bool,float,bool)const = 0;
	virtual int 			AreFootstepsAudible(void)const = 0;
	virtual int 			IsFootstepAudible(float,bool)const = 0;
	virtual bool			Weapon_CanUse( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			Weapon_Equip( CBaseCombatWeapon *pWeapon ) = 0;
	virtual	Vector			Weapon_ShootPosition( ) = 0;
	virtual int				OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual CNavArea 		*GetLastKnownArea(void)const = 0;
	virtual bool 			IsAreaTraversable(CNavArea const*)const = 0;
	virtual void 			OnNavAreaChanged(CNavArea *,CNavArea *) = 0;
	virtual void 			OnNavAreaRemoved(CNavArea *) = 0;
	virtual ZombieClassType GetClass(void)const = 0;
	virtual int 			CanBeA(ZombieClassType)const = 0;
	virtual void 			OnPursuedBy(INextBot *) = 0;
	virtual bool 			IsGhost(void)const = 0;
	virtual bool			WantsLagCompensationOnEntity( const CBasePlayer	*pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const = 0;
	virtual void 			InitialSpawn(void) = 0;
	virtual void			Cough(CBasePlayer*) = 0;
	virtual void			PreThink( void ) = 0;
	virtual void			PostThink( void ) = 0;
	virtual const char 		*GetCharacterDisplayName(void) = 0;
	virtual void			UpdateCollisionBounds(void) = 0;
	virtual bool			IsRunning(void)const  = 0;
	virtual bool 			IsAbleToAutoCenterOnLadders(void)const = 0;
	virtual void 			FlashlightTurnOn(void) = 0;
	virtual void 			FlashlightTurnOff(void) = 0;
	virtual void			DeathSound(CTakeDamageInfo const& info) = 0;
	virtual void			OnMainActivityComplete(Activity newActivity, Activity oldActivity) = 0;
	virtual void			OnMainActivityInterrupted(Activity newActivity, Activity oldActivity) = 0;
	virtual void			ImpulseCommands( void ) = 0;
	virtual void			CheatImpulseCommands( int iImpulse ) = 0;
	virtual bool			ClientCommand( const CCommand &args ) = 0;
	virtual bool			SetObserverMode(int mode) = 0;
	virtual bool			SetObserverTarget(CBaseEntity * target) = 0;
	virtual CBaseEntity		*FindNextObserverTarget( bool bReverse ) = 0;
	virtual int				PassesObserverFilter(CBaseEntity const*) = 0;
	virtual bool			IsValidObserverTarget(CBaseEntity * target) = 0;
	virtual int				WantsRoamingObserverMode(void)const = 0;
	virtual CBaseEntity		*EntSelectSpawnPoint( void ) = 0;
	virtual bool			BumpWeapon( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			ItemPostFrame( void ) = 0;
	virtual CBaseEntity		*GiveNamedItem( const char *szName, int, bool) = 0; //offset 1620
	virtual int				PlayWadeSound(void) = 0;
	virtual bool			CanBreatheUnderwater() const = 0;
	virtual int				CanRecoverCurrentDrowningDamage(void)const = 0;
	virtual void			PlayerUse( CBaseEntity* ) = 0;
	virtual CBaseEntity		*FindUseEntity( float,float,bool * ) = 0;
	virtual bool			IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps ) = 0;
	virtual void			OnUseEntity(CBaseEntity *,USE_TYPE) = 0;
	virtual void			PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize = true ) = 0;
	virtual void			ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldindThis = nullptr ) = 0;
	virtual float			GetHeldObjectMass( IPhysicsObject *pHeldObject ) = 0;
	virtual int				IsHoldingEntity(CBaseEntity *) = 0;
	virtual void			PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper) = 0;
	virtual void			CheckChatText( char *p, int bufsize ) = 0;
	virtual void			InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity ) = 0;
	virtual void 			OnUseEntityChanged(void) = 0;
	virtual unsigned int 	UpdateTonemapController(void) = 0;
	virtual int 			IsAutoCrouched(void)const = 0;
	virtual int 			GetAvailableSteadyStateSlots(void) = 0;
	virtual void 			OnSpeak(CBasePlayer*, char const*, float) = 0;
	virtual void 			OnVoiceTransmit(void) = 0;
	virtual int 			PlayerSolidMask(bool)const = 0;
	virtual bool			SpeakIfAllowed(CAI_Concept concepts, SpeechPriorityType priority, char const* modifiers, char *pszOutResponseChosen, unsigned int bufsize, IRecipientFilter *filter) = 0;
	virtual void 			OnTakeDamageInternal(CTakeDamageInfo &) = 0;
	virtual int 			AllowDamage(CTakeDamageInfo const&) = 0;
	virtual	long double 	GetHealthBuffer(void) const = 0;
	virtual	int 			IsIncapacitated(void) const = 0;
	virtual	long double 	GetAdjustedDamage(CTakeDamageInfo const&, float, float, bool) = 0;
	virtual	int 			DoBloodEffect(float, CTakeDamageInfo const&, Vector const&, trace_t *) = 0;
	virtual void 			DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 ) = 0;
	virtual void 			KickBack(float,float,float,float,float,float,int) = 0;
	virtual bool			IsImmobilized(void) const = 0;
	virtual void			GiveDefaultItems(void) = 0;
	virtual void 			RoundRespawn( void ) = 0;
	virtual void 			ObserverRoundRespawn( void ) = 0;
	virtual void 			Deafen( float flDistance , float, float) = 0;
	virtual void 			ResetMaxSpeed() = 0;
	virtual bool			HandleCommand_JoinTeam(int,char const*,bool) = 0;
	virtual void			GetIntoGame(void) = 0;
	virtual int				WantsMOTD(void) const = 0;
	virtual int				AutoSelectTeam(void) = 0;
	virtual int 			OnLeaveActiveState(void) = 0;
	virtual void 			Pain( bool HasArmour ) = 0;
	virtual int				OnTeamChanged(int) = 0;
	virtual int				CanAttack(void)const = 0;
	virtual int 			OnWeaponFired(void) = 0;
	virtual void 			OnHitPlayer(int,float,float,int) = 0;
	virtual void 			OnReloadStart(bool,int,bool) = 0;
	virtual void			CreateNoise(float) = 0;
	virtual void 			OnPreThinkObserverMode(void) = 0;
	virtual void 			OnEnterRescueState(void) = 0;
	virtual void 			OnPreThinkRescueState(void) = 0;
	virtual void 			OnLeaveRescueState(void) = 0;
	virtual void 			OnEnterGhostState(void) = 0;
	virtual void 			OnPreThinkGhostState(void) = 0;
	virtual void 			OnLeaveGhostState(void) = 0;
	virtual void 			OnEnterIntroCameraState(void) = 0;
	virtual void 			OnPreThinkIntroCameraState(void) = 0;
	virtual void 			OnLeaveIntroCameraState(void) = 0;
	virtual void 			OnLeaveDeathAnimState(void) = 0;
	virtual void 			OnLeaveDeathWaitForKeyState(void) = 0;
	virtual int				CanUseFlashlight(void) const = 0;
	virtual void 			UpdateAddonBits(void) = 0;
	virtual void 			UpdateRadar(void) = 0;
	virtual int				SelectDeathPose(CTakeDamageInfo const&) = 0;
	virtual int				CanChangeName(void)const = 0;
	virtual int				ChangeName(char const*) = 0;
	virtual bool			IsProgressBarActive(void)const = 0; // offset 2108
	virtual unsigned int	CreateRagdollEntity(CTakeDamageInfo const&) = 0;
	virtual int				GetRagdollType(void)const = 0;
	virtual int				SetModelFromClass(void) = 0;
	virtual int				SetModelFromClassAtSpawn(void) = 0;
	virtual int				DropWeapons(bool) override = 0;
	virtual void			RecordDamageTaken(char const*,int) override = 0;
	virtual void 			OnPlayerDisconnected(CTerrorPlayer*) = 0;
	virtual int 			OnSpawn(void) = 0;
	virtual int 			RestoreSpawn(void) = 0;
	virtual int 			OnBeginChangeLevel(const char *,KeyValues *) = 0;
	virtual int 			OnEndChangeLevel(void) = 0;
	virtual CTerrorPlayer	*SetDoingRestore(bool) = 0;
	virtual int 			GetTeamSwitchRule(void)const = 0;
	virtual bool 			IsReadyToShove(void) = 0;
	virtual void 			SetNextShoveTime(float) = 0;
	virtual int 			CommitSuicide(bool) = 0;
	virtual void			OnSpokeConcept(CAI_Concept, AI_Response *) = 0;
	virtual int 			RestoreWeapons(void) = 0;
	virtual CTerrorPlayer	*OnReloadEnd(void) = 0;
	virtual void 			OnReloadAbort(void) = 0;
	virtual int 			OnAttackSuccess(CBaseCombatCharacter *,bool) = 0;
	virtual bool			IsWielding(int)const = 0;
	virtual bool			IsZoomed(void) = 0;
	virtual int 			CanPlayerJump(void)const = 0;
	virtual void 			PlayerZombieAbortControl(void) = 0;
	virtual bool 			CanBeShoved(void) = 0;
	virtual int 			SetClass(ZombieClassType) = 0;
	virtual int 			SetCharacter(void) = 0;
	virtual void 			OnRevived(void) = 0; // 2224
	virtual unsigned int	OnAwardEarned(int, CBaseEntity *) = 0; // 2228
	virtual int 			OnAwardLost(int) = 0;
	virtual int 			ScoreKilledZombie(ZombieClassType) = 0;
};

class CReviveEnd : public EVENTS::CBaseEvent
{
public:
	CReviveEnd() : EVENTS::CBaseEvent("revive_end") { }
	virtual ~CReviveEnd() { }

	void Set( ITerrorPlayer* _userID, ITerrorPlayer* _subjectID, bool _ledge_hand )
	{
		if(pEvent)
		{
			pEvent->SetInt("userid", engine->GetPlayerUserId(_userID->edict()));
			pEvent->SetInt("subject", engine->GetPlayerUserId(_subjectID->edict()));
			pEvent->SetBool("ledge_hang", _ledge_hand);
		}
	}
};

class CPounceEnd : public EVENTS::CBaseEvent
{
public:
	CPounceEnd() : EVENTS::CBaseEvent("pounce_end") { }
	virtual ~CPounceEnd() { }

	void Set(ITerrorPlayer* _userID, ITerrorPlayer* _victim)
	{
		if(pEvent)
		{
			pEvent->SetInt("userid", engine->GetPlayerUserId(_userID->edict()));
			if(_victim != nullptr)
				pEvent->SetInt("victim", engine->GetPlayerUserId(_victim->edict()));
		}
	}
};

class CTongueRelease : public EVENTS::CBaseEvent
{
public:
	CTongueRelease() : EVENTS::CBaseEvent("tongue_release") { }
	virtual ~CTongueRelease() {}

	void Set(ITerrorPlayer* _userID, ITerrorPlayer* _victim, float flDistance)
	{
		if(pEvent)
		{
			pEvent->SetInt("userid", engine->GetPlayerUserId(_userID->edict()));
			pEvent->SetInt("victim", engine->GetPlayerUserId(_victim->edict()));
			pEvent->SetFloat("distance", flDistance);
		}
	}
};

class CChokeEnd : public EVENTS::CBaseEvent
{
public:
	CChokeEnd() : EVENTS::CBaseEvent("choke_end") {}
	virtual ~CChokeEnd() {}

	void Set(ITerrorPlayer* _userID, ITerrorPlayer* _victim)
	{
		if(pEvent)
		{
			if(_userID)
				pEvent->SetInt("userid", engine->GetPlayerUserId(_userID->edict()));

			pEvent->SetInt("victim", engine->GetPlayerUserId(_victim->edict()));
		}
	}
};

template <typename Func>
bool ForEachTerrorPlayer(Func &func )
{
	for(int i = 1; i <= g_pGlobals->maxClients; ++i)
	{
		IBaseEntity *pEntity = GetVirtualClass<IBaseEntity>(i);
		if(!pEntity)
			continue;

		ITerrorPlayer *client = access_dynamic_cast<ITerrorPlayer>(pEntity, "CTerrorPlayer");
		if(!client)
			continue;

		if(client->edict() == nullptr || g_Sample.IndexOfEdict(client->edict()) == 0)
			continue;

		if(client->IsPlayer())
		{
			if(client->IsConnected())
			{
				if(!func(client))
					return false;
			}
		}
	}
	return true;
}

#endif