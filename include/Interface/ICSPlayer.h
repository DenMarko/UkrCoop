#ifndef _INCLUDE_CS_PLAYER_H_
#define _INCLUDE_CS_PLAYER_H_
#include "IBaseMultiplayerPlayer.h"
#include "IWeaponCSBase.h"

class ICSPlayerAnimStateHelpers
{
public:
	virtual CWeaponCSBase* CSAnim_GetActiveWeapon() = 0;
	virtual bool CSAnim_CanMove() = 0;
};

class ICSPlayer : public IBaseMultiplayerPlayer, public ICSPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS(ICSPlayer, IBaseMultiplayerPlayer);

	void State_Transition(int newState)
	{
		g_CallHelper->CCSPlayer_State_Transition(this, newState);
	}

	void SetProgressBarTime(int barTime, const char* msg);
	IWeaponCSBase* GetActiveCSWeapon( void ) const { return (IWeaponCSBase*)GetActiveWeapon(); }
	bool HasSecondaryWeapon( void ) const;

public:
	virtual ~ICSPlayer() {}

	virtual void			SetModelIndex( int index ) = 0;
	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo ) = 0;
	virtual void			Spawn( void ) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr ) = 0;
	virtual int				OnTakeDamage( const CTakeDamageInfo &info ) = 0;
	virtual void			Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual void			ChangeTeam( int iTeamNum ) = 0;
	virtual Vector			GetSoundEmissionOrigin() const = 0;
	virtual void			VPhysicsShadowUpdate( IPhysicsObject *pPhysics ) = 0;
	virtual void 			HandleAnimEvent( animevent_t *pEvent ) = 0;
	
	virtual int				GetGroundSurface(void)const = 0;
	virtual int				IsFootstepAudible(float,bool)const = 0;
	virtual bool			Weapon_CanUse( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			Weapon_Equip( CBaseCombatWeapon *pWeapon ) = 0;
	virtual	bool			Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 ) = 0;
	virtual	bool			Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon) = 0;
	virtual int				OnTakeDamage_Alive( const CTakeDamageInfo &info ) = 0;
	virtual void			CreateViewModel( int viewmodelindex = 0 ) = 0;
	virtual void			SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize ) = 0;
	virtual bool			WantsLagCompensationOnEntity( const CBasePlayer	*pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const = 0;
	virtual void			InitialSpawn( void ) = 0;
	virtual void			ShowViewPortPanel( const char * name, bool bShow = true, KeyValues *data = nullptr ) = 0;
	virtual void			PlayerDeathThink( void ) = 0;
	virtual void			PreThink( void ) = 0;
	virtual void			PostThink( void ) = 0;
	virtual void			OnDamagedByExplosion( const CTakeDamageInfo &info ) = 0;
	virtual void			RemoveAllItems( bool removeSuit ) = 0;
	virtual int				FlashlightIsOn( void ) = 0;
	virtual void			FlashlightTurnOn( void ) = 0;
	virtual void			FlashlightTurnOff( void ) = 0;
	virtual void			UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity ) = 0;
	virtual void			PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force ) = 0;
	virtual void			DeathSound( const CTakeDamageInfo &info ) = 0;
	virtual void			SetAnimation( int playerAnim ) = 0;
	virtual void			CheatImpulseCommands( int iImpulse ) = 0;
	virtual bool			ClientCommand( const CCommand &args ) = 0;
	virtual bool			SetObserverTarget(CBaseEntity * target) = 0;
	virtual int				GetNextObserverSearchStartPoint( bool bReverse ) = 0;
	virtual int				PassesObserverFilter(CBaseEntity const*) = 0;
	virtual void			ResetObserverMode() = 0;
	virtual bool			StartReplayMode( float fDelay, float fDuration, int iEntity ) = 0;
	virtual void			StopReplayMode() = 0;
	virtual CBaseEntity		*EntSelectSpawnPoint( void ) = 0;
	virtual bool			BumpWeapon( CBaseCombatWeapon *pWeapon ) = 0;
	virtual void			PlayUseDenySound() = 0;
	virtual CBaseEntity		*FindUseEntity( float,float,bool * ) = 0;
	virtual bool			IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps ) = 0;
	virtual void			PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper) = 0;
	virtual void 			ModifyOrAppendPlayerCriteria( AI_CriteriaSet& set ) = 0;
	virtual void			InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity ) = 0;
protected:
	virtual int 			SpawnArmorValue(void)const = 0;
private:
	virtual void NetworkStateChanged_m_ArmorValue(void) = 0;
	virtual void NetworkStateChanged_m_ArmorValue(void *) = 0;
public:
	virtual int				FirePlayerHurtEvent(CTakeDamageInfo const&) = 0;
	virtual	void 			OnTakeDamageInternal(CTakeDamageInfo &) = 0;
	virtual int 			AllowDamage(CTakeDamageInfo const&) = 0;
	virtual	long double 	GetHealthBuffer(void) const = 0;
	virtual	int 			IsIncapacitated(void) const = 0;
	virtual	long double 	GetAdjustedDamage(CTakeDamageInfo const&, float, float, bool) = 0;
	virtual	int 			DoBloodEffect(float, CTakeDamageInfo const&, Vector const&, trace_t *) = 0;
	virtual CBaseEntity		*GiveNamedItem( const char *pszName, int iSubType = 0 ) = 0;
	virtual bool			IsBeingGivenItem() const = 0;
	virtual void 			DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 ) = 0;
	virtual CWeaponCSBase* 	CSAnim_GetActiveWeapon() = 0;
	virtual bool 			CSAnim_CanMove() = 0;
	virtual void 			KickBack( float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change ) = 0;
	virtual bool			IsImmobilized(void) const = 0;
	virtual void			GiveDefaultItems() = 0;
	virtual void 			RoundRespawn( void ) = 0;
	virtual void 			ObserverRoundRespawn( void ) = 0;
	virtual void			Blind( float holdTime, float fadeTime, float startingAlpha = 255 ) = 0;
	virtual void 			Deafen( float flDistance , float, float) = 0;
	virtual void 			ResetMaxSpeed() = 0;
	virtual bool 			HandleCommand_JoinClass( int iClass ) = 0;
	virtual bool			HandleCommand_JoinTeam(int,char const*,bool) = 0;
	virtual void			GetIntoGame(void) = 0;
	virtual int				WantsMOTD(void) const = 0;
	virtual int				AutoSelectTeam(void) = 0;
	virtual int 			OnLeaveActiveState(void) = 0;
	virtual void 			Pain( bool HasArmour ) = 0;
	virtual int				OnTeamChanged(int) = 0;
	virtual int				CanAttack(void)const = 0;
	virtual int				OnWeaponFired(void) = 0;
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
	virtual bool			IsProgressBarActive(void)const = 0;
protected:
	virtual unsigned int	CreateRagdollEntity(CTakeDamageInfo const&) = 0;
	virtual int				GetRagdollType(void)const = 0;
	virtual int				SetModelFromClass(void) = 0;
	virtual int				SetModelFromClassAtSpawn(void) = 0;
	virtual int				DropWeapons(bool) = 0;
public:
	virtual void			RecordDamageTaken(const char *,int) = 0;
};


#endif