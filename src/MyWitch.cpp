#include "MyWitch.h"
#include "ConVar_l4d.h"

/**
 * WitchAttack                  +
 * WitchDying					+
 * WitchKillIncapVictim         +
 * WitchStandingAction          +
 * WitchBlinded                 +
 * WitchShoved                  +
 * WitchRetreat                 +
 * WitchAngry                   +
 * WitchIdle                    +
 * WitchBurn                    +
 * WitchBehavior                +
 * WitchExecAction              +
 * WitchIntention               +
 */

const float intensityMultipliers[] = { 0.05f, 0.2f, 0.5f, 1.0f };

CWitchEvil *g_pWitchEvil = nullptr;

ConVar z_witch_wander_personal_space("z_witch_wander_personal_space", "240", FCVAR_CHEAT);

bool IsValidEnemy(IBaseEntity *pPlayer)
{
    return (pPlayer && pPlayer->IsPlayer() && pPlayer->IsAlive() && pPlayer->GetTeamNumber() == 2);
}

bool IsValidEnemy(CBaseEntity *pPlayer)
{
    IBaseEntity* pEnt = GetVirtualClass<IBaseEntity>(pPlayer);
    return (pEnt && pEnt->IsPlayer() && pEnt->IsAlive() && pEnt->GetTeamNumber() == 2);
}

ITerrorPlayer *GetClosestSurvivor(const Vector& vecPos, bool bIncapaci, bool bLastArea)
{
    ClosestSurvivorScan scan(vecPos, 1e+08, nullptr, bIncapaci, bLastArea);
    ForEachSurvivor(scan);

    ITerrorPlayer *pPlayer = scan.GetBestPlayer();
    if(pPlayer && pPlayer->IsPlayer())
        return pPlayer;

    return nullptr;
}

Action<IWitch> *WitchMainAction::InitialContainedAction(IWitch *me)
{
    return new WitchBehavior;
}

EventDesiredResult<IWitch> WitchMainAction::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    if((info.GetDamageType() & /*0x20000187*/ (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_CLUB | DMG_SHOCK | DMG_BUCKSHOT)) != 0)
    {
        me->AddGesture( ACT_TERROR_FLINCH, true);
    }
    return TryContinue();
}

ActionResult<IWitch> WitchMainAction::Update(IWitch *me, float interval)
{
    if(me && me->IsOnFire() && me->IsAlive())
    {
        if(me->m_burningTimer.GetElapsedTime() > z_witch_burn_time.GetFloat())
        {
            me->TakeDamage(CTakeDamageInfoHack(me, me, 9999.9f, DMG_BURN));
        }
    }

    return Continue();
}

WitchIntention::WitchIntention(INextBot *me) : IIntention(me)
{
    m_me = nullptr;
    m_behavior = nullptr;
}

WitchIntention::~WitchIntention()
{
    if(m_behavior)
        delete m_behavior;
}

bool WitchIntention::IsAbleToBlokMovementOf(const INextBot *)
{
    return true;
}

WitchMainAction *WitchIntention::InitialAction(void)
{
    return new WitchMainAction;
}

void WitchIntention::Reset()
{
    m_me = (IWitch*)GetBot()->GetEntity();
    if(m_behavior)
        delete m_behavior;

    m_behavior = new Behavior<IWitch>(InitialAction());
}

void WitchIntention::Update(void)
{
    if(m_behavior)
    {
        m_behavior->Update(static_cast<IWitch*>(m_me), GetUpdateInterval());
    }
}

class WitchCreateComponentsClass
{
    public: void WitchCreateComponents();
    static MemberClassFunctionWrapper<void, WitchCreateComponentsClass> WitchCreateComponents_Actual;
};

MemberClassFunctionWrapper<void, WitchCreateComponentsClass> WitchCreateComponentsClass::WitchCreateComponents_Actual;

void WitchCreateComponentsClass::WitchCreateComponents()
{
    WitchCreateComponents_Actual(this);

    if(ukr_my_witch_action.GetBool())
    {
        WitchIntention* &pIntention = access_member<WitchIntention*>(this, (869 * 4));      // получаєм WitchIntention offset 3476
        IWitch *pBot = reinterpret_cast<IWitch*>(this);                                     // приводим указатіль this до класу IWitch
        pBot->MyNextBotPointer()->RemoveComponent(pIntention);                              // вилучаєм наш WitchIntention з листа компонентів і видаляєм його
        pIntention = new WitchIntention(pBot->MyNextBotPointer());                          // создаєм наш WitchIntention і присвоюєм його нашій вічці
    }
}

class CWitchHookCallBack : public CAppSystem
{
private:
    CDetour *pDetourWitchCreateComponents;

public:
    CWitchHookCallBack() : CAppSystem(), pDetourWitchCreateComponents(nullptr)
    {}

	virtual void OnAllLoaded() override {}
	virtual void OnUnload() override
    {
        if(pDetourWitchCreateComponents)
        {
            pDetourWitchCreateComponents->Destroy();
            pDetourWitchCreateComponents = nullptr;
        }
        delete g_pWitchEvil;
    }
	virtual void OnLoad() override
    {
        pDetourWitchCreateComponents = CDetourManager::CreateDetour(
            (void *)GetCodeAddr(reinterpret_cast<VoidFunc>(&WitchCreateComponentsClass::WitchCreateComponents)), 
            WitchCreateComponentsClass::WitchCreateComponents_Actual, "Witch_CreateComponents");
        
        if(pDetourWitchCreateComponents)
        {
            pDetourWitchCreateComponents->EnableDetour();
        }

        g_pWitchEvil = new CWitchEvil();
    }
};

CWitchHookCallBack *g_pWitchHook = new CWitchHookCallBack();

float WitchPathCost::operator()(INavArea *area, INavArea *fromArea, const INavLadder *ladder, const CFuncElevator *elevator, float length) const
{
    // Базова перевірка - якщо немає початкової області
    if (!fromArea) {
        return BASE_COST;
    }

    // Перевірка валідності компонентів
    if (!IsValidForPathfinding(area)) {
        return INVALID_PATH_COST;
    }

    // Розрахунок вартості залежно від типу руху
    float movementCost = ladder ? 
        CalculateLadderCost(area, fromArea, ladder) :
        CalculateGroundCost(area, fromArea, length);

    if (movementCost < 0.0f) {
        return INVALID_PATH_COST;
    }

    // Фінальна вартість з урахуванням додаткових факторів
    return CalculateFinalCost(area, fromArea, movementCost);
}

WitchPathCost::WitchPathCost(INextBot *bot)
    : pBot(bot), flRandomFactor(1.f)
{
    if (pBot) {
        pLocomotion = pBot->GetLocomotionInterface();
        auto pInfected = reinterpret_cast<IBaseCombatCharacter*>(pBot->GetEntity())->MyInfectedPointer();
        if(pInfected)
            flRandomFactor = access_member<float>(pInfected, 3548);
    } else {
        pLocomotion = nullptr;
    }
}

float WitchPathCost::CalculateGroundCost(INavArea *area, INavArea *fromArea, float providedLength) const
{
    HeightInfo heightInfo = CalculateHeightDifference(area, fromArea);
    
    // Перевіряємо можливість подолання висоти
    if (!CanTraverseHeight(heightInfo.difference)) {
        return INVALID_PATH_COST;
    }

    // Розраховуємо базову дистанцію
    float distance = (providedLength > 0.0f) ? 
        providedLength : 
        CalculateHorizontalDistance(area, fromArea);

    // Застосовуємо штраф за висоту
    return ApplyHeightPenalty(distance, heightInfo.difference);
}

float WitchPathCost::CalculateLadderCost(INavArea *area, INavArea *fromArea, const INavLadder *ladder) const
{
    float difficultyFactor = 1.0f - flRandomFactor;
    float baseCost = difficultyFactor * LADDER_BASE_FACTOR + LADDER_BASE_FACTOR;

    // Збільшуємо вартість для руху вгору по драбині
    if (IsMovingUpward(fromArea, area)) {
        baseCost *= LADDER_UPWARD_MULTIPLIER;
    }

    return baseCost * ladder->m_length;
}

bool ClosestSurvivorScan::operator()(int index)
{
    auto pPlayer = GetVirtualClass<ITerrorPlayer>(index);
    if(pPlayer != pIgnorePlayer)
    {
        CNavArea *pLastArea = nullptr;

        if(pPlayer->IsPlayer() 
        && pPlayer->IsConnected() 
        && pPlayer->IsAlive() 
        && pPlayer->GetTeamNumber() == 2 
        && ( bIncapaci || !pPlayer->IsIncapacitated() || (survivor_skills.GetInt() && access_member<signed int>(pPlayer, 11072) == 4))
        && (!bLastArea || (pLastArea = pPlayer->GetLastKnownArea()) == nullptr || access_member<int>(pLastArea, 304) >= 0))
        {
            float newRange = pPlayer->WorldSpaceCenter().DistToSqr(vecPos);
            if(flMaxRange > newRange)
            {
                flMaxRange = newRange;
                pBestEntity = pPlayer;
            }
        }
    }
    return true;
}

bool WitchSafeScan::operator()(INavArea *area)
{
    Vector vec_center = (area->GetCenter() + Vector(0.f, 0.f, 40.f));
    if(g_CallHelper->Director_IsVisibleToTeam(vec_center, 2))
    {
        return true;
    }

    for(int i = 0; i < 4; i++)
    {
        Vector vec_corner = (area->GetCorner(static_cast<NavCornerType>(i)) + Vector(0.f, 0.f, 40.f));
        if(g_CallHelper->Director_IsVisibleToTeam(vec_corner, 2))
        {
            return true;
        }
    }

    INextBot *pBot = m_me->MyNextBotPointer();
    Vector vec_pos = pBot->GetPosition();
    float flDot = vec_pos.Dot(area->GetCenter());
    float min_range = z_witch_min_retreat_range.GetFloat();
    
    if((min_range + min_range) > flDot)
    {
        return true;
    }

    WitchPathCost cost(pBot);
    float resultCost = cost(area, area->GetParent(), nullptr, nullptr, 1.0);

    if(flDot < resultCost)
    {
        return true;
    }

    m_pBestArea = area;

    return true;
}

Intensity::Intensity()
{
    flPrimaryIntensity = 0;
    flAveragedIntensity = 0;
    m_updateInterval.Start();
}

Intensity::~Intensity()
{
}

void Intensity::Reset()
{
    flPrimaryIntensity = 0;
    flAveragedIntensity = 0;
    m_updateInterval.Start();
}

void Intensity::Update()
{
    static constexpr double EPSILON = 1.0e-6;
    float flNewIntensity = 0.f;
    float flElaps = m_updateInterval.GetElapsedTime();
    m_updateInterval.Start();

    if(m_decayTimer.IsElapsed())
    {
        flNewIntensity = flPrimaryIntensity - flElaps / g_pConVar->GetConVarFloat("intensity_decay_time");
        if(flNewIntensity < 0.f) {
            flPrimaryIntensity = 0.f;
            flNewIntensity = 0.f;
        } else {
            flPrimaryIntensity = flNewIntensity;
        }
    }
    else
    {
        flNewIntensity = flPrimaryIntensity;
    }

    float flDeltaIntensity = flNewIntensity - flAveragedIntensity;
    if(flDeltaIntensity >= EPSILON)
    {
        float flDecayFactory = logf(flDeltaIntensity) / g_pConVar->GetConVarFloat("intensity_averaged_following_decay") * flElaps;
        flNewIntensity = expf(flDecayFactory) + flAveragedIntensity;
    }

    flAveragedIntensity = flNewIntensity;
    
    float flLock = g_pConVar->GetConVarFloat("intensity_lock");
    if(flLock >= 0.f)
    {
        flPrimaryIntensity = flLock;
        flAveragedIntensity = flLock;
    }
}

void Intensity::Increase(IntensityType nType)
{
    float flFactory = g_pConVar->GetConVarFloat("intensity_factor");

    float flMultiplir = 0.f;
    if(nType >= 1 && nType <= 4)
    {
        flMultiplir = intensityMultipliers[nType - 1];
    }

    float flNewValue = (flMultiplir * flFactory) + flPrimaryIntensity;
    if(flNewValue > 1.f)
    {
        flNewValue = 1.f;
    }
    flPrimaryIntensity = flNewValue;

    if(m_decayTimer.GetRemainingTime() < 5.f)
    {
        m_decayTimer.Start(5.f);
    }

    float flLock = g_pConVar->GetConVarFloat("intensity_lock");
    if(flLock >= 0.f)
    {
        flPrimaryIntensity = flLock;
        flAveragedIntensity = flLock;
    }
}

void Intensity::InhibitDecay(float duration)
{
    m_decayTimer.Start(duration);
}

CWitchEvil::CWitchEvil() : 
    func_IsWitchEvil(luabridge::getGlobal(g_Sample.GetLuaState(), "IsWitchEvil")),
    func_GetParticleData(luabridge::getGlobal(g_Sample.GetLuaState(), "GetWitchParticle")),
    func_ClearData(luabridge::getGlobal(g_Sample.GetLuaState(), "Clear"))
{
}

CWitchEvil::~CWitchEvil() {}

bool CWitchEvil::IsWitchEvil(int id)
{
    if(func_IsWitchEvil.isFunction())
    {
        auto result = func_IsWitchEvil(id);
        return (result.cast<bool>() ? true : false);
    }

    return false;
}

void CWitchEvil::GetParticleData(int id, IParticleSystem **pParticleLeye, IParticleSystem **pParticleReye)
{
    if(!pParticleLeye && !pParticleReye)
        return;

    if(func_GetParticleData.isFunction())
    {
        auto table = func_GetParticleData(id);
        if(table.isTable())
        {
            unsigned int id_leye = static_cast<unsigned int>(table["particle_leye_id"].cast<int>());
            if(id_leye > 0)
            {
                *pParticleLeye = (IParticleSystem*)gamehelpers->ReferenceToEntity(id_leye);
            }

            unsigned int id_reye = static_cast<unsigned int>(table["particle_reye_id"].cast<int>());
            if(id_reye > 0)
            {
                *pParticleReye = (IParticleSystem*)gamehelpers->ReferenceToEntity(id_reye);
            }
        }
    }
}

void CWitchEvil::ClearData(int id)
{
    if(func_ClearData.isFunction())
    {
        func_ClearData(id);
    }
}

CSearchBestPlayer::CSearchBestPlayer(const Vector &vec) : m_pPlayer(nullptr), flMaxRange(1e+08)
{
    m_vec.Init(vec.x, vec.y, vec.z);
}

bool CSearchBestPlayer::operator()(int i)
{
    IBaseEntity *pEnt = GetVirtualClass<IBaseEntity>(i);
    if(!pEnt)
        return true;

    ITerrorPlayer* pPlayer = nullptr;
    if((pPlayer = access_dynamic_cast<ITerrorPlayer>(pEnt, "CTerrorPlayer")) == nullptr)
        return true;

    if( !pPlayer->IsConnected() || 
        !pPlayer->IsPlayer() || 
        !pPlayer->IsAlive() || 
        pPlayer->GetTeamNumber() != 2 || 
        pPlayer->IsIncapacitated())
    {
        return true;
    }

    float newRange = m_vec.DistToSqr(pPlayer->WorldSpaceCenter());
    if(flMaxRange > newRange)
    {
        flMaxRange = newRange;
        m_pPlayer = pPlayer;
    }
    return true;
}
