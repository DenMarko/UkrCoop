#include "MyTank.h"
#include "detours.h"
#include "Interface/IInfected.h"

ConVar tank_block_ladder("ukr_tank_block_ladder", "30", FCVAR_CHEAT, "distance to stargered blockera", true, 0.f, true, 200.f);

//====================== TankIntention and TankBehavior ======================//
TankIntention::TankIntention(INextBot *me) : IIntention(me)
{
    m_me = nullptr;
    m_behavior = nullptr;
}

TankIntention::~TankIntention()
{
    if(m_behavior)
        delete m_behavior;
}

void TankIntention::Reset()
{
    m_me = (ITank*)GetBot()->GetEntity();
    if(m_behavior)
        delete m_behavior;

    m_behavior = new Behavior<ITank>(new TankBehavior());
}

void TankIntention::Update(void)
{
    if(m_behavior)
    {
        m_behavior->Update(m_me, GetUpdateInterval());
    }
}
//==================== End TankIntention and TankBehavior ====================//

//====================== TankTankClass Detour ======================//
class TankTankClass
{
public:
    void TankTank();
    static MemberClassFunctionWrapper<void, TankTankClass> TankTank_Actual;
};
MemberClassFunctionWrapper<void, TankTankClass> TankTankClass::TankTank_Actual;

void TankTankClass::TankTank()
{
    TankTank_Actual(this);
    if(ukr_my_tank_action.GetBool())
    {
        TankIntention* &pIntention = access_member<TankIntention*>(this, (2985 * 4));   // получаєм TankIntention offset 11940
        ITank *pBot = reinterpret_cast<ITank*>(this);                                   // приводим указатіль this до класу ITank
        pBot->MyNextBotPointer()->RemoveComponent(pIntention);                          // вилучаєм наш TankIntention з листа компонентів і видаляєм його
        pIntention = new TankIntention(pBot->MyNextBotPointer());                       // создаєм наш TankIntention
    }
}

class CTankHookCallBack : public CAppSystem
{
private:
    CDetour *pDetourTankTank;

public:
    CTankHookCallBack() : CAppSystem(), pDetourTankTank(nullptr) 
    {}

    virtual void OnAllLoaded() override {}
    virtual void OnUnload() override
    {
        if(pDetourTankTank)
        {
            pDetourTankTank->Destroy();
            pDetourTankTank = nullptr;
        }
    }
    virtual void OnLoad() override
    {
        pDetourTankTank = CDetourManager::CreateDetour(
            (void *)GetCodeAddr(reinterpret_cast<VoidFunc>(&TankTankClass::TankTank)), 
            TankTankClass::TankTank_Actual, "Tank_Tank");

        if(pDetourTankTank)
        {
            pDetourTankTank->EnableDetour();
        }
    }
};

CTankHookCallBack *g_TankHookCallBack = new CTankHookCallBack();
//==================== End TankTankClass Detour ====================//

//====================== InfectedPathCost ======================//
InfectedPathCost::InfectedPathCost(INextBot *bot)
    : m_pBot(bot), m_pEnemy(nullptr), m_pLocomotion(nullptr), m_flRandomFactor(1.f)
{
    if(m_pBot)
    {
        m_pLocomotion = m_pBot->GetLocomotionInterface();
        m_pEnemy = reinterpret_cast<IBaseCombatCharacter*>(m_pBot->GetEntity());
        if(m_pEnemy)
        {
            auto pInfected = m_pEnemy->MyInfectedPointer();
            if(pInfected)
                m_flRandomFactor = pInfected->GetRandomFactor();
        }
    }
}

InfectedPathCost::InfectedPathCost(ITank *bot)
    : m_pBot(nullptr), m_pEnemy(nullptr), m_pLocomotion(nullptr), m_flRandomFactor(1.f)
{
    m_pBot = bot->MyNextBotPointer();
    if(m_pBot)
    {
        m_pLocomotion = m_pBot->GetLocomotionInterface();
        m_pEnemy = reinterpret_cast<IBaseCombatCharacter*>(m_pBot->GetEntity());
        if(m_pEnemy)
        {
            auto pInfected = m_pEnemy->MyInfectedPointer();
            if(pInfected)
                m_flRandomFactor = pInfected->GetRandomFactor();
        }
    }
}

// Основной метод расчета стоимости пути для зараженного персонажа (Infected) в игре.
float InfectedPathCost::operator()(INavArea *area, INavArea *fromArea, const INavLadder *ladder, const CFuncElevator *elevator, float length) const
{
    if(!fromArea)
    {
        // Якщо немає попередньої області, повертаємо вартість 0.
        return 0.f;
    }
    else
    {
        // Якщо немає бота, повертаємо вартість -1 (неможливий шлях).
        if(!m_pBot)
            return -1.f;

        // Якщо локомоція бота не визначена або область недоступна для руху, повертаємо вартість -1.
        if(!m_pLocomotion || !m_pLocomotion->IsAreaTraversable((CNavArea *)area))
            return -1.f;

        // Отримуємо кути області для подальших розрахунків.
        Vector vecEast = area->GetCorner(SOUTH_EAST);      // 16 20
        Vector vecWest = area->GetCorner(SOUTH_WEST);      // 4 8

        // Обчислюємо фактор випадковості для вартості шляху.
        float factor = (1.f - m_flRandomFactor);

        // Розрахунок вартості шляху в залежності від того, чи використовується сходи.
        if(!ladder)
        {
            Vector targetPoint;
            Vector closestPoint;

            // Перевіряємо різницю висот між областями.
            area->GetClosestPointOnArea(fromArea->GetCenter(), &closestPoint);
            fromArea->GetClosestPointOnArea(area->GetCenter(), &targetPoint);
            float distance = (closestPoint.z - targetPoint.z);

            // Якщо висота перевищує максимальну висоту стрибка, шлях неможливий.
            if(distance > m_pLocomotion->GetMaxJumpHeight())
            {
                return -1.f;
            }

            // Якщо довжина шляху не визначена, обчислюємо її як відстань між центрами областей.
            if(length <= 0.f)
            {
                length = (area->GetCenter() - fromArea->GetCenter()).Length();
            }

            float heightPenalty = 0.f;
            if(m_pEnemy)
            {
                if(m_pEnemy->GetTeamNumber() == 3)
                {
                    if(m_pEnemy->GetClass() == ZombieClassSmoker)
                    {
                        if(distance > 0.f)
                        {
                            float scaleFactor = factor * 3.0 + 1.0;
                            if (distance - 24.0 >= 0.0)
                            {
                                heightPenalty = distance * scaleFactor * (distance * scaleFactor);  // квадратичний штраф
                            }
                            else
                            {
                                heightPenalty = distance * distance;                                // квадратичний штраф
                            }
                        }
                    }
                    else if(distance > 0.f)
                    {
                        heightPenalty = distance * distance;
                    }
                }
                else if(distance > 0.f)
                {
                    heightPenalty = distance * distance;
                }
            }

            // Додаємо штраф за горизонтальну відстань, якщо вона перевищує певний поріг.
            float horizontalDist = heightPenalty + closestPoint.DistToSqr(targetPoint);
            if(horizontalDist > 9.f)
            {
                length += horizontalDist * (1.0f / (vec_t)FastSqrt(horizontalDist));
            }

            IBaseCombatCharacter* pMainEnt = reinterpret_cast<IBaseCombatCharacter*>(m_pBot->GetEntity());
            if(pMainEnt->GetTeamNumber() == 3)
            {
                ZombieClassType zType = pMainEnt->GetClass();
                switch (zType)
                {
                case ZombieClassSmoker:
                case ZombieClassBommer:
                    {
                        if(access_member<int>(area, 380*4))
                        {
                            length *= 10.f;
                        }
                        else if(access_member<int>(area, 376*4))
                        {
                            length *= 5.f;
                        }
                    }
                    break;
                case ZombieClassHunter:
                case ZombieClassTank:
                    {
                        if(access_member<int>(area, 380*4))
                        {
                            length *= 3.f;
                        }
                        else if(access_member<int>(area, 376*4))
                        {
                            length *= 2.f;
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            float cost = factor * 5.f + 5.f;
            if(fromArea->GetCenter().z > area->GetCenter().z)
            {
                cost *= 3.f;
            }
            else
            {
                if(m_pEnemy && m_pEnemy->GetTeamNumber() == 3)
                {
                    if(m_pEnemy->GetClass() == ZombieClassSmoker)
                    {
                        cost = 1.0f;
                    }
                }
            }

            length = cost * ladder->m_length;
        }

        float finalCost = length + fromArea->GetCostSoFar();
        if(area->GetPlayerCount(1))
        {
            float diff = (vecEast.x - vecWest.x + vecEast.y - vecWest.y) * 0.5f;
            if(diff >= 1.f)
            {
                finalCost += 50000.0f * area->GetPlayerCount(1) / diff;
            }
        }

        return finalCost;
    }
}
//==================== End InfectedPathCost ====================//

//====================== LadderBlockFix ======================//
LadderBlockFix::LadderBlockFix() : m_pNextBot(nullptr), m_me(nullptr) { }

void LadderBlockFix::Init(ITank *me)    
{
    m_me = me;                                  // set tank pointer
    if(m_me) {                                  // if valid tank
        m_pNextBot = m_me->MyNextBotPointer();  // get NextBot pointer
    }
    m_timer.Invalidate();                       // invalidate timer
}

void LadderBlockFix::UpdateTimer()
{
    if(!m_timer.HasStarted())                   // if timer not started
    {
        m_timer.Start();                        // start timer
    }
    else
    {
        if(m_timer.IsGreaterThen(1.5))          // if 1.5 seconds passed
        {
            if(m_PlayerCollect.Count() > 0)     // and we have processed players
                m_PlayerCollect.RemoveAll();    // clear processed players list

            m_timer.Invalidate();               // invalidate timer to start it again
        }
    }
}

bool LadderBlockFix::operator()(ITerrorPlayer *pPlayer)
{
    if(!m_pNextBot || !m_me) return false;         // safety check

    if(pPlayer->GetTeamNumber() != 2)   // only humans
        return true;

    if(!pPlayer->IsAlive())             // only alive
        return true;

    if(pPlayer->IsIncapacitated())      // no incapacitated
        return true;

    if(m_pNextBot->GetRangeTo((CBaseEntity*)pPlayer) <= tank_block_ladder.GetFloat())   // distance check
    {
        if(!pPlayer->IsStaggering())    // only staggering
        {
            if(!m_PlayerCollect.HasElement(pPlayer->GetRefEHandle()))   // not processed yet
            {
                pPlayer->OnStaggered(m_me); // stagger player
                m_PlayerCollect.AddToTail(pPlayer->GetRefEHandle());    // add to processed list
            }
        }
    }

    return true;
}
//==================== End LadderBlockFix ====================//