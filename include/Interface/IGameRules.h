#ifndef _INCLUDE_GAME_RULES_
#define _INCLUDE_GAME_RULES_
#include "ITerrorPlayer.h"

class CUserCmd;

class IGameSystem
{
public:
	virtual char const *Name() = 0;

	virtual bool Init() = 0;
	virtual void PostInit() = 0;
	virtual void Shutdown() = 0;
	virtual void LevelInitPreEntity() = 0;
	virtual void LevelInitPostEntity() = 0;
	virtual void LevelShutdownPreEntity() = 0;
	virtual void LevelShutdownPostEntity() = 0;
	virtual void OnSave() = 0;
	virtual void OnRestore() = 0;
	virtual void SafeRemoveIfDesired() = 0;
	virtual bool IsPerFrame() = 0;

	virtual ~IGameSystem() {}
};

class IGameSystemPerFrame : public IGameSystem
{
public:
	virtual ~IGameSystemPerFrame() {}

	virtual void 		FrameUpdatePreEntityThink() = 0;
	virtual void 		FrameUpdatePostEntityThink() = 0;
	virtual void 		PreClientUpdate() = 0;
};

class CBaseGameSystemPerFrame : public IGameSystemPerFrame
{
public:
	virtual char const *Name() = 0;

	virtual bool Init() = 0;
	virtual void PostInit() = 0;
	virtual void Shutdown() = 0;
	virtual void LevelInitPreEntity() = 0;
	virtual void LevelInitPostEntity() = 0;
	virtual void LevelShutdownPreEntity() = 0;
	virtual void LevelShutdownPostEntity() = 0;
	virtual void OnSave() = 0;
	virtual void OnRestore() = 0;
	virtual void SafeRemoveIfDesired() = 0;
	virtual bool IsPerFrame() = 0;
	virtual ~CBaseGameSystemPerFrame() {}
	virtual void FrameUpdatePreEntityThink() = 0;
	virtual void FrameUpdatePostEntityThink() = 0;
	virtual void PreClientUpdate() = 0;
};

class CBaseGameSystem : public IGameSystem
{
public:

	virtual char const *Name() = 0;

	virtual bool Init() = 0;
	virtual void PostInit() = 0;
	virtual void Shutdown() = 0;

	virtual void LevelInitPreEntity() = 0;
	virtual void LevelInitPostEntity() = 0;
	virtual void LevelShutdownPreEntity() = 0;
	virtual void LevelShutdownPostEntity() = 0;

	virtual void OnSave() = 0;
	virtual void OnRestore() = 0;
	virtual void SafeRemoveIfDesired() = 0;

	virtual bool IsPerFrame() = 0;
private:

	virtual void FrameUpdatePreEntityThink() = 0;
	virtual void FrameUpdatePostEntityThink() = 0;
	virtual void PreClientUpdate() = 0;
};

class IAutoGameSystem : public CBaseGameSystem
{
public:
	IAutoGameSystem *m_pNext;
	virtual char const *Name() = 0;
private:
	char const *m_pszName;
};

class CAutoGameSystemPerFrame : public CBaseGameSystemPerFrame
{
public:
	CAutoGameSystemPerFrame( char const *name = NULL );
	CAutoGameSystemPerFrame *m_pNext;

	virtual char const *Name() { return m_pszName ? m_pszName : "unnamed"; }
	
private:
	char const *m_pszName;
};

class CViewVectors;
class CItem;

class IGameRules : public CAutoGameSystemPerFrame
{
public:
	virtual char const *Name() = 0;
	
	virtual ~IGameRules() {}

	virtual bool		Damage_IsTimeBased( int iDmgType ) = 0;
	virtual bool		Damage_ShouldGibCorpse( int iDmgType ) = 0;
	virtual bool		Damage_ShowOnHUD( int iDmgType ) = 0;
	virtual bool		Damage_NoPhysicsForce( int iDmgType ) = 0;
	virtual bool		Damage_ShouldNotBleed( int iDmgType ) = 0;
	virtual int			Damage_GetTimeBased( void ) = 0;
	virtual int			Damage_GetShouldGibCorpse( void ) = 0;
	virtual int			Damage_GetShowOnHud( void ) = 0;					
	virtual int			Damage_GetNoPhysicsForce( void )= 0;
	virtual int			Damage_GetShouldNotBleed( void ) = 0;

	virtual void 		FrameUpdatePostEntityThink() = 0;
	virtual bool 		SwitchToNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon ) = 0;
	virtual CBaseCombatWeapon *GetNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon ) = 0;
	virtual bool 		ShouldCollide( int collisionGroup0, int collisionGroup1 ) = 0;

	virtual int 		DefaultFOV( void ) = 0;
	virtual const CViewVectors* GetViewVectors() const = 0;
	virtual float 		GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType ) = 0;
    virtual float 		GetDamageMultiplier( void ) = 0;    
	virtual bool		IsMultiplayer( void ) = 0;
	virtual const unsigned char *GetEncryptionKey() = 0;
	virtual bool 		InRoundRestart( void ) = 0;
	virtual void 		GetTaggedConVarList(KeyValues *) = 0;
	virtual void 		CheckHaptics(CBasePlayer *) = 0;
	virtual void 		OnBeginChangeLevel(char const*,KeyValues *) = 0;
	virtual void 		LevelShutdown( void ) = 0;
	virtual void 		Precache( void ) = 0;
	virtual void 		RefreshSkillData( bool forceUpdate ) = 0;
	virtual void 		Think( void ) = 0;
	virtual bool 		IsAllowedToSpawn( CBaseEntity *pEntity ) = 0;
	virtual void 		EndGameFrame( void ) = 0;
	virtual bool 		IsSkillLevel( int iLevel ) = 0;
	virtual int			GetSkillLevel() = 0;
	virtual void 		OnSkillLevelChanged( int iNewLevel ) = 0;
	virtual void 		SetSkillLevel( int iLevel ) = 0;
	virtual bool 		FAllowFlashlight( void ) = 0;
	virtual bool 		FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ) = 0;
	virtual bool 		IsDeathmatch( void ) = 0;
	virtual bool 		IsTeamplay( void ) = 0;
	virtual bool 		IsCoOp( void ) = 0;
	virtual const char 	*GetGameDescription( void ) = 0;
	virtual bool 		ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) = 0;
	virtual void 		InitHUD( CBasePlayer *pl ) = 0;
	virtual void 		ClientDisconnected( edict_t *pClient ) = 0;
	virtual float 		FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;
	virtual bool 		FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) = 0;
	virtual bool 		ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target ) = 0;
	virtual float 		GetAutoAimScale( CBasePlayer *pPlayer ) = 0;
	virtual int			GetAutoAimMode() = 0;
	virtual bool 		ShouldUseRobustRadiusDamage(CBaseEntity *pEntity) = 0;
	virtual void 		RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore ) = 0;
	virtual bool		FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) = 0;
	virtual bool 		AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual void 		PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	virtual void 		PlayerThink( CBasePlayer *pPlayer ) = 0;
	virtual bool 		FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;
	virtual float 		FlPlayerSpawnTime( CBasePlayer *pPlayer ) = 0;
	virtual CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer ) = 0;
	virtual bool		IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer ) = 0;
	virtual bool 		AllowAutoTargetCrosshair( void ) = 0;
	virtual bool 		ClientCommand( CBaseEntity *pEdict, const CCommand &args ) = 0;
	virtual void 		ClientSettingsChanged( CBasePlayer *pPlayer ) = 0;
	virtual int 		IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;
	virtual void 		PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual void 		DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )=  0;
	virtual const char 	*GetDamageCustomString( const CTakeDamageInfo &info ) = 0;
	virtual float		AdjustPlayerDamageInflicted( float damage ) = 0;
	virtual void		AdjustPlayerDamageTaken( CTakeDamageInfo *pInfo ) = 0;
	virtual bool		CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ) = 0;
	virtual int 		WeaponShouldRespawn( CBaseCombatWeapon *pWeapon ) = 0;
	virtual float 		FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon ) = 0;
	virtual float 		FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon ) = 0;
	virtual Vector 		VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon ) = 0;
	virtual bool 		CanHaveItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;
	virtual void 		PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;
	virtual int 		ItemShouldRespawn( CItem *pItem ) = 0;
	virtual float 		FlItemRespawnTime( CItem *pItem ) = 0;
	virtual Vector 		VecItemRespawnSpot( CItem *pItem ) = 0;
	virtual QAngle 		VecItemRespawnAngles( CItem *pItem ) = 0;
	virtual bool		CanHaveAmmo( CBaseCombatCharacter *pPlayer, int iAmmoIndex ) = 0;
	virtual bool		CanHaveAmmo( CBaseCombatCharacter *pPlayer, const char *szName ) = 0;
	virtual void 		PlayerGotAmmo( CBaseCombatCharacter *pPlayer, char *szName, int iCount ) = 0;
	virtual float 		GetAmmoQuantityScale( int iAmmoIndex ) = 0;
	virtual void		InitDefaultAIRelationships( void ) = 0;
	virtual const char	*AIClassText(int classType) = 0;
	virtual float 		FlHealthChargerRechargeTime( void ) = 0;
	virtual float		FlHEVChargerRechargeTime( void ) = 0;
	virtual int 		DeadPlayerWeapons( CBasePlayer *pPlayer ) = 0;
	virtual int 		DeadPlayerAmmo( CBasePlayer *pPlayer ) = 0;
	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;
	virtual int 		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget ) = 0;
	virtual bool 		PlayerCanHearChat( CBasePlayer *pListener, CBasePlayer *pSpeaker ) = 0;
	virtual void 		CheckChatText( CBasePlayer *pPlayer, char *pText ) = 0;
	virtual int 		GetTeamIndex( const char *pTeamName ) = 0;
	virtual const char 	*GetIndexedTeamName( int teamIndex ) = 0;
	virtual bool		IsValidTeam( const char *pTeamName ) = 0;
	virtual void 		ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, bool bKill, bool bGib ) = 0;
	virtual const char 	*SetDefaultPlayerTeam( CBasePlayer *pPlayer ) = 0;
	virtual void 		UpdateClientData( CBasePlayer *pPlayer ) = 0;
	virtual bool 		PlayTextureSounds( void ) = 0;
	virtual bool 		PlayFootstepSounds( CBasePlayer *pl ) = 0;
	virtual bool 		FAllowNPCs( void ) = 0;
	virtual void 		EndMultiplayerGame( void ) = 0;
	virtual float 		WeaponTraceEntity( CBaseEntity *pEntity, const Vector &vecStart, const Vector &vecEnd, unsigned int mask, trace_t *ptr ) = 0;
	virtual void 		CreateStandardEntities() = 0;
	virtual const char 	*GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual const char 	*GetChatLocation( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual const char 	*GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual bool 		ShouldBurningPropsEmitLight() = 0;
	virtual bool 		CanEntityBeUsePushed( CBaseEntity *pEnt ) = 0;
	virtual void 		CreateCustomNetworkStringTables( void ) = 0;
	virtual void 		MarkAchievement ( IRecipientFilter& filter, char const *pchAchievementName ) = 0;
	virtual void 		ResetMapCycleTimeStamp( void ) = 0;
	virtual void 		UpdateGameplayStatsFromSteam(void) = 0;
	virtual const char 	*GetGameTypeName( void ) = 0;
	virtual int 		GetGameType( void ) = 0;
	virtual int 		ForceSplitScreenPlayersOnToSameTeam(void) = 0;
	virtual int 		GetMaxHumanPlayers(void)const = 0;
};

typedef struct
{
	int	 m_iConcept;
	bool m_bShowSubtitle;
	bool m_bDistanceBasedSubtitle;
	char m_szGestureActivity[64];
} VoiceCommandMenuItem_t;

class CBaseMultiplayerPlayer;

class IMultiplayRules : public IGameRules
{
public:
	virtual bool		Init() = 0;

	virtual ~IMultiplayRules() {}

	virtual bool		Damage_IsTimeBased( int iDmgType ) = 0;
	virtual bool		Damage_ShouldGibCorpse( int iDmgType ) = 0;
	virtual bool		Damage_ShowOnHUD( int iDmgType ) = 0;
	virtual bool		Damage_NoPhysicsForce( int iDmgType ) = 0;
	virtual bool		Damage_ShouldNotBleed( int iDmgType ) = 0;
	virtual int			Damage_GetTimeBased( void ) = 0;
	virtual int			Damage_GetShouldGibCorpse( void ) = 0;
	virtual int			Damage_GetShowOnHud( void ) = 0;					
	virtual int			Damage_GetNoPhysicsForce( void )= 0;
	virtual int			Damage_GetShouldNotBleed( void ) = 0;
	virtual bool 		SwitchToNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon ) = 0;
	virtual CBaseCombatWeapon *GetNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon ) = 0;
	virtual bool		IsMultiplayer( void ) = 0;
	virtual void 		RefreshSkillData( bool forceUpdate ) = 0;
	virtual void 		Think( void ) = 0;
	virtual bool 		IsAllowedToSpawn( CBaseEntity *pEntity ) = 0;
	virtual bool 		FAllowFlashlight( void ) = 0;
	virtual bool 		FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ) = 0;
	virtual bool 		IsDeathmatch( void ) = 0;
	virtual bool 		IsCoOp( void ) = 0;
	virtual bool 		ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) = 0;
	virtual void 		InitHUD( CBasePlayer *pl ) = 0;
	virtual void 		ClientDisconnected( edict_t *pClient ) = 0;
	virtual float 		FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;
	virtual bool 		FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) = 0;
	virtual bool 		AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual void 		PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	virtual void 		PlayerThink( CBasePlayer *pPlayer ) = 0;
	virtual bool 		FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;
	virtual float 		FlPlayerSpawnTime( CBasePlayer *pPlayer ) = 0;
	virtual CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer ) = 0;
	virtual bool 		AllowAutoTargetCrosshair( void ) = 0;
	virtual bool 		ClientCommand( CBaseEntity *pEdict, const CCommand &args ) = 0;
	virtual void 		ClientSettingsChanged( CBasePlayer *pPlayer ) = 0;
	virtual int 		IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;
	virtual void 		PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual void 		DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )=  0;
	virtual bool		CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ) = 0;
	virtual int 		WeaponShouldRespawn( CBaseCombatWeapon *pWeapon ) = 0;
	virtual float 		FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon ) = 0;
	virtual float 		FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon ) = 0;
	virtual Vector 		VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon ) = 0;
	virtual bool 		CanHaveItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;
	virtual void 		PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;
	virtual int 		ItemShouldRespawn( CItem *pItem ) = 0;
	virtual float 		FlItemRespawnTime( CItem *pItem ) = 0;
	virtual Vector 		VecItemRespawnSpot( CItem *pItem ) = 0;
	virtual QAngle 		VecItemRespawnAngles( CItem *pItem ) = 0;
	virtual void 		PlayerGotAmmo( CBaseCombatCharacter *pPlayer, char *szName, int iCount ) = 0;
	virtual float 		FlHealthChargerRechargeTime( void ) = 0;
	virtual float		FlHEVChargerRechargeTime( void ) = 0;
	virtual int 		DeadPlayerWeapons( CBasePlayer *pPlayer ) = 0;
	virtual int 		DeadPlayerAmmo( CBasePlayer *pPlayer ) = 0;
	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;
	virtual int 		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget ) = 0;
	virtual bool 		PlayerCanHearChat( CBasePlayer *pListener, CBasePlayer *pSpeaker ) = 0;
	virtual bool 		PlayTextureSounds( void ) = 0;
	virtual bool 		PlayFootstepSounds( CBasePlayer *pl ) = 0;
	virtual bool 		FAllowNPCs( void ) = 0;
	virtual void 		EndMultiplayerGame( void ) = 0;
	virtual void 		ResetMapCycleTimeStamp( void ) = 0;
	virtual CBasePlayer *GetDeathScorer( CBaseEntity *pKiller, CBaseEntity *pInflictor, CBaseEntity *pVictim ) = 0;
	virtual VoiceCommandMenuItem_t *VoiceCommand( CBaseMultiplayerPlayer *pPlayer, int iMenu, int iItem ) = 0;
	virtual void		InitCustomResponseRulesDicts( void ) = 0;
	virtual void		ShutdownCustomResponseRulesDicts( void ) = 0;
	virtual bool		UseSuicidePenalty( void ) = 0;
	virtual void		GetNextLevelName( char *szNextMap, int bufsize ) = 0;
	virtual void		ChangeLevel( void ) = 0;
	virtual void		GoToIntermission( void ) = 0;
};

class ITemplayRules : public IMultiplayRules
{
public:
	virtual ~ITemplayRules() {}

	virtual void 		Precache( void ) = 0;
	virtual void 		Think( void ) = 0;
	virtual bool 		IsTeamplay( void ) = 0;
	virtual const char*	GetGameDescription( void ) = 0;
	virtual void 		InitHUD( CBasePlayer *pl ) = 0;
	virtual void 		ClientDisconnected( edict_t *pClient ) = 0;
	virtual bool 		FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) = 0;
	virtual bool 		ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target ) = 0;
	virtual bool 		ClientCommand( CBaseEntity *pEdict, const CCommand &args ) = 0;
	virtual void 		ClientSettingsChanged( CBasePlayer *pPlayer ) = 0;
	virtual int 		IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;
	virtual void 		PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual void 		DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )=  0;
	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;
	virtual int 		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget ) = 0;
	virtual bool 		PlayerCanHearChat( CBasePlayer *pListener, CBasePlayer *pSpeaker ) = 0;
	virtual int 		GetTeamIndex( const char *pTeamName ) = 0;
	virtual const char 	*GetIndexedTeamName( int teamIndex ) = 0;
	virtual bool		IsValidTeam( const char *pTeamName ) = 0;
	virtual void 		ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, bool bKill, bool bGib ) = 0;
	virtual const char 	*SetDefaultPlayerTeam( CBasePlayer *pPlayer ) = 0;
	virtual int			GetCaptureValueForPlayer( CBasePlayer *pPlayer ) = 0;
	virtual bool		TeamMayCapturePoint( int iTeam, int iPointIndex ) = 0;
	virtual bool		PlayerMayCapturePoint( CBasePlayer *pPlayer, int iPointIndex, char *pszReason = NULL, int iMaxReasonLength = 0 ) = 0;
	virtual bool		PlayerMayBlockPoint( CBasePlayer *pPlayer, int iPointIndex, char *pszReason = NULL, int iMaxReasonLength = 0 ) = 0;
	virtual bool		PointsMayBeCaptured( void ) = 0;
	virtual bool		TimerMayExpire( void ) = 0;
	virtual void		SetWinningTeam( int team, int iWinReason, bool bForceMapReset = true, bool bSwitchTeams = false ) = 0;
	virtual void		SetStalemate( int iReason, bool bForceMapReset = true, bool bSwitchTeams = false ) = 0;
	virtual void		SetSwitchTeams( bool bSwitch ) = 0;
	virtual bool		ShouldSwitchTeams( void ) = 0;
	virtual void		HandleSwitchTeams( void ) = 0;
	virtual void		SetScrambleTeams( bool bScramble ) = 0;
	virtual bool		ShouldScrambleTeams( void ) = 0;
	virtual void		HandleScrambleTeams( void ) = 0;
};

class ICSGameRules : public ITemplayRules
{
public:
	virtual void LevelInitPreEntity() = 0;
	virtual void LevelInitPostEntity() = 0;

	virtual ~ICSGameRules() {}

	virtual CBaseCombatWeapon *GetNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon ) = 0;
	virtual bool 		ShouldCollide( int collisionGroup0, int collisionGroup1 ) = 0;
	virtual int 		DefaultFOV( void ) = 0;
	virtual const CViewVectors* GetViewVectors() const = 0;
	virtual const unsigned char *GetEncryptionKey() = 0;
	virtual void 		LevelShutdown( void ) = 0;
	virtual void 		Think( void ) = 0;
	virtual void 		EndGameFrame( void ) = 0;
	virtual bool 		FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ) = 0;
	virtual const char 	*GetGameDescription( void ) = 0;
	virtual void 		ClientDisconnected( edict_t *pClient ) = 0;
	virtual float 		FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;
	virtual void 		RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore ) = 0;
	virtual bool		FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) = 0;
	virtual void 		PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	virtual bool 		FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;
	virtual CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer ) = 0;
	virtual bool		IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer ) = 0;
	virtual bool 		ClientCommand( CBaseEntity *pEdict, const CCommand &args ) = 0;
	virtual void 		ClientSettingsChanged( CBasePlayer *pPlayer ) = 0;
	virtual void 		PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual void 		DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )=  0;
	virtual void		InitDefaultAIRelationships( void ) = 0;
	virtual const char	*AIClassText(int classType) = 0;
	virtual const char 	*SetDefaultPlayerTeam( CBasePlayer *pPlayer ) = 0;
	virtual void 		UpdateClientData( CBasePlayer *pPlayer ) = 0;
	virtual bool 		PlayTextureSounds( void ) = 0;
	virtual bool 		FAllowNPCs( void ) = 0;
	virtual void 		CreateStandardEntities() = 0;
	virtual const char 	*GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual const char 	*GetChatLocation( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual const char 	*GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual void 		CreateCustomNetworkStringTables( void ) = 0;
	virtual void		GoToIntermission( void ) = 0;
	virtual bool		PlayersAllowedToAttack( void ) const = 0;
	virtual float		GetBuyTimeLength() = 0;
	virtual int			PopulateDeathEvent(IGameEvent *, CBaseEntity const*, CBaseCombatCharacter const*, CTakeDamageInfo const&) = 0;
	virtual bool		DoesEntityBlockExplosions(CBaseEntity *)const = 0;
	virtual int			DeathNoticeForEntity(CBaseCombatCharacter *, CTakeDamageInfo const&) = 0;
	virtual void		CleanUpMap(void) = 0;
	virtual void		CheckRoundTimeExpired(void) = 0;
	virtual void		CheckWinConditions(void) = 0;
	virtual void		TerminateRound( float tmDelay, int reason ) = 0;
	virtual void		RestartRound() = 0;
	virtual bool		TeamFull(int) = 0;
	virtual bool		CheckGameOver() = 0;
	virtual bool		CheckMaxRounds() = 0;
	virtual bool		CheckWinLimit() = 0;
	virtual bool		CheckFragLimit() = 0;
	virtual void		CheckRestartRound() = 0;
	virtual bool		NeededPlayersCheck( bool &bNeededPlayers ) = 0;
	virtual void		SetAllowWeaponSwitch( bool allow ) = 0;
	virtual bool		GetAllowWeaponSwitch( void ) = 0;
};

class ITerrorGameRules : public ICSGameRules
{
public:
	virtual void LevelInitPreEntity() = 0;
	virtual void LevelInitPostEntity() = 0;

	virtual ~ITerrorGameRules() {}

	virtual const CViewVectors* GetViewVectors() const = 0;
	virtual float 		GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType ) = 0;
	virtual const unsigned char *GetEncryptionKey() = 0;
	virtual bool 		InRoundRestart( void ) = 0;
	virtual void 		OnBeginChangeLevel(char const*,KeyValues *) = 0;
	virtual void 		Think( void ) = 0;
	virtual bool 		FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ) = 0;
	virtual const char 	*GetGameDescription( void ) = 0;
	virtual bool 		ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) = 0;
	virtual void 		ClientDisconnected( edict_t *pClient ) = 0;
	virtual float 		FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;
	virtual int			GetAutoAimMode() = 0;
	virtual void 		PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	virtual bool 		FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;
	virtual CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer ) = 0;
	virtual void 		ClientSettingsChanged( CBasePlayer *pPlayer ) = 0;
	virtual void 		PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info ) = 0;
	virtual float 		WeaponTraceEntity( CBaseEntity *pEntity, const Vector &vecStart, const Vector &vecEnd, unsigned int mask, trace_t *ptr ) = 0;
	virtual void 		CreateStandardEntities() = 0;
	virtual const char 	*GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual const char 	*GetChatLocation( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual const char 	*GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer ) = 0;
	virtual void 		UpdateGameplayStatsFromSteam(void) = 0;
	virtual int 		GetMaxHumanPlayers(void)const = 0;
	virtual bool		PlayersAllowedToAttack( void ) const = 0;
	virtual float		GetBuyTimeLength() = 0;
	virtual int			PopulateDeathEvent(IGameEvent *, CBaseEntity const*, CBaseCombatCharacter const*, CTakeDamageInfo const&) = 0;
	virtual bool		DoesEntityBlockExplosions(CBaseEntity *)const = 0;
	virtual void		CleanUpMap(void) = 0;
	virtual void		CheckRoundTimeExpired(void) = 0;
	virtual void		CheckWinConditions(void) = 0;
	virtual void		TerminateRound( float tmDelay, int reason ) = 0;
	virtual void		RestartRound() = 0;
	virtual bool		TeamFull(int) = 0;
	virtual bool		CheckGameOver() = 0;
	virtual bool		CheckMaxRounds() = 0;
	virtual bool		CheckWinLimit() = 0;
	virtual bool		CheckFragLimit() = 0;
	virtual void		CheckRestartRound() = 0;
	virtual bool		NeededPlayersCheck( bool &bNeededPlayers ) = 0;
	virtual unsigned int PopulateDeathEvent(KeyValues *, const CBaseEntity*, const CBaseCombatCharacter*, const CTakeDamageInfo&) = 0;
	virtual	void		TrackPlayerZombieDamage(CTerrorPlayer *, int, bool) = 0;
	virtual	int			GetPlayerZombieDamage(ZombieClassType, bool) = 0;
	virtual	int			ClearPlayerZombieDamage(void) = 0;
};
#endif