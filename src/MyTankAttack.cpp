#include "MyTank.h"
#include "Interface/IBaseAbility.h"

TankAttack::TankAttack() : m_path(IChasePath::LEAD_SUBJECT),
    cv_stuckFailsafe("tank_stuck_failsafe"), 
    cv_stasisSuicideTime("tank_stasis_time_suicide"),
    cv_stuckNewTargetTime("tank_stuck_time_choose_new_target"),
    cv_stuckNewTargetVisTol("tank_stuck_visibility_tolerance_choose_new_target"),
    cv_stuckSuicideTime("tank_stuck_time_suicide"),
    cv_stuckSuicideVisTol("tank_stuck_visibility_tolerance_suicide"),
    cv_visSuicideTol("tank_visibility_tolerance_suicide"),
    cv_allowAiAbilities("z_allow_ai_to_use_abilities"),
    cv_throwLoftRate("tank_throw_loft_rate"),
    cv_throwMaxLoft("tank_throw_max_loft_angle"),
    cv_throwForce("z_tank_throw_force"),
    cv_throwLeadFactor("tank_throw_lead_time_factor"),
    cv_throwAimError("tank_throw_aim_error"),
    cv_throwRange("tank_throw_allow_range"),
    cv_throwInterval("tank_throw_min_interval")
{
    m_hPlayer.Term();
}

TankAttack::~TankAttack()
{
}

EventDesiredResult<ITank> TankAttack::OnCommandAttack(ITank *me, CBaseEntity *victim)
{
    if(victim) {
        IBaseEntity *pPlayer = GetVirtualClass<IBaseEntity>(victim);
        m_hPlayer.Term();
        if(pPlayer->IsPlayer()) {
            m_hPlayer = pPlayer->GetRefEHandle();
        }
    }

    return TryContinue();
}

EventDesiredResult<ITank> TankAttack::OnMoveToSuccess(ITank *me, const Path *path)
{
    m_stuckTimer.Invalidate();
    return TryContinue();
}

EventDesiredResult<ITank> TankAttack::OnUnStuck(ITank *me)
{
    m_stuckTimer.Invalidate();
    return TryContinue();
}

EventDesiredResult<ITank> TankAttack::OnShoved(ITank *me, CBaseEntity *pusher)
{
    m_stuckTimer.Invalidate();
    return TryToSustain();
}

EventDesiredResult<ITank> TankAttack::OnMoveToFailure(ITank *me, const Path *path, MoveToFailureType reason)
{
    if(!m_stuckTimer.HasStarted())
        m_stuckTimer.Start();

    return TryContinue();
}

EventDesiredResult<ITank> TankAttack::OnStuck(ITank *me)
{
    if(!m_stuckTimer.HasStarted())
        m_stuckTimer.Start();

    return TryContinue();
}

EventDesiredResult<ITank> TankAttack::OnContact(ITank *me, CBaseEntity *other, CGameTrace *result)
{
    ITerrorPlayer* pPlayer = GetVirtualClass<ITerrorPlayer>(other);
    if(pPlayer && pPlayer->IsPlayer())
    {
        m_stuckTimer.Invalidate();

        if(pPlayer->GetTeamNumber() == 2 && !pPlayer->IsIncapacitated())
        {
            if(m_hPlayer != NULL)
            {
                if(!m_hPlayer->IsPlayer())
                {
                    SetTarget(pPlayer);
                    return TryToSustain();
                }

                if(access_member<CountdownTimers>(GetTarget(), 2043*4).IsElapsed())
                {
                    SetTarget(pPlayer);
                }
            }
            else
            {
                SetTarget(pPlayer);
            }
        }
    }

    return TryToSustain();
}

EventDesiredResult<ITank> TankAttack::OnInjured(ITank *me, const CTakeDamageInfo &info)
{
    IBaseEntity* pEntity = (IBaseEntity*)info.GetAttacker();
    if((info.GetDamageType() & DMG_BURN) != 0 && pEntity)
    {
        if(pEntity->IsPlayer())
        {
            m_stuckTimer.Invalidate();
            CZombieIgnite ignite;
            ignite.Set(engine->GetPlayerUserId(pEntity->edict()), me->entindex(), "Tank");
        }
    }

    return TryToSustain();
}

EventDesiredResult<ITank> TankAttack::OnCommandApproach(ITank *me, const Vector &pos, float maxDistance)
{
    return TrySuspendFor(new BehaviorMoveTo<ITank, InfectedPathCost>(pos), RESULT_CRITICAL, "Debug move to");
}

ActionResult<ITank> TankAttack::OnStart(ITank *me, Action<ITank> *priorAction)
{
    IBody *pBody = me->GetBodyInterface();
    pBody->SetArousal(IBody::INTENSE);
    pBody->SetDesiredPosture(IBody::STAND);

    m_attackTimer.Start();
    m_stuckTimer.Invalidate();
    m_throwCooldown.Invalidate();
    m_blockLader.Init(me);

    UpdateThrowAimError();

    TankAttackMusic music;
    ForEachTerrorPlayer(music);

    return Continue();
}

bool IsFinaleVehicleReady(void *pThis)
{
    if(access_member<bool>(pThis, 358))
    {
        if(access_member<int>(pThis, 102*4) == 1)
        {
            return access_member<float>(pThis, 146*4) <= 0.f;
        }
    }

    return false;
}

ActionResult<ITank> TankAttack::Update(ITank *me, float interval)
{
    auto KillMe = [this, me]() -> ActionResult<ITank>
    {
        me->LeaveStasis();
        DevMsg("Tank committing suicide\n");
        CTakeDamageInfoHack info(me, me, me->GetHealth(), DMG_BULLET);
        me->TakeDamage(info);
        m_stuckTimer.Invalidate();
        return Done("Killing self");
    };

    auto ShouldSuicide = [&]() -> bool
    {
        float visTime = me->GetVisionInterface()->GetTimeSinceVisible(2);

        void* pDirector = g_HL2->GetDirector();
        extern bool IsFinaleEscapeInProgress(void *pThis);
        
        if (access_member<bool>(pDirector, 1880) &&
            (IsFinaleVehicleReady(pDirector) || IsFinaleEscapeInProgress(pDirector)))
        {
            visTime = 0.f;
        }

        // ===== Phase 0: visibility-only suicide =====
        if (visTime != FLT_MAX &&
            visTime > cv_visSuicideTol.GetFloat())
        {
            return true;
        }

        // ===== Phase 1: stuck timer not started =====
        if (!m_stuckTimer.HasStarted())
        {
            return false;
        }

        float stuckTime = m_stuckTimer.GetElapsedTime();

        // ===== Phase 2: still stuck? =====
        bool stillStuck =
            !m_stuckTimer.IsGreaterThen(cv_stuckNewTargetTime.GetFloat()) ||
            visTime >= cv_stuckNewTargetVisTol.GetFloat();

        if (!stillStuck)
        {
            // phase transition: unstuck
            m_stuckTimer.Invalidate();
            m_path.Invalidate();
            m_hPlayer.Term();
            return false;
        }

        // ===== Phase 3: long enough? =====
        if (stuckTime <= cv_stuckSuicideTime.GetFloat())
        {
            return false;
        }

        // ===== Phase 4: visibility tolerance =====
        if (visTime == FLT_MAX ||
            visTime > cv_stuckSuicideVisTol.GetFloat())
        {
            return true;
        }

        return false;
    };

    TankLocomotion *pLocomotion = me->GetLocomotionInterface();
    if(cv_stuckFailsafe.GetBool())
    {
        if(me->IsMotionControlledXY(access_member<Activity>(me, 1615*4)) 
        || me->IsMotionControlledZ(access_member<Activity>(me, 1615*4)))
        {
            pLocomotion->ClearStuckStatus("Motion controlled animation");
            m_stuckTimer.Invalidate();
        }

        if(access_member<bool>(me, 11973))
        {
            pLocomotion->ClearStuckStatus("In Stasis");
            m_stuckTimer.Invalidate();

            if(!m_stasisTimer.HasStarted())
            {
                m_stasisTimer.Start();
            }
            else if(m_stasisTimer.IsGreaterThen(cv_stasisSuicideTime.GetFloat()))
            {
                m_stasisTimer.Invalidate();
                return KillMe();
            }
        }
        
        if(ShouldSuicide())
        {
            return KillMe();
        }
    }

    if(access_member<bool>(me, 11973))
    {
        return Continue();
    }

    m_blockLader.UpdateTimer();

    INextBot *pNextBot = me->MyNextBotPointer();
    IBody *pBody = me->GetBodyInterface();
    if(!pLocomotion->IsUsingLadder())
    {
        if(m_hPlayer == NULL)
        {
            MinigunnerScan scan;
            ForEachTerrorPlayer(scan);
            SetTarget(scan.GetBestPlayer() != nullptr ? scan.GetBestPlayer() : me->ChooseVictim(m_hPlayer));

            if(m_hPlayer == NULL || !m_hPlayer->IsAlive())
            {
                return Continue();
            }

            TryToThrowRock(me);
            pBody->AimHeadTowards((CBaseEntity*)GetTarget(), IBody::IMPORTANT, 1.f, nullptr, "Looking at my victim");
            
            if(pNextBot->IsRangeGreaterThan((CBaseEntity*)GetTarget(), 30) 
            || !pLocomotion->IsPotentiallyTraversable(me->GetAbsOrigin(), m_hPlayer->GetAbsOrigin(), ILocomotion::EVENTUALLY))
            {
                InfectedPathCost cost(pNextBot);
                m_path.Update(pNextBot, GetTarget(), cost);
                if(!m_path.IsValid())
                {
                    pLocomotion->Approach(m_hPlayer->GetAbsOrigin());
                }
            }

            return Continue();
        }
    }
    else
    {
        ForEachTerrorPlayer(m_blockLader);

        if(m_hPlayer != NULL && m_path.IsValid())
        {
            InfectedPathCost cost(pNextBot);
            m_path.Update(pNextBot, GetTarget(), cost);
            return Continue();
        }
    }

    if(m_hPlayer != NULL)
    {
        IBaseCombatCharacter* pEnt = reinterpret_cast<IBaseCombatCharacter*>(me->GetEntity());
        int &activity = access_member<int>(pEnt, 6460);
        if(activity == 1286 || activity == 1287)
        {
            Vector vecPos = m_hPlayer->GetAbsOrigin() - pNextBot->GetPosition();
            QAngle angle;
            VectorAngles(vecPos, angle);
            pEnt->SetAbsAngles(angle);
        }
    }

    CHandle<IBaseAbility> &hAbility = access_member<CHandle<IBaseAbility>>(me, 1934*4);
    if(hAbility != NULL && !hAbility->IsActive())
    {
        MinigunnerScan scan;
        ForEachTerrorPlayer(scan);
        SetTarget(scan.GetBestPlayer() != nullptr ? scan.GetBestPlayer() : me->ChooseVictim(m_hPlayer));
    }

    if(m_hPlayer == NULL || !m_hPlayer->IsAlive())
    {
        return Continue();
    }

    TryToThrowRock(me);
    pBody->AimHeadTowards((CBaseEntity*)GetTarget(), IBody::IMPORTANT, 1.f, nullptr, "Looking at my victim");
    
    if(pNextBot->IsRangeGreaterThan((CBaseEntity*)GetTarget(), 30) 
    || !pLocomotion->IsPotentiallyTraversable(me->GetAbsOrigin(), m_hPlayer->GetAbsOrigin(), ILocomotion::EVENTUALLY))
    {
        InfectedPathCost cost(pNextBot);
        m_path.Update(pNextBot, GetTarget(), cost);
        if(!m_path.IsValid())
        {
            pLocomotion->Approach(m_hPlayer->GetAbsOrigin());
        }
    }
    
    return Continue();
}

inline void TankAttack::UpdateThrowAimError(void)
{
    if(GetDifficulty())
        m_throwAimError = 0;
    else
        m_throwAimError = RandomFloat(-M_PI, M_PI) * 0.5f;
}

NOINLINE ITerrorPlayer *TankAttack::GetTarget(void) const
{
    return m_hPlayer != NULL ? m_hPlayer.Get() : NULL;
}

inline void TankAttack::SetTarget(ITerrorPlayer *pTarget)
{
    if(pTarget) {
        m_hPlayer = pTarget->GetRefEHandle();
    } else {
        m_hPlayer = NULL;
    }
}

void TankAttack::TryToThrowRock(ITank *me)
{
    if(!cv_allowAiAbilities.GetBool())
    {
        return;
    }

    void *pDirector = g_HL2->GetDirector();
    if( !access_member<bool>(pDirector, 358) )
    {
		if(!GetDifficulty() || GetDifficulty() == 1 )
		{
			if(m_attackTimer.IsLessThen(10.f))
				return;
		}
	}

	CHandle<IBaseAbility> hAbility = access_member<CHandle<IBaseAbility>>(me, 1934*4);
	if( hAbility == NULL )
	{
		return;
	}

    TankLocomotion *pLocomotion = me->GetLocomotionInterface();

	if(!m_throwCooldown.IsElapsed())
	{
		if(hAbility != NULL && hAbility->IsActive())
		{
			float flDistSqr = me->GetAbsOrigin().DistTo(m_hPlayer->GetAbsOrigin());
			float flThrowRate = flDistSqr * cv_throwLoftRate.GetFloat();
			if(flThrowRate > cv_throwMaxLoft.GetFloat() )
			{
				flThrowRate = cv_throwMaxLoft.GetFloat();
			}

			Vector throwAim;
			if(GetDifficulty())
			{
				float flTimeFactor = flDistSqr / cv_throwForce.GetFloat() * cv_throwLeadFactor.GetFloat();
				throwAim.x = m_hPlayer->GetAbsVelocity().x * flTimeFactor;
				throwAim.y = flTimeFactor * m_hPlayer->GetAbsVelocity().y;
				throwAim.z = 0.f;
			}
			else
			{
				Vector vecDelta = (m_hPlayer->GetAbsOrigin() - me->GetAbsOrigin());
				Vector ring;
				Vector up;
				VectorVectors(vecDelta, ring, up);
				float flSin = 0.f;
				float flCos = 0.f;
				FastSinCos(m_throwAimError, &flSin, &flCos);
				
				throwAim = (up * flCos + ring * flSin) * cv_throwAimError.GetFloat();
			}

			throwAim.z = tan(flThrowRate * 0.017453292f) * flDistSqr + throwAim.z;
			me->GetBodyInterface()->AimHeadTowards((GetTarget()->WorldSpaceCenter() + throwAim), IBody::CRITICAL, 4.0f, nullptr, "Throw loft angle");
			pLocomotion->ClearStuckStatus("Throwing rock");
			return;
		}
	}
	else if(!me->IsDucked())
	{
        IZombieBotVision *pVision = me->GetVisionInterface();
        INextBot *pNextBot = me->MyNextBotPointer();

		if(!pLocomotion->IsOnGround()) return;
        if(pLocomotion->IsClimbingOrJumping()) return;
        if(m_hPlayer == NULL || !m_hPlayer->IsPlayer()) return;
        if(m_hPlayer == NULL || m_hPlayer->IsIncapacitated()) return;
        if(pNextBot->IsRangeLessThan((CBaseEntity*)GetTarget(), cv_throwRange.GetFloat())) return;
        if(!pVision->IsLineOfSightClearToEntity((CBaseEntity*)GetTarget())) return;
        if(hAbility == NULL || !hAbility->IsAbilityReadyToFire()) return;
        if(hAbility == NULL || !hAbility->HasAbilityTarget()) return;
        if(!pVision->IsLookingAt((CBaseCombatCharacter*)GetTarget())) return;

        me->PressMeleeButton();
        m_throwCooldown.Start(cv_throwInterval.GetFloat());
        UpdateThrowAimError();
        pLocomotion->ClearStuckStatus("Throwing rock");
	}
    return;
}

const char *sDifficultyStrings[] = { "Easy", "Normal", "Hard", "Impossible" };
const int TankAttack::GetDifficulty() const
{
    ConVarRef z_difficulty("z_difficulty");
    const char *difficult = nullptr;
    if(z_difficulty.IsFlagSet(FCVAR_NEVER_AS_STRING))
    {
        difficult = "FCVAR_NEVER_AS_STRING";
    }
    else
    {
        difficult = "";
        if(z_difficulty.GetString())
        {
            difficult = z_difficulty.GetString();
        }
    }

    for(int i = 0; i < 4; ++i)
    {
        if(difficult == sDifficultyStrings[i] || g_Sample.my_bStrcmp(difficult, sDifficultyStrings[i]))
        {
            return i;
        }
    }

    return 1;
}