#include "ITerrorPlayer.h"
#include "IMusic.h"
#include "IBaseAbility.h"
#include "IPlayerAnimState.h"
#include "../MemberFunctionWrapper.h"
#include "../CUserMessage.h"

template<typename T>
bool ForEachSurvivor(T &func)
{
    for(int i = 1; i <= g_pGlobals->maxClients; ++i)
    {
        auto pPlayer = playerhelpers->GetGamePlayer(i);
        if(pPlayer)
        {
            if(pPlayer->IsConnected() && pPlayer->IsInGame())
            {
                if(!func(i))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

float ITerrorPlayer::GetProgressBarPercent()
{
    if(!IsProgressBarActive())
    {
        return 0.0f;
    }

    long double &m_iProgressBarDuration     = access_member<long double>(this, 5984);
    float &m_flProgressBarStartTime         = access_member<float>(this, 5980);
    
    if(GetSimulationTime() > m_flProgressBarStartTime + m_iProgressBarDuration)
        return 1.0f;
    else
        return (GetSimulationTime() - m_flProgressBarStartTime) / m_iProgressBarDuration;
}

bool ITerrorPlayer::IsStaggering(void)
{
    if(access_member<int>(this, 2027 * 4) == -1)
    {
        CountdownTimers &m_staggerTimer = access_member<CountdownTimers>(this, 8068);
        if(m_staggerTimer.IsElapsed())
        {
            return false;
        }

        float m_staggerDist         = access_member<float>(this, 8104);
        Vector &m_staggerStart      = access_member<Vector>(this, 8080);
        
        if(IsEFlagsSet(EFL_SETTING_UP_BONES))
        {
            CalcAbsolutePosition();
            m_staggerDist = access_member<float>(this, 8104);
        }
        
        Vector v2 = GetAbsOrigin() - m_staggerStart;
        return v2.Length() <= m_staggerDist ? true : false;
    }
    return true;
}

const Vector &ITerrorPlayer::GetStaggerDir(void)
{
    CountdownTimers &m_staggerTimer = access_member<CountdownTimers>(this, 8068);

    if(m_staggerTimer.IsElapsed())
    {
        return vec3_origin;
    }

    float m_staggerDist         = access_member<float>(this, 8104);
    Vector &m_staggerStart      = access_member<Vector>(this, 8080);

    if(IsEFlagsSet(EFL_SETTING_UP_BONES))
    {
        CalcAbsolutePosition();
        m_staggerDist = access_member<float>(this, 8104);
    }
    
    Vector v2 = GetAbsOrigin() - m_staggerStart;
    if(v2.Length() > m_staggerDist)
        return vec3_origin;

    return access_member<Vector>(this, 8092);
}

float ITerrorPlayer::GetTimeSinceLastFiredWeapon(void)
{
    IntervalTimers &m_LastFireWeapon = access_member<IntervalTimers>(this, 7880);
    if(!m_LastFireWeapon.HasStarted())
    {
        return g_pGlobals->curtime;
    }

    if(m_LastFireWeapon.IsLessThen(0.0f))
    {
        return (g_pGlobals->curtime - access_member<float>(this, 7888));
    }

    return m_LastFireWeapon.GetElapsedTime();
}

bool ITerrorPlayer::IsFiringWeapon(void)
{
    IntervalTimers &m_LastFireWeapon = access_member<IntervalTimers>(this, 7880);
    return m_LastFireWeapon.HasStarted() && m_LastFireWeapon.IsLessThen(0.5);
}

ITerrorPlayer *ITerrorPlayer::GetOtherResponsibleForMovement()
{
    CHandle<ITerrorPlayer> &m_tongueOwner = access_member<CHandle<ITerrorPlayer>>(this, 8516);
    if(m_tongueOwner != NULL)
    {
        return m_tongueOwner;
    }

    CountdownTimers &m_timer = access_member<CountdownTimers>(this, 8208);
    CHandle<ITerrorPlayer> &m_handle1 = access_member<CHandle<ITerrorPlayer>>(this, 8220);
    if(m_timer.IsElapsed() && m_handle1 == NULL)
    {
        CountdownTimers &m_timer1 = access_member<CountdownTimers>(this, 8028);
        CHandle<ITerrorPlayer> &m_handle2 = access_member<CHandle<ITerrorPlayer>>(this, 8040);
        if(!m_timer1.IsElapsed() && m_handle2 != NULL)
        {
            return GetRecentPusher();
        }

        CountdownTimers &m_timer2 = access_member<CountdownTimers>(this, 8224);
        CHandle<ITerrorPlayer> &m_handle3 = access_member<CHandle<ITerrorPlayer>>(this, 8236);
        if(!m_timer2.HasStarted() && m_timer2.IsElapsed())
        {
            if(m_handle3 == NULL)
            {
                return nullptr;
            }
            
            if(GetGroundEntity())
            {
                return nullptr;
            }
        }

        if(m_handle3)
        {
            return m_handle3;
        }
        return nullptr;
    }

    if(!m_timer.IsElapsed() && m_handle1 != NULL)
        return m_handle1;

    return nullptr;
}

ITerrorPlayer *ITerrorPlayer::GetRecentPusher()
{
    CountdownTimers &m_timer = access_member<CountdownTimers>(this, 8028);
    CHandle<ITerrorPlayer> &m_handle = access_member<CHandle<ITerrorPlayer>>(this, 8040);
    if(!m_timer.IsElapsed() && m_handle != NULL)
    {
        return m_handle;
    }

    return nullptr;
}

bool ITerrorPlayer::IsStill(bool val) const
{
    if(val)
        return access_member<float>(const_cast<ITerrorPlayer*>(this), 7952) > 0.0f;

    if(access_member<float>(const_cast<ITerrorPlayer*>(this), 7952) <= 0.f)
        return false;

    return access_member<float>(const_cast<ITerrorPlayer*>(this), 7960) > 0.0f;
}

class FindCharacter
{
public:
    FindCharacter(int8_t charact) : m_character(charact), m_pPlayer(nullptr) {}

    virtual bool operator()(int index)
    {
        auto pPlayer = GetVirtualClass<ITerrorPlayer>(index);
        if(!pPlayer)
            return true;

        if(!pPlayer->IsPlayer())
            return true;

        if(!pPlayer->IsConnected())
            return true;

        if(pPlayer->GetTeamNumber() == 2)
        {
            int8_t m_survivorCharacter = access_member<int8_t>(pPlayer, 6472);
            if(m_survivorCharacter == m_character)
            {
                m_pPlayer = pPlayer;
                return false;
            }
        }
        return true;
    }

    inline ITerrorPlayer *GetPlayer(void) { return m_pPlayer; }

private:
    int8_t m_character;
    ITerrorPlayer *m_pPlayer;
};

ITerrorPlayer *ITerrorPlayer::GetPlayerByCharacter(ITerrorPlayer* pPlayer, int charact)
{
    if(charact == 4)
        return nullptr;

    FindCharacter find(charact);
    ForEachSurvivor(find);

    if(pPlayer == find.GetPlayer())
    {
        return find.GetPlayer();
    }

    return nullptr;
}

#include "INavArea.h"
float ITerrorPlayer::GetFlowDistance(int typeFlow) const
{
    auto pResult = (INavArea*)GetLastKnownArea();
    if(pResult)
    {
        return access_array_member<float>(pResult, 348, typeFlow);
    }

    return -9999.0f;
}

IBaseEntity *ITerrorPlayer::GetMinigun()
{
    if(access_member<bool>(this, 8508))
    {
        return access_dynamic_cast<IBaseEntity>(const_cast<IBaseEntity*>(GetUseEntity()), "CPropMinigun");
    }

    return nullptr;
}

ITerrorWeapon *ITerrorPlayer::GetActiveTerrorWeapon() const
{
    return (ITerrorWeapon*)GetActiveWeapon();
}

void ITerrorPlayer::OnStaggered(IBaseEntity *pAttacker, const Vector *forceDirect)
{
    CHandle<ITerrorPlayer> &m_hStaggerAttacker = access_member<CHandle<ITerrorPlayer>>(this, 8040);
    CountdownTimers &m_Timer = access_member<CountdownTimers>(this, 8028);
    CountdownTimers &m_staggerTimer = access_member<CountdownTimers>(this, 8068);
    Vector &m_staggerDir = access_member<Vector>(this, 8092);
    Vector &m_staggerStart = access_member<Vector>(this, 8080);
    float &m_staggerDist = access_member<float>(this, 8104);

    if(!IsPlayingDeathAnim())
    {
        if(!IsIncapacitated())
        {
            StopRevivingSomeone(true);
            ClearUseEntity();
            SetPushEntity(nullptr);
            OnPounceEnded();
            ReleaseTongueVictim(false);

            if(GetActiveTerrorWeapon())
            {
                GetActiveTerrorWeapon()->SuppressHelpingHands(0.f);
            }

            if(m_hStaggerAttacker == NULL)
            {
                if(GetTeamNumber() == 2)
                {
                    EmitSound("Player.Shoved");
                }
            }

            m_hStaggerAttacker = NULL;
            if(pAttacker)
            {
                IBaseCombatCharacter* pCharact = (IBaseCombatCharacter*)pAttacker->MyCombatCharacterPointer();
                if(pCharact != nullptr)
                {
                    m_hStaggerAttacker = pCharact->GetRefEHandle();
                }
            }

            m_Timer.Start(3.0f);

            Vector staggerDirect;
            if(forceDirect)
                staggerDirect = GetAbsOrigin() - (*forceDirect);
            else
                staggerDirect = GetAbsOrigin() - pAttacker->GetAbsOrigin();

            if(this == pAttacker && !forceDirect)
                EyeVectors(&staggerDirect);

            VectorNormalize(staggerDirect);
            QAngle angle;
            VectorAngles(staggerDirect, angle);
            float anglediff = AngleNormalize(EyeAngles().y - angle.y);

            if(anglediff >= -45.f && anglediff <= 45.f)
            {
                SetMainActivity(ACT_TERROR_SHOVED_FORWARD);
            }
            else if(anglediff > 45.f && anglediff < 135.f)
            {
                SetMainActivity(ACT_TERROR_SHOVED_RIGHTWARD);
            }
            else if(anglediff < -45.f && anglediff > -135.f)
            {
                SetMainActivity(ACT_TERROR_SHOVED_LEFTWARD);
            }
            else
            {
                SetMainActivity(ACT_TERROR_SHOVED_BACKWARD);
            }

            if(GetTeamNumber() == 3)
            {
                static ConVarRef z_max_stagger_duration("z_max_stagger_duration");
                static ConVarRef z_tank_max_stagger_distance("z_tank_max_stagger_distance");

                m_staggerTimer.Start(z_max_stagger_duration.GetFloat());
                if(z_tank_max_stagger_distance.GetFloat() != m_staggerDist)
                {
                    NetworkStateChanged(8104);
                    m_staggerDist = z_tank_max_stagger_distance.GetFloat();
                }
            }
            else if(GetTeamNumber() == 2)
            {
                static ConVarRef survivor_max_tongue_stagger_duration("survivor_max_tongue_stagger_duration");
                static ConVarRef survivor_max_tongue_stagger_distance("survivor_max_tongue_stagger_distance");

                m_staggerTimer.Start(survivor_max_tongue_stagger_duration.GetFloat());
                if(survivor_max_tongue_stagger_distance.GetFloat() != m_staggerDist)
                {
                    NetworkStateChanged(8104);
                    m_staggerDist = survivor_max_tongue_stagger_distance.GetFloat();
                }
            }

            if(m_staggerDir != staggerDirect)
            {
                NetworkStateChanged(8092);
                m_staggerDir = staggerDirect;
            }

            if(m_staggerStart != GetAbsOrigin())
            {
                NetworkStateChanged(8080);
                m_staggerStart = GetAbsOrigin();
            }
        }
    }
}

void ITerrorPlayer::OnShovedBySurvivor(ITerrorPlayer *pPusher, const Vector &force)
{
    if(IsPlayingDeathAnim())
        return;

    bool isHunterPouncing = false;
    if(pPusher)
    {
        CHandle<ITerrorPlayer> &m_pounceAttacker = access_member<CHandle<ITerrorPlayer>>(pPusher, 2717*4);
        if(m_pounceAttacker != NULL)
            isHunterPouncing = true;
    }

    Vector &m_staggerDir = access_member<Vector>(this, 8092);
    static ConVarRef z_tank_max_stagger_duration("z_tank_max_stagger_duration");
    static ConVarRef z_stagger_immunity_time("z_stagger_immunity_time");

    StopRevivingSomeone(true);
    Vector shoveDir = force;
    if(shoveDir.LengthSqr() > 10000.f)
    {
        shoveDir.NormalizeInPlace();
        shoveDir *= 100.f;
    }

    Vector forceDir = shoveDir;
    forceDir.z = 0.f;
    forceDir.NormalizeInPlace();
    if(forceDir != m_staggerDir)
    {
        NetworkStateChanged(8092);
        m_staggerDir = forceDir;
    }

    QAngle angle;
    VectorAngles(m_staggerDir, angle);
    QAngle eyeAngles = EyeAngles();

    if(IsRenderYawOverridden())
    {
        eyeAngles.y = access_member<float>(this, 6416);     // m_overriddenRenderYaw
        SnapEyeAngles(eyeAngles);
    }

    float anglDiff = AngleNormalize(eyeAngles.y - angle.y);
    Activity shoveActivity = ACT_INVALID;

    if(anglDiff <= 45.f && anglDiff >= -45.f)
    {
        shoveActivity = ACT_TERROR_SHOVED_FORWARD;
    }
    else if(anglDiff > 45.f && anglDiff < 135.f)
    {
        shoveActivity = ACT_TERROR_SHOVED_RIGHTWARD;
    }
    else if(anglDiff < -45.f)
    {
        shoveActivity = ACT_TERROR_SHOVED_BACKWARD;
    }
    else
    {
        shoveActivity = (anglDiff > -135.f) ? ACT_TERROR_SHOVED_BACKWARD : ACT_TERROR_SHOVED_LEFTWARD;
    }

    if(GetTeamNumber() == 3)
    {
        if(GetClass() == ZombieClassHunter)
        {
            int &m_lastPouncheTime = access_member<int>(pPusher, 2732*4);
            if(m_lastPouncheTime == g_pGlobals->framecount)
            {
                m_lastPouncheTime = -1;

                if(shoveActivity == ACT_TERROR_SHOVED_RIGHTWARD)
                    shoveActivity = ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_R;
                else if(shoveActivity == ACT_TERROR_SHOVED_BACKWARD)
                    shoveActivity = ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_BACKWARD;
                else if(shoveActivity == ACT_TERROR_SHOVED_FORWARD)
                    shoveActivity = ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_FORWARD;
                else
                    shoveActivity = ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_L;
            }
        }
    }

    if(isHunterPouncing)
    {
        if(GetTeamNumber() == 3 && GetClass() == ZombieClassHunter)
        {
            shoveActivity = ACT_TERROR_SHOVED_FORWARD;
        }
    }

    CountdownTimers &m_timer8056 = access_member<CountdownTimers>(this, 8056);
    if(m_timer8056.HasStarted())
    {
        m_timer8056.Invalidate();
    }

    float &m_staggerDist = access_member<float>(this, 8104);
    if(m_staggerDist != z_tank_max_stagger_duration.GetFloat())
    {
        NetworkStateChanged(8104);
        m_staggerDist = z_tank_max_stagger_duration.GetFloat();
    }

    Vector &m_staggerStart = access_member<Vector>(this, 8080);
    if(m_staggerStart != GetAbsOrigin())
    {
        NetworkStateChanged(8080);
        m_staggerStart = GetAbsOrigin();
    }

    bool &m_isAttemptingToPounce = access_member<bool>(this, 10884);
    if(m_isAttemptingToPounce)
    {
        NetworkStateChanged(10884);
        m_isAttemptingToPounce = false;
    }

    float stunDuration = 0.f;
    bool plaedStunAnimation = false;

    if(!m_timer8056.HasStarted())
    {
        if(!access_member<bool>(this, 11192))
        {
            if(GetGroundEntity())
            {
                SetMainActivity(shoveActivity);
                IPlayerAnimState *pAnimState = access_member<IPlayerAnimState*>(this, 6024);
                pAnimState->OnStaggerStart();
                pAnimState->ClearAccumulatedMotion();
                plaedStunAnimation = true;
            }
            else
            {
                access_member<Activity>(this, 8108) = shoveActivity;
                Vector &m_shoveForce = access_member<Vector>(this, 8472);
                if(m_shoveForce != shoveDir)
                {
                    NetworkStateChanged(8472);
                    m_shoveForce = shoveDir;
                }

                if(pPusher)
                {
                    access_member<CHandle<ITerrorPlayer>>(this, 8236) = pPusher->GetRefEHandle();
                    CountdownTimers &m_timer8224 = access_member<CountdownTimers>(this, 8224);
                    m_timer8224.Start(1.0);
                }
                else
                {
                    access_member<CHandle<ITerrorPlayer>>(this, 8236).Term();
                    CountdownTimers &m_timer8224 = access_member<CountdownTimers>(this, 8224);
                    m_timer8224.Invalidate();
                }

                SetGroundEntity(NULL);
                plaedStunAnimation = false;
            }

            stunDuration = z_stagger_immunity_time.GetFloat();
            if(stunDuration > 0.f)
            {
                CountdownTimers &m_timer8056 = access_member<CountdownTimers>(this, 8056);
                m_timer8056.Start(stunDuration);
            }
        }
    }


    if(GetTeamNumber() == 3 && GetClass() == ZombieClassTank)
    {
        static ConVarRef z_tank_stagger_fade_alpha("z_tank_stagger_fade_alpha");
        static ConVarRef z_tank_stagger_fade_duration("z_tank_stagger_fade_duration");
        color32 color = { 255, 255, 255, 0 };
        color.a = z_tank_stagger_fade_alpha.GetInt();
        UTIL_ScreenFade(this, color, z_tank_stagger_fade_duration.GetFloat(), 0.f, 1);
    }
    else
    {
        static ConVarRef z_max_stagger_duration("z_max_stagger_duration");
        static ConVarRef z_max_hunter_pounce_stagger_duration("z_max_hunter_pounce_stagger_duration");
        stunDuration = SequenceDuration(GetSequence());
        if(z_max_stagger_duration.GetFloat() > 0.0f && stunDuration > z_max_stagger_duration.GetFloat() )
        {
            stunDuration = z_max_stagger_duration.GetFloat();
        }

        extern bool IsVersusMode();
        if(isHunterPouncing && IsVersusMode())
        {
            stunDuration = z_max_hunter_pounce_stagger_duration.GetFloat();
        }
    }

    if(plaedStunAnimation)
    {
        OnStunned(stunDuration);

        CountdownTimers &m_timer8068 = access_member<CountdownTimers>(this, 8068);
        m_timer8068.Start(stunDuration);
    }

    DoAnimationEvent(static_cast<PlayerAnimEvent_t>(33));
    IBaseAbility *pAbility = GetCustomAbility();
    if(pAbility)
    {
        if(GetTeamNumber() == 3)
        {
            if(GetClass() == ZombieClassTank)
                pAbility->OnStunned(z_tank_max_stagger_duration.GetFloat());
            else if(GetClass() == ZombieClassHunter)
                pAbility->OnStunned(0.5f);
        }
    }

    bool shouldTrackShoveTime = !isHunterPouncing;
    if(!shouldTrackShoveTime)
    {
        if(GetTeamNumber() == 3)
        {
            if(GetClass() == ZombieClassHunter || GetClass() == ZombieClassSmoker)
            {
                shouldTrackShoveTime = false;
            }
        }
    }

    float &m_bashedStart = access_member<float>(this, 10964);
    if(m_bashedStart != g_pGlobals->curtime)
    {
        NetworkStateChanged(10964);
        m_bashedStart = g_pGlobals->curtime;
    }

    CUtlVector<float> &m_shoveTimeHistory = access_member<CUtlVector<float>>(this, 8452);
    int &m_skill = access_member<int>(pPusher, 11072);
    int &m_consecutiveShoveCount = access_member<int>(this, 0x2100);
    bool shouldDamage = false;

    m_shoveTimeHistory.AddToTail(g_pGlobals->curtime);

    if(m_skill == 7)
    {
        m_consecutiveShoveCount = 1;
        shouldDamage = (m_shoveTimeHistory.Count() >= 1);
    }
    else
    {
        shouldDamage = (m_shoveTimeHistory.Count() >= m_consecutiveShoveCount);
    }

    if(GetTeamNumber() == 3)
    {
        if(GetClass() == ZombieClassBommer 
        || GetClass() == ZombieClassSmoker 
        || GetClass() == ZombieClassHunter)
        {
            CTakeDamageInfoHack damageInfo(this, pPusher, 250.f, DMG_CLUB);
            
            if(shouldDamage)
            {
                trace_t tr;
                memset(&tr, 0, sizeof(tr));

                Vector vecCenter = WorldSpaceCenter();

                tr.startpos = vecCenter;
                tr.endpos = vecCenter;

                tr.plane.normal = -forceDir;
                tr.plane.dist = DotProduct(-forceDir, vecCenter);
                tr.plane.type = PLANE_ANYZ;
                tr.plane.signbits = SignbitsForPlane(&tr.plane);

                tr.fraction = 1.f;
                tr.m_pEnt = (CBaseEntity*)this;

                Vector vecShotPos = pPusher->Weapon_ShootPosition();
                CalculateMeleeDamageForce(&damageInfo, forceDir, vecShotPos, 0.2f);
                g_CallHelper->ClearMultiDamages();
                DispatchTraceAttack(damageInfo, forceDir, &tr);
                g_CallHelper->ApplyMultiDamages();

                return;
            }

            EmitInfectedPainSound(damageInfo);
        }
    }
}

void ITerrorPlayer::SetPushEntity(IBaseEntity *pEntity)
{
    CHandle<IBaseEntity> &m_hPushEntity = access_member<CHandle<IBaseEntity>>(this, 8492);
    float &m_flPushEntityDistance = access_member<float>(this, 8496);

    CHandle<IBaseEntity> newPushHandle = NULL;
    if(pEntity)
    {
        newPushHandle = pEntity->GetRefEHandle();
    }

    if(m_hPushEntity != newPushHandle)
    {
        NetworkStateChanged(8492);
        m_hPushEntity = newPushHandle;
    }

    if(newPushHandle != NULL)
    {
        NetworkStateChanged(8496);
        m_flPushEntityDistance = 32.f;
    }

    return;
}

void ITerrorPlayer::StopRevivingSomeone(bool playSound)
{
    CHandle<ITerrorPlayer> &m_reviveTarget = access_member<CHandle<ITerrorPlayer>>(this, 1765*4);

    SetProgressBarTime(0, "");
    if(m_reviveTarget != NULL)
    {
        ITerrorPlayer *pTarget = m_reviveTarget.Get();
        bool &m_isHangingFromLedge = access_member<bool>(pTarget, 10001);
        if(GetActiveTerrorWeapon())
        {
            GetActiveTerrorWeapon()->SuppressHelpingHands(0.0f);
        }

        DoAnimationEvent(static_cast<PlayerAnimEvent_t>(17));
        pTarget->DoAnimationEvent(static_cast<PlayerAnimEvent_t>(19));

        CHandle<ITerrorPlayer> &m_reviveOwner = access_member<CHandle<ITerrorPlayer>>(pTarget, 7056);
        NetworkStateChanged(7056);
        m_reviveOwner = NULL;

        pTarget->SetProgressBarTime(0, "");
        if(playSound)
        {
            EmitSound("Player.StopVoice");
        }

        CReviveEnd *revive_end = new CReviveEnd;
        revive_end->Set( this, pTarget, m_isHangingFromLedge);
        delete revive_end;
        
        if(playSound)
        {
            extern const char *SurvivorCharacterName(int8_t character);
            AI_CriteriaSet criter;
            int8_t m_survivorCharacter = access_member<int8_t>(this, 6472);
            criter.AppendCriteria("Subject", SurvivorCharacterName(m_survivorCharacter));
            criter.AppendCriteria("LedgeHand", m_isHangingFromLedge ? "1" : "0");
            CAI_Concept concepts("ReviveMeInterrupted");
            CFollowupTargetSpec_t targetSpec;
            targetSpec.m_iTargetType = kDRT_SPECIFIC;
            targetSpec.m_hHandle = pTarget->GetRefEHandle();
            g_CallHelper->CResponseQueue_Add(concepts, &criter, g_pGlobals->curtime + 0.2f, targetSpec, this);
        }
    }

    if(m_reviveTarget != NULL)
    {
        NetworkStateChanged(7060);
        m_reviveTarget = NULL;
    }

    StopHealingSomeone();
    return;
}

void ITerrorPlayer::StopHealingSomeone(void)
{
    SetProgressBarTime(0, "");
    ITerrorPlayer *pTarget = GetHealTarget();
    if(pTarget != nullptr)
    {
        if(GetActiveTerrorWeapon())
        {
            GetActiveTerrorWeapon()->SuppressHelpingHands(0.f);
        }

        pTarget->SetProgressBarTime(0, "");
        CHandle<ITerrorPlayer> &m_healOwner = access_member<CHandle<ITerrorPlayer>>(pTarget, 6380);
        if(m_healOwner != NULL)
        {
            pTarget->NetworkStateChanged(6380);
            m_healOwner.Term();
        }
    }

    CHandle<ITerrorPlayer> &m_healTarget = access_member<CHandle<ITerrorPlayer>>(this, 1596*4);
    NetworkStateChanged(6384);
    m_healTarget.Term();
}

void ITerrorPlayer::OnPounceEnded(void)
{
    bool &m_isAttemptingToPounce = access_member<bool>(this, 10884);
    CountdownTimers &m_stunTimer = access_member<CountdownTimers>(this, 8280);
    CHandle<IBaseAbility> &m_customAbility = access_member<CHandle<IBaseAbility>>(this, 1934*4);

    if(m_isAttemptingToPounce && !m_stunTimer.IsElapsed())
            EmitSound("HunterZombie.Pounce.Cancel");

    // Update last attacked time to avoid being targeted immediately again
    if(access_member<bool>(this, 10885))
        access_member<int>(this, 2732*4) = g_pGlobals->framecount;

    ITerrorPlayer *pVictim = GetPounceVictim();
    if( pVictim != nullptr )
    {
        DoAnimationEvent(static_cast<PlayerAnimEvent_t>(33));
        IMusic &m_music = access_member<IMusic>(this, 10160);
        m_music.OnPounceEnded();
        CHandle<ITerrorPlayer> &m_pounceAttacker = access_member<CHandle<ITerrorPlayer>>(pVictim, 10868);
        if(m_pounceAttacker != NULL)
        {
            pVictim->NetworkStateChanged(10868);
            m_pounceAttacker = NULL;
        }

        auto ActiveCSWeapon = pVictim->GetActiveCSWeapon();
        if(ActiveCSWeapon)
            ActiveCSWeapon->Deploy();

        access_member<CountdownTimers>(pVictim, 7112).Start(2.0f);

        if(pVictim->IsIncapacitated())
        {
            ITerrorGun *pGun = nullptr;
            if(ActiveCSWeapon 
            && (pGun = (ITerrorGun*)ActiveCSWeapon->GetTerrorGun()) != nullptr 
            && pGun->IsDualWielding())
            {
                SetMainActivity(ACT_IDLE_INCAP_ELITES);
            }
            else
            {
                SetMainActivity(ACT_IDLE_INCAP_PISTOL);
            }
        }
        else
            SetMainActivity(ACT_TERROR_POUNCED_TO_STAND);

        SetDucked(true);
        SetDucking(false);
        SetDucktime(0.f);
        AddFlag(FL_DUCKING);
    }

    CPounceEnd *pounce_end = new CPounceEnd;
    pounce_end->Set(this, pVictim);
    delete pounce_end;

    CHandle<ITerrorPlayer> &m_pounceVictim = access_member<CHandle<ITerrorPlayer>>(this, 2716*4); // 10864
    if(m_pounceVictim != NULL)
    {
        NetworkStateChanged(10864);
        m_pounceVictim = NULL;
    }

    CHandle<ITerrorPlayer> &m_pounceAttacker = access_member<CHandle<ITerrorPlayer>>(this, 10868);
    if(m_pounceAttacker != NULL)
    {
        NetworkStateChanged(10868);
        m_pounceAttacker = NULL;
    }

    if(m_isAttemptingToPounce)
    {
        NetworkStateChanged(10884);
        m_isAttemptingToPounce = false;
    }

    if(access_member<bool>(this, 10885))
    {
        auto pActWeapon = GetActiveCSWeapon();
        if(pActWeapon)
            pActWeapon->SendWeaponAnim(546);
    }
    access_member<bool>(this, 10885) = false;

    if(m_customAbility != NULL)
    {
        m_customAbility->OnStunned(0);
    }
}

void ITerrorPlayer::ReleaseTongueVictim(bool force)
{
    CHandle<IBaseAbility> &m_customAbility = access_member<CHandle<IBaseAbility>>(this, 1934*4);

    ITerrorPlayer* pVictim = GetTongueVictim();
    if( !force && pVictim == nullptr)
    {
        return;
    }

    bool bResetTougle = false;
    ITongue* pAbility = nullptr;
    if(m_customAbility != NULL)
    {
        pAbility = access_dynamic_cast<ITongue>(m_customAbility.Get(), "CTongue");
        if(pAbility)
        {
            pAbility->ResetTongueTimer();
            bResetTougle = true;
        }
    }

    CTongueRelease *tongue_release = new CTongueRelease;

    float distance = 0.f;
    if(bResetTougle)
    {
        distance = pVictim->GetAbsOrigin().DistTo(pAbility->GetLastVictimPosition());
    }

    tongue_release->Set(this, pVictim, distance);
    delete tongue_release;

    if(pVictim)
    {
        pVictim->OnReleasedByTongue();
    }

    OnReleasingWithTongue();
}

void ITerrorPlayer::SetMainActivity(Activity activity, bool bRestart)
{
    CountdownTimers &m_staggerTimer = access_member<CountdownTimers>(this, 8068);
    CountdownTimers &m_tugTimer = access_member<CountdownTimers>(this, 8112);
    float &m_mainSequenceStartTime = access_member<float>(this, 6300);

    MDLCACHE_CRITICAL_SECTION_(g_pMDLCache);

    int nSeqence = SelectWeightedSequence(activity);
    if(nSeqence < 0)
        return;

    auto flCycle = GetCycle();
    if(IsActivityFinished())
        flCycle = 1.f;

    if(m_staggerTimer.GetRemainingTime() > 0.f)
        CancelStagger();

    if(m_tugTimer.GetRemainingTime() > 0.f)
        CancelTug();

    SetSequence(nSeqence);
    m_flAnimTime = g_pGlobals->curtime;
    SetCycle(0.f);

    NetworkStateChanged(6300);
    m_mainSequenceStartTime = g_pGlobals->curtime;

    Activity &nActivity = access_member<Activity>(this, 1615*4);

    Activity nOldActivity = nActivity;
    nActivity = GetSequenceActivity(nSeqence);
    access_member<bool>(this, 6464) = bRestart;
    ResetSequenceInfo();
    if(IsMotionControlledXY(activity))
        SetMoveType(MOVETYPE_CUSTOM);

    if(flCycle < 0.9f) {
        OnMainActivityInterrupted(nActivity, nOldActivity);
    } else {
        OnMainActivityComplete(nActivity, nOldActivity);
    }
}

void ITerrorPlayer::CancelStagger(void)
{
    CountdownTimers &m_staggerTimer = access_member<CountdownTimers>(this, 8068);
    m_staggerTimer.Invalidate();

    Vector &m_staggerDir = access_member<Vector>(this, 8092);
    if(m_staggerDir != 0.f)
    {
        NetworkStateChanged(8092);
        m_staggerDir = vec3_origin;
    }

    float &m_staggerDist = access_member<float>(this, 8104);
    if(m_staggerDist != 0.f)
    {
        NetworkStateChanged(8104);
        m_staggerDist = 0.f;
    }

    Vector &m_staggerStart = access_member<Vector>(this, 8080);
    if(m_staggerStart != 0.f)
    {
        NetworkStateChanged(8080);
        m_staggerStart = vec3_origin;
    }
}

void ITerrorPlayer::CancelTug(void)
{
    CountdownTimers &m_tugTimer = access_member<CountdownTimers>(this, 8112);
    m_tugTimer.Invalidate();

    Vector &m_tugDir = access_member<Vector>(this, 8136);
    if(m_tugDir != 0.f)
    {
        NetworkStateChanged(8136);
        m_tugDir = vec3_origin;
    }
    
    float &m_tugDist = access_member<float>(this, 8148);
    if(m_tugDist != 0.f)
    {
        NetworkStateChanged(8148);
        m_tugDist = 0.f;
    }

    Vector &m_tugStart = access_member<Vector>(this, 8124);
    if(m_tugStart != 0.f)
    {
        NetworkStateChanged(8124);
        m_tugStart = vec3_origin;
    }

    access_member<CHandle<IBaseEntity>>(this, 1724*4).Term();
}

void ITerrorPlayer::OnReleasedByTongue(void)
{
    static ConVarRef tongue_release_fatigue_penalty("tongue_release_fatigue_penalty");
    IntervalTimers &m_tongueVictimTimer = access_member<IntervalTimers>(this, 8520);
    CountdownTimers& m_timer_8208 = access_member<CountdownTimers>(this, 8208);
    CHandle<ITerrorPlayer> &m_tongueOwner = access_member<CHandle<ITerrorPlayer>>(this, 8516);
    bool &m_isProneTongueDrag = access_member<bool>(this, 8548);
    float &m_flStamina = access_member<float>(this, 5792);
    IntervalTimers &m_interval_8552 = access_member<IntervalTimers>(this, 8552);
    IntervalTimers &m_interval_8576 = access_member<IntervalTimers>(this, 8576);
    CountdownTimers &m_timer_7112 = access_member<CountdownTimers>(this, 7112);

    Vector vecVelocity = GetAbsVelocity();
    vecVelocity.z = 0.f;

    SetAbsVelocity(vecVelocity);
    m_tongueVictimTimer.Invalidate();
    m_timer_8208.Start(3.0f);
    SetImpactEnergyScale(1.f);
    OnStopHangingFromTongue(3);

    if(m_tongueOwner != NULL)
    {
        NetworkStateChanged(8516);
        m_tongueOwner = NULL;
    }

    if(m_isProneTongueDrag)
    {
        NetworkStateChanged(8548);
        m_isProneTongueDrag = false;
    }

    float flNewStamina = m_flStamina + tongue_release_fatigue_penalty.GetFloat();
    if(m_flStamina != flNewStamina)
    {
        NetworkStateChanged(5792);
        m_flStamina = flNewStamina;
    }
    ResetMaxSpeed();
    m_interval_8552.Invalidate();
    m_interval_8576.Invalidate();

    IMusic &m_music = access_member<IMusic>(this, 10160);
    m_music.OnReleasedByTongue();
    m_timer_7112.Start(2.0f);
}

void ITerrorPlayer::OnReleasingWithTongue(void)
{
    IBaseAbility *pAbility = GetCustomAbility();
    if(pAbility != nullptr)
    {
        ITongue *pTongue = access_dynamic_cast<ITongue>(pAbility, "CTongue");
        if(pTongue)
        {
            pTongue->SnapTongueBackToMouth();
        }
    }

    access_member<bool>(this, 8572) = false;
    access_member<bool>(this, 8573) = false;

    CHandle<ITerrorPlayer> &m_tongueVictim = access_member<CHandle<ITerrorPlayer>>(this, 8512);
    if(m_tongueVictim != NULL)
    {
        NetworkStateChanged(8512);
        m_tongueVictim = NULL;
    }

    ResetMaxSpeed();
}

void ITerrorPlayer::OnStopHangingFromTongue(int flags)
{
    bool &m_isHangingFromTongue = access_member<bool>(this, 8540);
    bool &m_reachedTongueOwner = access_member<bool>(this, 8541);
    bool &m_isProneTongueDrag = access_member<bool>(this, 8548);
    unsigned int &nFlags_8544 = access_member<unsigned int>(this, 8544);
    CHandle<ITerrorPlayer> &m_tongueOwner = access_member<CHandle<ITerrorPlayer>>(this, 8516);

    unsigned int oldFlags = nFlags_8544;
    nFlags_8544 &= ~flags;

    if(m_isHangingFromTongue)
    {
        if(nFlags_8544)
        {
            if(oldFlags != nFlags_8544)
            {
                if((nFlags_8544 & 2) != 0)
                {
                    if(!m_reachedTongueOwner)
                    {
                        if(m_tongueOwner == NULL && !m_isProneTongueDrag)
                        {
                            SetMainActivity(ACT_TERROR_STANDING_CHOKE_FROM_TONGUE);
                        }
                    }

                    SetMainActivity(ACT_TERROR_CHOKING_TONGUE_GROUND);
                }
                else
                {
                    SetMainActivity(ACT_TERROR_HANGING_FROM_TONGUE);
                }
                return;
            }
        }
        else
        {
            NetworkStateChanged(8540);
            m_isHangingFromTongue = false;

            if(flags == 3)
            {
                if(IsIncapacitated() && !access_member<bool>(this, 10001))
                {
                    if((GetFlags() & 1) != 0)
                        SetMainActivity(ACT_TERROR_INCAP_FROM_TONGUE);
                    else
                        SetMainActivity(ACT_TERROR_IDLE_FALL_FROM_TONGUE);
                }

                auto pWeapon = GetActiveTerrorWeapon();
                CountdownTimers &m_timer_7112 = access_member<CountdownTimers>(this, 7112);
                if(pWeapon && pWeapon->IsEffectActive(0x20))
                {
                    pWeapon->Deploy();
                    if(IsIncapacitated())
                    {
                        if(HasSecondaryWeapon())
                        {
                            Weapon_Switch(Weapon_GetSlot(1));
                        }
                    }
                    else
                        m_timer_7112.Start(2.0f);
                }
            }

            CChokeEnd *choke_end = new CChokeEnd;
            choke_end->Set(m_tongueOwner, this);
            delete choke_end;

            if(m_tongueOwner != NULL)
            {
                m_tongueOwner->AbilityDebug(this, "Tongue choke ending.");
            }
        }
    }
}

void ITerrorPlayer::OnStunned(float duration)
{
    if(GetTeamNumber() == 3
    && GetClass() == ZombieClassBommer
    && GetFallVelocity() != 0.0)
    {
        SetFallVelocity(0.f);
    }

    CountdownTimers &m_timer8280 = access_member<CountdownTimers>(this, 8280);
    CountdownTimers &m_timer8292 = access_member<CountdownTimers>(this, 8292);

    if(m_timer8280.IsElapsed())
    {
        m_timer8280.Start(duration);
        IBaseAbility* pAbility = GetCustomAbility();
        if(pAbility)
        {
            pAbility->OnStunned(duration);
        }

        ITerrorWeapon *pWeapon = GetActiveTerrorWeapon();
        if(pWeapon)
        {
            pWeapon->OnStunned(duration);
        }
    }

    static ConVarRef survivor_stun_immunity_duration("survivor_stun_immunity_duration");
    m_timer8292.Start(survivor_stun_immunity_duration.GetFloat());

    OnPounceEnded();

    CHandle<ITerrorPlayer>& m_tongueVictim = access_member<CHandle<ITerrorPlayer>>(this, 8512);
    if(m_tongueVictim != NULL)
    {
        m_tongueVictim->AbilityDebug(m_tongueVictim, "Tongue letting go on getting bashed.");
    }

    ReleaseTongueVictim(false);
}

void ITerrorPlayer::EmitInfectedPainSound(const CTakeDamageInfo &info)
{
    CountdownTimers &m_timer7708 = access_member<CountdownTimers>(this, 7708);
    if(m_timer7708.IsElapsed())
    {
        m_timer7708.Start(0.7f);

        switch (GetClass())
        {
            case ZombieClassSmoker:
            {
                if((info.GetDamageType() & 0x82) != 0)
                    Vocalize("SmokerZombie.PainShort", 0.f, 0.f);
                else
                    Vocalize("SmokerZombie.Pain", 0.f, 0.f);

                break;
            }
            case ZombieClassBommer:
            {
                if((info.GetDamageType() & 0x82) != 0)
                    Vocalize("BoomerZombie.PainShort", 0.f, 0.f);
                else
                    Vocalize("BoomerZombie.Pain", 0.f, 0.f);

                break;
            }
            case ZombieClassHunter:
            {
                if((info.GetDamageType() & 0x82) != 0)
                    Vocalize("HunterZombie.PainShort", 0.f, 0.f);
                else
                    Vocalize("HunterZombie.Pain", 0.f, 0.f);

                break;
            }
            case ZombieClassTank:
            {
                if(!IsIncapacitated())
                {
                    Vocalize("HulkZombie.Pain", 0.7f, 0.f);
                }
                break;
            }
        }
    }
}

void ITerrorPlayer::Vocalize(const char *szName, float flDuration, float val2)
{
    CountdownTimers &m_timer7696 = access_member<CountdownTimers>(this, 7696);

    if(flDuration < 0)
    {
        EmitSound(szName);
    }
    else if(flDuration == 0)
    {
        float _flDuration = 0.0f;
        EmitSound(szName, 0.f, &_flDuration);

        if(flDuration > _flDuration)
            m_timer7696.Start(flDuration + 0.2f);
        else
            m_timer7696.Start(_flDuration + 0.2f);
    }
    else if(flDuration > 0)
    {
        EmitSound(szName);
        m_timer7696.Start(flDuration);
    }

    if(val2 > 0.f)
    {
        g_CallHelper->CSoundEnt_InsertSound(8, GetAbsOrigin(), 2048, val2, (CBaseEntity*)this, 10);
    }
}

bool ITerrorPlayer::IsMotionControlledXY(Activity activity)
{
    bool IsMotionXY = false;
    switch (activity)
    {
        case ACT_TERROR_SHOVED_FORWARD:
        case ACT_TERROR_SHOVED_BACKWARD:
        case ACT_TERROR_SHOVED_LEFTWARD:
        case ACT_TERROR_SHOVED_RIGHTWARD:
        case ACT_TERROR_TUGGED_FORWARD:
        case ACT_TERROR_TUGGED_BACKWARD:
        case ACT_TERROR_TUGGED_LEFTWARD:
        case ACT_TERROR_TUGGED_RIGHTWARD:
        case ACT_TERROR_DIE_FROM_STAND:
        case ACT_TERROR_DIE_WHILE_RUNNING:
        case ACT_TERROR_LEDGE_CLIMB:
        case ACT_TERROR_LEDGE_CLIMB_TO_CROUCH:
        case ACT_TERROR_CLIMB_36_FROM_STAND:
        case ACT_TERROR_CLIMB_70_FROM_STAND:
        case ACT_TERROR_CLIMB_115_FROM_STAND:
        case ACT_TERROR_CLIMB_130_FROM_STAND:
        case ACT_TERROR_CLIMB_150_FROM_STAND:
        case ACT_TERROR_CLIMB_166_FROM_STAND:
        case ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_L:
        case ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_R:
        case ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_BACKWARD:
        case ACT_TERROR_HUNTER_POUNCE_KNOCKOFF_FORWARD:
        case ACT_TERROR_HULK_VICTORY:
        case ACT_TERROR_HULK_VICTORY_B:
        case ACT_TERROR_RAGE_AT_ENEMY:
        case ACT_TERROR_RAGE_AT_KNOCKDOWN:
        {
            IsMotionXY = true;
            break;
        }
        default:
        {
            IsMotionXY = false;
            break;
        }
    }

    return IsMotionXY;
}

bool ITerrorPlayer::IsMotionControlledZ(Activity activity)
{
    bool bIsMotionZ = false;
    switch(activity)
    {
        case ACT_TERROR_LEDGE_CLIMB:
        case ACT_TERROR_LEDGE_CLIMB_TO_CROUCH:
        case ACT_TERROR_CLIMB_36_FROM_STAND:
        case ACT_TERROR_CLIMB_70_FROM_STAND:
        case ACT_TERROR_CLIMB_115_FROM_STAND:
        case ACT_TERROR_CLIMB_130_FROM_STAND:
        case ACT_TERROR_CLIMB_150_FROM_STAND:
        case ACT_TERROR_CLIMB_166_FROM_STAND:
        {
            bIsMotionZ = true;
            break;
        }
        default:
        {
            bIsMotionZ = false;
            break;
        }    
    }

    return bIsMotionZ;
}

bool ITerrorPlayer::IsRenderYawOverridden()
{
    Activity &activity = access_member<Activity>(this, 1615*4);
    CHandle<ITerrorPlayer> &m_pounceVictim = access_member<CHandle<ITerrorPlayer>>(this, 10864);
    CHandle<ITerrorPlayer> &m_pounceAttacker = access_member<CHandle<ITerrorPlayer>>(this, 10868);
    CHandle<ITerrorPlayer> &m_tongueOwner = access_member<CHandle<ITerrorPlayer>>(this, 8516);
    CHandle<ITerrorPlayer> &m_tongueVictim = access_member<CHandle<ITerrorPlayer>>(this, 8512);
    CHandle<ITerrorPlayer> &m_healTarget = access_member<CHandle<ITerrorPlayer>>(this, 1596*4);
    CHandle<ITerrorPlayer> &m_reviveTarget = access_member<CHandle<ITerrorPlayer>>(this, 1765*4);

    if(activity >= 1077 && (activity <= 1079 || (activity - 1258) <= 3))
        return true;

    if(m_pounceVictim != NULL)
        return true;

    if(m_pounceAttacker != NULL)
        return true;

    if(IsIncapacitated() )
        return true;

    if( m_tongueOwner != NULL)
        return true;

    if(m_tongueVictim != NULL)
        return true;

    if(IsMotionControlledZ(activity))
        return true;

    if(m_healTarget != NULL && m_healTarget == this)
        return true;

    if(m_reviveTarget != NULL)
        return true;

    return false;
}

void ITerrorPlayer::AbilityDebug(ITerrorPlayer *pVictim, const char *msg, ...)
{
    char v6[4108];
    va_list v;
    IBaseAbility *pAbility = GetCustomAbility();
    if(pAbility != nullptr)
    {
        va_start(v, msg);
        Q_vsnprintf(v6, 0x1000, msg, v);
        pAbility->AbilityDebug((CTerrorPlayer*)pVictim, v6);
    }
}

NOINLINE ITerrorPlayer *ITerrorPlayer::GetHealTarget()
{
    CHandle<ITerrorPlayer> &m_healTarget = access_member<CHandle<ITerrorPlayer>>(this, 1596*4);
    if(m_healTarget != NULL)
        return m_healTarget;

    return nullptr;
}

NOINLINE ITerrorPlayer *ITerrorPlayer::GetPounceVictim()
{
    CHandle<ITerrorPlayer> &m_pounceVictim = access_member<CHandle<ITerrorPlayer>>(this, 2716*4); // 10864
    if(m_pounceVictim != NULL)
        return m_pounceVictim;

    return nullptr;
}

NOINLINE ITerrorPlayer *ITerrorPlayer::GetTongueVictim()
{
    CHandle<ITerrorPlayer> &m_tongueVictim = access_member<CHandle<ITerrorPlayer>>(this, 8512);
    if(m_tongueVictim != NULL)
        return m_tongueVictim;

    return nullptr;
}

NOINLINE IBaseAbility *ITerrorPlayer::GetCustomAbility()
{
    CHandle<IBaseAbility> &m_customAbility = access_member<CHandle<IBaseAbility>>(this, 1934*4);
    if(m_customAbility != NULL)
        return m_customAbility;

    return nullptr;
}

bool ITerrorPlayer::IsPlayingDeathAnim() const
{
    Activity &activity = access_member<Activity>(const_cast<ITerrorPlayer*>(this), 1615*4);
    return activity == ACT_TERROR_DIE_FROM_STAND || activity == ACT_TERROR_DIE_WHILE_RUNNING;
}
