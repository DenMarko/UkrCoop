#include "MyWitch.h"

WanderingWitchPathCost::WanderingWitchPathCost(INextBot *pBot, IWitchLocomotion *pLocomotion)
 : m_pBot(pBot), m_pLocomotion(pLocomotion)
{
}

// Основний оператор розрахунку вартості
float WanderingWitchPathCost::operator()(INavArea* area, INavArea* fromArea, 
                                                const INavLadder* ladder, const CFuncElevator* elevator, 
                                                float length) const
{
    // Базова перевірка - якщо немає початкової області
    if (!fromArea) {
        return BASE_COST;
    }

    // Перевірка доступності області для ходячої Witch
    if (!IsAreaSuitableForWandering(area)) {
        return INVALID_PATH_COST;
    }

    // Обробка руху по драбинах
    if (ladder) {
        return CalculateLadderCost(area, fromArea, ladder);
    }

    // Розрахунок вартості переходу
    return CalculateTraversalCost(area, fromArea, length);
}

float WanderingWitchPathCost::CalculateLadderCost(INavArea* area, INavArea* fromArea, 
                                                        const INavLadder* ladder) const
{
    if (!ladder || !CanUseLadder(ladder)) {
        return INVALID_PATH_COST;
    }

    // Визначаємо напрямок руху по драбині
    bool isMovingUp = fromArea->GetCenter().z < area->GetCenter().z;
    
    // Базова вартість
    float baseCost = LADDER_BASE_COST;
    
    // Застосовуємо штраф залежно від напрямку
    if (isMovingUp) {
        baseCost *= LADDER_UPWARD_MULTIPLIER;  // Підйом важчий
    } else {
        baseCost *= LADDER_DOWNWARD_MULTIPLIER; // Спуск легший
    }
    
    // Множимо на довжину драбини
    float ladderLength = ladder->m_length;
    float totalCost = baseCost * ladderLength;
    
    // Додаємо накопичену вартість
    return totalCost + fromArea->GetCostSoFar();
}

// Перевірка можливості використання драбини
bool WanderingWitchPathCost::CanUseLadder(const INavLadder* ladder) const
{
    if (!ladder) {
        return false;
    }
    
    // Перевірка довжини драбини
    if (ladder->m_length > MAX_LADDER_LENGTH) {
        return false; // Занадто довга драбина
    }
    
    return true;
}

// Перевірка придатності області для блукання
bool WanderingWitchPathCost::IsAreaSuitableForWandering(INavArea* area) const
{
    if (!area) {
        return false;
    }

    // Перевірка заборонених атрибутів
    if (HasForbiddenAttributes(area)) {
        return false;
    }

    // Перевірка можливості проходження
    if (!IsAreaTraversable(area)) {
        return false;
    }

    return true;
}

// Перевірка заборонених атрибутів
bool WanderingWitchPathCost::HasForbiddenAttributes(INavArea* area) const
{
    // Ходяча Witch не може стрибати
    if (area->HasAttributes(NAV_MESH_NO_JUMP)) {
        return true;
    }

    // Перевірка спеціальних spawn атрибутів
    if (area->HasSpawnAttributes(SPAWN_ATTRIBUTE_FORBIDDEN)) {
        return true;
    }

    return false;
}

// Перевірка можливості проходження області
bool WanderingWitchPathCost::IsAreaTraversable(INavArea* area) const
{
    if (!m_pLocomotion) {
        return false;
    }

    return m_pLocomotion->IsAreaTraversable((CNavArea*)area);
}

// Розрахунок вартості переходу
float WanderingWitchPathCost::CalculateTraversalCost(INavArea* area, INavArea* fromArea, float providedLength) const
{
    float heightDifference = GetHeightDifference(area, fromArea);
    
    // Ходяча Witch може долати тільки невеликі висоти
    if (!CanTraverseHeight(heightDifference)) {
        return INVALID_PATH_COST;
    }

    // Розраховуємо базову дистанцію
    float distance = CalculateDistance(area, fromArea, providedLength);
    
    // Застосовуємо штраф за висоту
    distance = ApplyHeightPenalty(distance, heightDifference);
    
    // Додаємо накопичену вартість від попередніх областей
    return distance + fromArea->GetCostSoFar();
}

// Отримання різниці висот між областями
float WanderingWitchPathCost::GetHeightDifference(INavArea* area, INavArea* fromArea) const
{
    return fromArea->GetZDeltaAtEdgeToArea(area);
}

// Перевірка можливості подолання висоти
bool WanderingWitchPathCost::CanTraverseHeight(float heightDifference) const
{
    // Ходяча Witch обмежена у вертикальному русі
    return fabs(heightDifference) <= MAX_TRAVERSABLE_HEIGHT;
}

// Розрахунок дистанції між областями
float WanderingWitchPathCost::CalculateDistance(INavArea* area, INavArea* fromArea, float providedLength) const
{
    if (providedLength > 0.0f) {
        return providedLength;
    }

    // Розраховуємо відстань між центрами областей
    return (area->GetCenter() - fromArea->GetCenter()).Length();
}

// Застосування штрафу за висоту
float WanderingWitchPathCost::ApplyHeightPenalty(float baseDistance, float heightDifference) const
{
    // Обробляємо рух вниз з особливою обережністю
    float adjustedHeight = heightDifference;
    if (IsMovingDownward(heightDifference)) {
        adjustedHeight = ApplyDownwardPenalty(heightDifference);
    }

    // Розраховуємо штраф залежно від висоти
    float heightCost = CalculateHeightCost(adjustedHeight);
    
    // Додаємо штраф тільки якщо він значний
    if (IsSignificantCost(heightCost)) {
        baseDistance += heightCost;
    }

    return baseDistance;
}

// Перевірка руху вниз
bool WanderingWitchPathCost::IsMovingDownward(float heightDifference) const
{
    return heightDifference < 0.0f;
}

// Застосування штрафу за рух вниз
float WanderingWitchPathCost::ApplyDownwardPenalty(float heightDifference) const
{
    // Ходяча Witch особливо обережна при русі вниз
    return heightDifference * DOWNWARD_PENALTY_MULTIPLIER;
}

// Розрахунок вартості висоти
float WanderingWitchPathCost::CalculateHeightCost(float adjustedHeight) const
{
    float heightPenalty = (fabs(adjustedHeight) <= HEIGHT_THRESHOLD) ? 
        LOW_HEIGHT_PENALTY : 
        HIGH_HEIGHT_PENALTY;

    return fabs(adjustedHeight * heightPenalty);
}

// Перевірка значимості вартості
bool WanderingWitchPathCost::IsSignificantCost(float cost) const
{
    return cost > MINIMUM_COST_THRESHOLD;
}

class NavAreaCollector
{
	bool m_checkForDuplicates;
public:
	NavAreaCollector( bool checkForDuplicates = false ) : m_checkForDuplicates(checkForDuplicates) { }

	bool operator() ( INavArea *area )
	{
		if ( m_checkForDuplicates && m_area.HasElement( area ) )
			return true;

		m_area.AddToTail( area );
		return true;
	}
	CUtlVector< INavArea * > m_area;
};

ConVar z_witch_wander_hear_radius("z_witch_wander_hear_radius", "72", FCVAR_CHEAT);

WitchWander::WitchWander() : z_witch_speed("z_witch_speed") { }

ActionResult<IWitch> WitchWander::OnStart(IWitch *me, Action<IWitch> *priorAction)
{
    float m_rang = access_member<float>(me, 3632);
    if(m_rang > 0.f)
    {
        return ChangeTo(new WitchWanderAngry(0.f));
    }
    
    me->SetHarasser(nullptr);
    IWitchBody *pBody = me->GetBodyInterface();
    pBody->SetArousal(IBody::ArousalType::NEUTRAL);
    if(!pBody->StartActivity(ACT_TERROR_IDLE_NEUTRAL))
        return ChangeTo(new WitchWanderAngry(0.f));

    pBody->SetDesiredPosture(IBody::PostureType::STAND);

    m_growlTimer.Start(2.f);

    float flRand = RandomFloat(0.f, 1.f);
    m_pathRebuildTimer.Start(flRand);
    m_WarningBuildCount = 0;
    m_Victim.Term();
    m_Victim_sound.Invalidate();

    me->GetLocomotionInterface()->Stop();

    return Continue();
}

void WitchWander::OnEnd(IWitch *me, Action<IWitch> *nextAction)
{
    me->GetLocomotionInterface()->SetSpeedLimit(z_witch_speed.GetFloat());
}

ActionResult<IWitch> WitchWander::Update(IWitch *me, float interval)
{
    if(m_growlTimer.IsElapsed())
    {
        me->EmitSound("WitchZombie.Despair");
        float flRandVal = RandomFloat(2.f, 4.f);
        m_growlTimer.Start(flRandVal);
        g_CallHelper->CSoundEnt_InsertSound(SOUND_DANGER, me->GetAbsOrigin(), 2048, flRandVal, (CBaseEntity*)me, 10);
    }

    float flRang = 0.f;
    INextBot* pBot = me->MyNextBotPointer();
    IWitchBody *pBody = me->GetBodyInterface();
    IWitchVision* pVision = me->GetVisionInterface();
    ITerrorPlayer* pPlayer = (ITerrorPlayer*)pVision->GetPrimaryRecognizedThreat();
    if(!pPlayer)
    {
        if(m_Victim_sound.HasStarted())
        {
            if(m_Victim_sound.IsElapsed())
            {
                pPlayer = m_Victim != NULL ? m_Victim.Get() : nullptr;
                m_Victim_sound.Invalidate();
            }
        }

        if(!pPlayer)
        {
            pPlayer = GetClosestSurvivor(pBot->GetPosition());
            if(!pPlayer 
            || !pBot->IsRangeLessThan((CBaseEntity*)pPlayer, z_witch_wander_personal_space.GetFloat()) 
            || !pVision->IsLineOfSightClearToEntity((CBaseEntity*)pPlayer))
            {
                UpdatePath(me);
                return Continue();
            }
        }

        pBody->AimHeadTowards((CBaseEntity*)pPlayer, IBody::IMPORTANT, 1.f);
        me->EmitSound("WitchZombie.Surprised");
        flRang = 0.f;
    }

    float flRangToEnemy = me->IsHostileToMe(pPlayer) ? (z_witch_wander_personal_space.GetFloat() * 2) : z_witch_wander_personal_space.GetFloat();
    if( !pBot->IsRangeLessThan((CBaseEntity*)pPlayer, flRangToEnemy) )
    {
        UpdatePath(me);
        return Continue();
    } 

    if(!pBody->IsActivity(ACT_TERROR_IDLE_NEUTRAL))
        pBody->StartActivity(ACT_TERROR_IDLE_NEUTRAL);

    m_path.Invalidate();
    IWitchLocomotion* pLocomotion = me->GetLocomotionInterface();
    pLocomotion->Stop();
    pLocomotion->SetVelocity(vec3_origin);
    me->SetAbsVelocity(vec3_origin);

    return ChangeTo(new WitchWanderAngry(flRang));
}

void WitchWander::UpdatePath(IWitch *me)
{
    IWitchLocomotion *pLocomotion = me->GetLocomotionInterface();
    IWitchBody *pBody = me->GetBodyInterface();

    if(m_path.IsValid())
    {
        m_path.Update(me->MyNextBotPointer());
        if(!pLocomotion->IsClimbingOrJumping() && !pLocomotion->IsUsingLadder() && pLocomotion->IsOnGround())
        {
            if (m_lastPosition == me->GetAbsOrigin())
            {
                m_path.Invalidate();
                m_pathRebuildTimer.Start(RandomFloat(1.f, 2.f));
            } else {
                m_lastPosition = me->GetAbsOrigin();
            }

            if (!pBody->IsActivity(ACT_TERROR_WALK_NEUTRAL))
            {
                pBody->StartActivity(ACT_TERROR_WALK_NEUTRAL);
                pLocomotion->SetSpeedLimit(35.f);
                m_lastPosition = vec3_origin;
            }
        }
    }
    else
    {
        if (!pBody->IsActivity(ACT_TERROR_IDLE_NEUTRAL))
        {
            pBody->StartActivity(ACT_TERROR_IDLE_NEUTRAL);
            pLocomotion->Stop();
            pLocomotion->SetVelocity(vec3_origin);
            me->SetAbsVelocity(vec3_origin);
        }

        if (m_pathRebuildTimer.IsElapsed())
        {
            if (!BuildWanderPath(me))
            {
                float flRand = RandomFloat(2.f, 4.f);
                m_pathRebuildTimer.Start(flRand);
            }
        }
    }
}

ActionResult<IWitch> WitchWander::OnSuspend(IWitch *me, Action<IWitch> *interruptingAction)
{
    IWitchLocomotion *pLocomotion = me->GetLocomotionInterface();
    pLocomotion->SetSpeedLimit(z_witch_speed.GetFloat());
    return Continue();
}

ActionResult<IWitch> WitchWander::OnResume(IWitch *me, Action<IWitch> *interruptingAction)
{
    m_path.Invalidate();
    float flRand = RandomFloat(1.f, 2.f);
    m_pathRebuildTimer.Start(flRand);

    return Continue();
}

EventDesiredResult<IWitch> WitchWander::OnShoved(IWitch *me, CBaseEntity *pusher)
{
    if(!pusher) {
        return TryContinue();
    }

    ITerrorPlayer* pTerror = nullptr;
    if((pTerror = access_dynamic_cast<ITerrorPlayer>((IBasePlayer*)pusher, "CTerrorPlayer")) != nullptr && pTerror->GetTeamNumber() == 3)
    {
        return TryContinue();
    }

    if(g_pWitchEvil->IsWitchEvil(me->entindex()))
    {
        return TrySuspendFor(new WitchEvilAttack((ITerrorPlayer*)pusher), RESULT_IMPORTANT);
    }
    
    return TrySuspendFor(new WitchAttack((ITerrorPlayer*)pusher), RESULT_IMPORTANT);
}

EventDesiredResult<IWitch> WitchWander::OnAnimationEvent(IWitch *me, animevent_t *event)
{
    if(event->event == 45)
    {
        me->DoAttack(nullptr);
    }
    
    return TryContinue();
}

EventDesiredResult<IWitch> WitchWander::OnCommandAttack(IWitch *me, CBaseEntity *victim)
{
    float m_rage = access_member<float>(me, 3632);
    if(m_rage < 1.0f)
    {
        return TryContinue();
    }

    if(g_pWitchEvil->IsWitchEvil(me->entindex()))
    {
        return TrySuspendFor(new WitchEvilAttack((ITerrorPlayer*)victim), RESULT_TRY, "I was commanded to");
    }

    return TrySuspendFor(new WitchAttack((ITerrorPlayer*)victim), RESULT_TRY, "I was commanded to");
}

EventDesiredResult<IWitch> WitchWander::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    ITerrorPlayer *pAttacker = GetVirtualClass<ITerrorPlayer>(info.GetAttacker());
    if(IsValidEnemy(pAttacker))
    {
        if(g_pWitchEvil->IsWitchEvil(me->entindex()))
        {
            return TrySuspendFor(new WitchEvilAttack(pAttacker), RESULT_IMPORTANT, "Attacking Survivor that injured me");
        }
        return TrySuspendFor(new WitchAttack(pAttacker), RESULT_IMPORTANT, "Attacking Survivor that injured me");
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchWander::OnSound(IWitch *me, CBaseEntity *source, const Vector &pos, KeyValues *keys)
{
    if(source)
    {
        if(!m_Victim_sound.HasStarted())
        {
            ITerrorPlayer *pSource = (ITerrorPlayer*)source;
            if(IsValidEnemy(pSource))
            {
                float flRange = me->MyNextBotPointer()->GetRangeSquaredTo(source); // Left 4 dead 2 INextBot poiter offset 1679(6716)
                float flHeatRadius = z_witch_wander_hear_radius.GetFloat();
                if((flHeatRadius * flHeatRadius) >= flRange)
                {
                    if(me->IsLineOfSightClear(pSource->EyePosition()))
                    {
                        if(pSource->GetTimeSinceLastFiredWeapon() < 0.2f)
                        {
                            m_Victim_sound.Start(0.4f);
                            m_Victim = pSource->GetRefEHandle();
                        }
                    }
                }
            }
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchWander::OnContact(IWitch *me, CBaseEntity *other, CGameTrace *result)
{
    if(!other)
    {
        return TryContinue();
    }

    ITerrorPlayer* pOther = (ITerrorPlayer*)other;
    if(g_CallHelper->IsBreakableEntity(other, false, true)
    || pOther->ClassMatches("prop_door*")
    || pOther->ClassMatches("func_door*"))
    {
        BuildWanderPath(me);
        return TryContinue();
    }
    
    if(!pOther->IsPlayer())
    {
        if(pOther->MyCombatCharacterPointer())
        {
            if(!me->IsPlayingGesture(ACT_TERROR_ATTACK))
            {
                me->AddGesture(ACT_TERROR_ATTACK);
            }
        }
    }

    return TryContinue();
}

EventDesiredResult<IWitch> WitchWander::OnMoveToSuccess(IWitch *me, const Path *path)
{
    float flRand = RandomFloat(6.f, 10.f);
    m_pathRebuildTimer.Start(flRand);
    m_path.Invalidate();
    
    me->GetLocomotionInterface()->Stop();
    me->GetBodyInterface()->StartActivity(ACT_TERROR_IDLE_NEUTRAL); // #todo Activity 690 ACT_TERROR_WITCH_WANDER_IDLE

    return TryContinue();
}

EventDesiredResult<IWitch> WitchWander::OnMoveToFailure(IWitch *me, const Path *path, MoveToFailureType reason)
{
    if(!BuildWanderPath(me))
    {
        m_path.Invalidate();
        me->GetLocomotionInterface()->Stop();
        me->GetBodyInterface()->StartActivity(ACT_TERROR_IDLE_NEUTRAL); // #todo Activity 690 ACT_TERROR_WITCH_WANDER_IDLE

        float flRand = RandomFloat(3.f, 6.f);
        m_pathRebuildTimer.Start(flRand);
    }

    return TryContinue();
}

INavArea *WitchWander::FindWanderArea(IWitch *me)
{
    NavAreaCollector collect(true);
    collect.m_area.EnsureCapacity(1000);
    const INavArea* area = (const INavArea*)me->GetLastKnownArea();
    if(!area)
    {
		INavMesh *pMesh = g_HL2->GetTheNavMesh();
		if(!pMesh) return nullptr;

		return pMesh->GetNearestNavArea(me->GetAbsOrigin());
    }

    SearchSurroundingAreas(const_cast<INavArea*>(area), me->GetAbsOrigin(), collect, 960.f);

    int nCount = collect.m_area.Count();
    if(nCount <= 0)
    {
        return nullptr;
    }

    int swapAreas = 0;
    int i = 0;
    while( i < nCount )
    {
        swapAreas = RandomInt(0, nCount - 1);
        V_swap(collect.m_area[swapAreas], collect.m_area[i]);
        ++i;
    }

    i = 0;
    while( i < nCount )
    {
        INavArea *pArea = collect.m_area[i];
        if(pArea)
        {
            Vector vec_nw = pArea->GetCorner(NORTH_WEST);       // 4,  8,  12
            Vector vec_se = pArea->GetCorner(SOUTH_EAST);       // 16, 20, 24

            if(pArea != area && !pArea->IsDamaging() && (vec_se.x - vec_nw.x) >= 36.f && (vec_se.y - vec_nw.y) >= 36.f)
            {
                if(fabs(pArea->GetCenter().z - me->GetAbsOrigin().z) <= 120.f && !pArea->HasSpawnAttributes((1<<3)))
                {
                    return pArea;
                }
            }
        }
        ++i;
    }
    return nullptr;
}

bool WitchWander::BuildWanderPath(IWitch *me)
{
    INavArea *pFindArea = FindWanderArea(me);
    if(pFindArea)
    {
        INextBot *pBot = me->MyNextBotPointer();
        IWitchLocomotion *pLocomotion = (IWitchLocomotion*)pBot->GetLocomotionInterface();

        WanderingWitchPathCost cost(pBot, pLocomotion);
        Vector vec_nw = pFindArea->GetCorner(NORTH_WEST);       // 4, 8, 12
        Vector vec_se = pFindArea->GetCorner(SOUTH_EAST);       // 16, 20, 24

        float flRandVecX = RandomFloat((vec_se.x - vec_nw.x) * -0.25f, 0.25f * (vec_se.x - vec_nw.x));
        float flRandVecY = RandomFloat((vec_se.y - vec_nw.y) * -0.25f, 0.25f * (vec_se.y - vec_nw.y));

        Vector vec_goal((flRandVecX + pFindArea->GetCenter().x), (flRandVecY + pFindArea->GetCenter().y), pFindArea->GetCenter().z);

        if(m_path.Compute(pBot, vec_goal, cost))
        {
            m_WarningBuildCount = 0;
            return true;
        }
    }

    m_path.Invalidate();
    ++m_WarningBuildCount;
    if(m_WarningBuildCount > 10)
    {
        DevMsg("Witch %d failing excessively!\n", me->entindex());
    }
    return false;
}

