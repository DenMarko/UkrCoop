#ifndef _INCLUDE_MUSIC_HEADER_
#define _INCLUDE_MUSIC_HEADER_

#include "ITerrorPlayer.h"

class IMusic
{
public:
    virtual ~IMusic() {}
    
    virtual void        NetworkStateChanged(void) = 0;
    virtual void        NetworkStateChanged(void *) = 0;
    virtual void        ClearDeathDSP(void) = 0;
    virtual DWORD       AppendMissionStr(char *, char *, int) = 0;
    virtual DWORD       OnMissionStart(void) = 0;
    virtual DWORD       OnEnterCheckpoint(void) = 0;
    virtual void        OnEnterHoldout(void) = 0;
    virtual DWORD       OnHoldoutStart(void) = 0;
    virtual DWORD       OnLeaveMissionStartArea(void) = 0;
    virtual DWORD       OnCheckpointReached(void) = 0;
    virtual DWORD       OnFinaleCheckpointReached(void) = 0;
    virtual DWORD       OnLeaveCheckpoint(void) = 0;
    virtual DWORD       OnLandmarkRevealed(string_t) = 0;
    virtual DWORD       OnBossApproaching(void) = 0;
    virtual DWORD       OnBossSeen(void) = 0;
    virtual DWORD       OnBossDefeated(ZombieClassType) = 0;
    virtual int         OnFinaleStart(void) = 0;
    virtual DWORD       OnFinaleEscapeStart(void) = 0;
    virtual DWORD       OnFinaleWave(int) = 0;
    virtual DWORD       OnFinaleHalftimeTank(void) = 0;
    virtual DWORD       OnFinaleTankBrothers(void) = 0;
    virtual DWORD       OnFinaleFinalBattle(void) = 0;
    virtual DWORD       OnMissionWon(void) = 0;
    virtual DWORD       OnMissionWonDone(void) = 0;
    virtual DWORD       OnMissionLost(void) = 0;
    virtual DWORD       OnKilled(bool) = 0;
    virtual void        SetPZAlertTimerMins(void) = 0;
    virtual DWORD       OnPZInRange(void) = 0;
    virtual void        OnBoomerSpawn(void) = 0;
    virtual void        OnBoomerAlert(int, float) = 0;
    virtual DWORD       OnVomitedUpon(void) = 0;
    virtual DWORD       OnITExpired(void) = 0;
    virtual void        OnSmokerSpawn(void) = 0;
    virtual void        OnSmokerAlert(int, float) = 0;
    virtual DWORD       OnPulledByTongue(void) = 0;
    virtual DWORD       OnChoked(void) = 0;
    virtual DWORD       OnReleasedByTongue(void) = 0;
    virtual void        OnHunterSpawn(void) = 0;
    virtual void        OnHunterAlert(int, float) = 0;
    virtual DWORD       OnLunged(void) = 0;
    virtual DWORD       OnPounced(void) = 0;
    virtual DWORD       OnPounceEnded(void) = 0;
    virtual void        OnWitchAlert(int, float) = 0;
    virtual DWORD       OnWitchGettingMad(void) = 0;
    virtual DWORD       OnWitchAttacking(void) = 0;
    virtual void        OnWitchBurning(void) = 0;
    virtual void        OnWitchAttackDone(void) = 0;
    virtual void        OnWitchKilled(void) = 0;
    virtual DWORD       OnMobSpawn(int, int) = 0;
    virtual DWORD       OnMobApproachingNear(int) = 0;
    virtual DWORD       OnMobApproachingFar(int) = 0;
    virtual DWORD       OnITMobApproaching(void) = 0;
    virtual DWORD       OnLedgeHangTwoHands(void) = 0;
    virtual DWORD       OnLedgeHangOneHand(void) = 0;
    virtual DWORD       OnLedgeHangFingers(void) = 0;
    virtual DWORD       OnLedgeHangBleedingOut(void) = 0;
    virtual int         OnSavedFromLedgeHang(void) = 0;
    virtual DWORD       OnFellToDeathFromLedgeHang(void) = 0;
    virtual DWORD       OnDroppedFromLedgeHang(void) = 0;
    virtual DWORD       OnDown(void) = 0;
    virtual DWORD       OnDownEnded(void) = 0;
    virtual DWORD       OnDownAndBeaten(void) = 0;
    virtual DWORD       OnDownAndBeatenEnded(void) = 0;
    virtual DWORD       OnBleedingOut(void) = 0;
    virtual DWORD       OnBleedingOutEnded(void) = 0;
    virtual void        OnLargeAreaRevealed(void) = 0;
    virtual DWORD       OnSmallAreaRevealed(void) = 0;
    virtual DWORD       OnPlayerRestored(CTerrorPlayer *) = 0;
    virtual DWORD       PlayInOut(void *, char const*, int) = 0;
};

#endif