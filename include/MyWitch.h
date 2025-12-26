#ifndef _HEADER_MY_WITCH_INCLUDE_
#define _HEADER_MY_WITCH_INCLUDE_
#include "extension.h"

#include "CActionHook.h"
#include "Interface/IParticleSystem.h"
#include "Interface/INextBotChasePath.h"
#include "Interface/INavLadder.h"
#include "Interface/IMusic.h"
#include "Interface/IWitch.h"

#include "CTempEntity.h"
#include "detours.h"

#include "LuaBridge/LuaBridge.h"

// Клас для керування злими ефектами Witch
class CWitchEvil
{
public:
    CWitchEvil();
    ~CWitchEvil();

public:
    bool IsWitchEvil(int id);
    void GetParticleData(int id, IParticleSystem** pParticleLeye, IParticleSystem** pParticleReye);
    void ClearData(int id);
private:
    luabridge::LuaRef func_IsWitchEvil;
    luabridge::LuaRef func_GetParticleData;
    luabridge::LuaRef func_ClearData;
};

extern ConVar z_witch_wander_personal_space;
extern CWitchEvil *g_pWitchEvil;

const char *SurvivorCharacterName(int8_t character);
ITerrorPlayer *GetClosestSurvivor(const Vector& pos, bool Incapaci = false, bool LastArea = false);
bool IsValidEnemy(IBaseEntity* pPlayer);
bool IsValidEnemy(CBaseEntity* pPlayer);

inline ITerrorPlayer* ToTerrorPlayer(IBaseEntity* pEntity)
{
    if(!pEntity || !pEntity->IsPlayer())
        return nullptr;

    return static_cast<ITerrorPlayer*>(pEntity);
}

// Таймери інтервалів
class Intensity
{
public:
    enum IntensityType {
        None = 0,
        Low,
        Medium,
        High,
        Max
    };

    Intensity();
    ~Intensity();

    void Reset();
    void Update();
    void Increase(IntensityType nType);
    void InhibitDecay(float duration);

private:
    float flPrimaryIntensity;
    float flAveragedIntensity;
    IntervalTimers m_updateInterval;
    CountdownTimers m_decayTimer;
};

// Спеціалізований клас вартості шляху для Witch
class WitchPathCost : public IPathCost
{
private:
    INextBot* pBot;
    ILocomotion* pLocomotion;
    float flRandomFactor;

    // Константи для налаштування
    static constexpr float INVALID_PATH_COST = -1.0f;
    static constexpr float BASE_COST = 0.0f;
    static constexpr float HEIGHT_THRESHOLD = 24.0f;
    static constexpr float MINIMUM_WEIGHT = 3.0f;
    static constexpr float DOWNWARD_PENALTY_MULTIPLIER = 5.0f;
    static constexpr float HIGH_HEIGHT_PENALTY = 4.0f;
    static constexpr float LOW_HEIGHT_PENALTY = 1.0f;
    static constexpr float LADDER_BASE_FACTOR = 5.0f;
    static constexpr float LADDER_UPWARD_MULTIPLIER = 3.0f;
    static constexpr float PLAYER_AVOIDANCE_PENALTY = 50000.0f;
    static constexpr int SURVIVOR_TEAM = 1;

public:
    WitchPathCost(INextBot* bot);

    virtual float operator()(INavArea* area, INavArea* fromArea, const INavLadder* ladder, const CFuncElevator* elevator, float length) const;

private:
    // Перевірка валідності для pathfinding
    bool IsValidForPathfinding(INavArea* area) const
    {
        if (!pBot) {
            return false;
        }

        if (!pLocomotion || !pLocomotion->IsAreaTraversable((CNavArea*)area)) {
            return false;
        }

        return true;
    }

    // Розрахунок вартості руху по землі
    float CalculateGroundCost(INavArea* area, INavArea* fromArea, float providedLength) const;

    // Розрахунок вартості руху по драбині
    float CalculateLadderCost(INavArea* area, INavArea* fromArea, const INavLadder* ladder) const;

    // Структура для інформації про висоту
    struct HeightInfo {
        float difference;
        Vector fromPoint;
        Vector toPoint;
    };

    // Розрахунок різниці висот між областями
    HeightInfo CalculateHeightDifference(INavArea* area, INavArea* fromArea) const
    {
        HeightInfo info;
        
        area->GetClosestPointOnArea(fromArea->GetCenter(), &info.toPoint);
        fromArea->GetClosestPointOnArea(area->GetCenter(), &info.fromPoint);
        info.difference = info.toPoint.z - info.fromPoint.z;
        
        return info;
    }

    // Перевірка можливості подолання висоти
    bool CanTraverseHeight(float heightDifference) const
    {
        return heightDifference <= pLocomotion->GetMaxJumpHeight();
    }

    // Розрахунок горизонтальної дистанції
    float CalculateHorizontalDistance(INavArea* area, INavArea* fromArea) const
    {
        return area->GetCenter().DistTo(fromArea->GetCenter());
    }

    // Застосування штрафу за висоту
    float ApplyHeightPenalty(float baseDistance, float heightDifference) const
    {
        // Рух вниз отримує додатковий штраф (Witch обережна)
        float adjustedHeight = heightDifference;
        if (adjustedHeight < 0.0f) {
            adjustedHeight *= DOWNWARD_PENALTY_MULTIPLIER;
        }

        // Визначаємо штраф залежно від висоти
        float heightPenalty = (heightDifference <= HEIGHT_THRESHOLD) ? 
            LOW_HEIGHT_PENALTY : 
            HIGH_HEIGHT_PENALTY;

        float weight = fabs(adjustedHeight * heightPenalty);
        
        // Додаємо штраф тільки якщо він значний
        if (weight > MINIMUM_WEIGHT) {
            baseDistance += weight;
        }

        return baseDistance;
    }

    // Перевірка руху вгору
    bool IsMovingUpward(INavArea* fromArea, INavArea* area) const
    {
        return fromArea->GetCenter().z > area->GetCenter().z;
    }

    // Розрахунок фінальної вартості з додатковими факторами
    float CalculateFinalCost(INavArea* area, INavArea* fromArea, float baseCost) const
    {
        float finalCost = baseCost + fromArea->GetCostSoFar();
        
        // Додаємо штраф за присутність гравців
        finalCost += CalculatePlayerAvoidancePenalty(area);
        
        return finalCost;
    }

    // Розрахунок штрафу за уникнення гравців
    float CalculatePlayerAvoidancePenalty(INavArea* area) const
    {
        int playerCount = area->GetPlayerCount(SURVIVOR_TEAM);
        if (playerCount == 0) {
            return 0.0f;
        }

        // Розраховуємо розмір області для штрафу
        float areaSize = CalculateAreaSize(area);
        
        if (areaSize >= 1.0f) {
            return (PLAYER_AVOIDANCE_PENALTY * playerCount) / areaSize;
        }

        return 0.0f;
    }

    // Розрахунок розміру області
    float CalculateAreaSize(INavArea* area) const
    {
        Vector southEast = area->GetCorner(SOUTH_EAST);
        Vector southWest = area->GetCorner(SOUTH_WEST);
        
        // Середній розмір по X та Y осях
        return (southEast.x - southWest.x + southEast.y - southWest.y) * 0.5f;
    }
};

// Спеціалізований клас вартості шляху для ходячої Witch
class WanderingWitchPathCost : public IPathCost
{
public:
    explicit WanderingWitchPathCost(INextBot* pBot, IWitchLocomotion* pLocomotion);
    virtual ~WanderingWitchPathCost() = default;

    // Основний метод розрахунку вартості шляху
    virtual float operator()(INavArea* area, INavArea* fromArea, const INavLadder* ladder, 
                           const CFuncElevator* elevator, float length) const override;

private:
    INextBot* m_pBot;
    IWitchLocomotion* m_pLocomotion;

    // Константи для налаштування поведінки ходячої Witch
    static constexpr float INVALID_PATH_COST = -1.0f;
    static constexpr float BASE_COST = 0.0f;
    static constexpr float MAX_TRAVERSABLE_HEIGHT = 16.0f;          // Максимальна висота для ходьби
    static constexpr float HEIGHT_THRESHOLD = 24.0f;                // Поріг для збільшеного штрафу
    static constexpr float DOWNWARD_PENALTY_MULTIPLIER = 5.0f;      // Штраф за рух вниз
    static constexpr float HIGH_HEIGHT_PENALTY = 4.0f;              // Штраф за великі висоти
    static constexpr float LOW_HEIGHT_PENALTY = 1.0f;               // Штраф за малі висоти
    static constexpr float MINIMUM_COST_THRESHOLD = 3.0f;           // Мінімальний поріг для додавання штрафу
    static constexpr int   SPAWN_ATTRIBUTE_FORBIDDEN = (1 << 3);    // Заборонені spawn атрибути
    static constexpr float LADDER_BASE_COST = 5.0f;                 // Базова вартість драбини
    static constexpr float LADDER_UPWARD_MULTIPLIER = 2.5f;         // Штраф за підйом по драбині
    static constexpr float LADDER_DOWNWARD_MULTIPLIER = 1.5f;       // Штраф за спуск по драбині
    static constexpr float MAX_LADDER_LENGTH = 128.0f;              // Максимальна довжина драбини

    // Методи для перевірки придатності області
    bool IsAreaSuitableForWandering(INavArea* area) const;
    bool HasForbiddenAttributes(INavArea* area) const;
    bool IsAreaTraversable(INavArea* area) const;

    // Методи розрахунку вартості
    float CalculateTraversalCost(INavArea* area, INavArea* fromArea, float providedLength) const;
    float CalculateLadderCost(INavArea* area, INavArea* fromArea, const INavLadder* ladder) const;
    float GetHeightDifference(INavArea* area, INavArea* fromArea) const;
    bool CanTraverseHeight(float heightDifference) const;
    bool CanUseLadder(const INavLadder* ladder) const;
    float CalculateDistance(INavArea* area, INavArea* fromArea, float providedLength) const;

    // Методи обробки штрафів за висоту
    float ApplyHeightPenalty(float baseDistance, float heightDifference) const;
    bool IsMovingDownward(float heightDifference) const;
    float ApplyDownwardPenalty(float heightDifference) const;
    float CalculateHeightCost(float adjustedHeight) const;
    bool IsSignificantCost(float cost) const;
};

// Пошук найкращого вижившого гравця
class CSearchBestPlayer
{
public:
    CSearchBestPlayer(const Vector& vec);

    virtual bool operator() (int i);

    inline ITerrorPlayer* GetPlayer()
    {
        return m_pPlayer;
    }
private:
    ITerrorPlayer* m_pPlayer;
    Vector m_vec;
    float flMaxRange;
};

// Пошук безпечної області для Witch
class WitchSafeScan
{
public:
    WitchSafeScan(IWitch* me) : m_me(me), z_witch_min_retreat_range("z_witch_min_retreat_range")
    {
        m_pBestArea = nullptr;
    }

    virtual bool operator()(INavArea* area);

    inline INavArea *GetBestArea() const { return m_pBestArea; }
private:
    IWitch      *m_me;
    INavArea    *m_pBestArea;
    ConVarRef   z_witch_min_retreat_range;
};

// Ітерування по виживших
template<typename T>
bool ForEachSurvivor(T &func)
{
    for(int i = 1; i <= g_pGlobals->maxClients; ++i)
    {
        ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(i);
        if(pPlayer)
        {
            if(pPlayer->IsPlayer())
            {
                if(pPlayer->IsConnected() && pPlayer->GetTeamNumber() == 2)
                {
                    if(!func(i))
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

class WitchKilledMusic
{
public:
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
            IMusic &m_music = access_member<IMusic>(pPlayer, 10160);
            m_music.OnWitchKilled();
        }
        return true;
    }
};

class WitchStopMusic
{
public:
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
            IMusic &m_music = access_member<IMusic>(pPlayer, 10160);
            m_music.OnWitchAttackDone();
        }
        return true;
    }
};

class WitchBurningMusic
{
public:
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
            IMusic &m_music = access_member<IMusic>(pPlayer, 10160);
            m_music.OnWitchBurning();
        }
        return true;
    }
};

class WitchAttackMusic
{
public:
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
            IMusic &m_music = access_member<IMusic>(pPlayer, 10160);
            m_music.OnWitchAttacking();
        }
        return true;
    }
};

// Пошук найближчого вижившого
class ClosestSurvivorScan
{
public:
    ClosestSurvivorScan(const Vector &pos, float maxRang = 1e+08, ITerrorPlayer* p_IgnorePlayer = nullptr, bool Incapaci = true, bool LastArea = true) :
        vecPos(pos), 
        pBestEntity(nullptr), 
        flMaxRange(maxRang), 
        pIgnorePlayer(nullptr), 
        bIncapaci(Incapaci), 
        bLastArea(LastArea),
        survivor_skills("survivor_skills")
    {
        if(p_IgnorePlayer)
            pIgnorePlayer = p_IgnorePlayer;
    }

    virtual bool operator()(int index);

    inline ITerrorPlayer *GetBestPlayer()
    {
        return pBestEntity;
    }

private:
    Vector vecPos;
    ITerrorPlayer *pBestEntity;
    float flMaxRange;
    ITerrorPlayer *pIgnorePlayer;
    bool bIncapaci;
    bool bLastArea;

    ConVarRef survivor_skills;
};

// Дія Witch при смерті
class WitchDying :public Action< IWitch >
{
public:
    WitchDying( const CTakeDamageInfo &info);
    virtual ~WitchDying() {}

    virtual const char* GetName( void ) const override
    {
        return "WitchDying";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnAnimationActivityComplete(IWitch* me, int activity) override;
    virtual EventDesiredResult< IWitch > OnAnimationEvent( IWitch *me, animevent_t *event) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    
    virtual bool IsAbleToBlockMovementOf(const INextBot* botInMotion) const override
    {
        return false;
    }

private:
    bool TryToStartDeathThroes(IWitch *me);
    void ComputeShoveForce(IWitch *me, const Vector& ShoveDir );
    bool BecomeRagdoll(IWitch *me);
    bool IsStumbling(IWitch *me);

    CTakeDamageInfoHack m_info;
    float flValue_31;
    int nValue_32;
    int PlayerIndex;
};

class WitchStandingAction : public Action< IWitch >
{
public:
    WitchStandingAction(Action< IWitch >* pKillIncalVictim, Activity activity, const ITerrorPlayer* pVictim = nullptr);
    WitchStandingAction(Action< IWitch >* pKillIncalVictim, Activity activity, const char *szSound = nullptr);
    WitchStandingAction(Action< IWitch >* pKillIncalVictim, Activity activity, float timer);
    virtual ~WitchStandingAction() {}

    virtual const char* GetName(void) const override
    {
        return "WitchStandingAction";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnAnimationActivityComplete(IWitch* me, int activity) override;
    
private:
    CountdownTimers m_timer;
    Action< IWitch > *m_ChangeAction;
    Activity nActivity;
    bool bStartToChange;

    const char *szNameSound;
    CHandle<ITerrorPlayer> hVictim;
};

// Дія Witch при вбивстві безпорадної жертви
class WitchKillIncapVictim : public Action< IWitch >
{
public:
    WitchKillIncapVictim(ITerrorPlayer *pVictim);
    virtual ~WitchKillIncapVictim() {}

    virtual const char* GetName(void) const override
    {
        return "WitchKillIncapVictim";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;

    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnAnimationEvent( IWitch *me, animevent_t *event) override;

private:
    CountdownTimers m_timer1;
    CHandle<ITerrorPlayer> hVictim;

    ConVarRef z_witch_damage_per_kill_hit;
    ConVarRef survivor_incap_health;
    ConVarRef z_difficulty;
};

// Дія Witch при відступі
class WitchRetreat : public Action< IWitch >
{
public:
    WitchRetreat();
    virtual ~WitchRetreat() {}

    virtual const char* GetName(void) const override
    {
        return "WitchRetreat";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnAnimationEvent( IWitch *me, animevent_t *event) override;
    virtual EventDesiredResult< IWitch > OnMoveToSuccess(IWitch* me, const Path* path) override;
    virtual EventDesiredResult< IWitch > OnMoveToFailure(IWitch *me, const Path* path, MoveToFailureType reason) override;

private:
    INavArea *FindSafeArea(IWitch *me);
    bool BuildRetreatPath(IWitch *me);

    IPathFollower m_path;
    CountdownTimers m_timer_4610_18440;
    CountdownTimers m_horrorSoundTimer;
    IntervalTimers m_retreatStartTimer;
    IntervalTimers m_visibilityCheckTimer;
    CountdownTimers m_timer_4620_18480;

    CUtlVector<CHandle<ITerrorPlayer>> m_contactedPlayers;

    ConVarRef m_minRetreatDuration;
    ConVarRef m_exitRange;
    ConVarRef m_exitHiddenDuration;
    ConVarRef m_maxRetreatRange;
};

// Дія Witch у злобному стані
class WitchAngry : public Action< IWitch >
{
public:
    WitchAngry(float rage);
    virtual ~WitchAngry() { }

    virtual const char* GetName(void) const override
    {
        return "WitchAngry";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch > OnSuspend(IWitch* me, Action< IWitch > *interruptingAction) override;
    virtual ActionResult< IWitch > OnResume(IWitch *me, Action< IWitch > *interruptingAction) override;

    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnBlinded(IWitch* me, CBaseEntity* blinder) override;
private:
    void Growl(IWitch* me);

    CountdownTimers m_timer1;
    CountdownTimers m_timer2;
    float flCurentRage;

    ConVarRef z_witch_hostile_at_me_anger;
    ConVarRef z_witch_anger_rate;
    ConVarRef z_witch_threat_normal_range;
    ConVarRef z_witch_threat_hostile_range;
    ConVarRef z_witch_relax_rate;
    ConVarRef z_difficulty;
};

// Дія Witch при осліпленні
class WitchBlinded : public Action< IWitch >
{
public:
    WitchBlinded(Action< IWitch >* pAngry, const char* szNameSound);
    virtual ~WitchBlinded() {}

    virtual const char* GetName() const override
    {
        return "WitchBlinded";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
	virtual EventDesiredResult< IWitch > OnSound(IWitch* me, CBaseEntity* source, const Vector& pos, KeyValues* keys) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnBlinded(IWitch* me, CBaseEntity* blinder) override;

private:
    Action< IWitch > *m_pNextAction;
    const char *m_szNameSound;
    float flValue;
    CountdownTimers m_timer1;
    CountdownTimers m_timer2;
};

// Дія Witch при злобному блуканні
class WitchWanderAngry : public Action< IWitch >
{
public:
    WitchWanderAngry(float fltime);
    virtual ~WitchWanderAngry() {}

    virtual const char* GetName(void) const override
    {
        return "WitchWanderAngry";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch > OnSuspend(IWitch* me, Action< IWitch > *interruptingAction) override;
    virtual ActionResult< IWitch > OnResume(IWitch *me, Action< IWitch > *interruptingAction) override;

    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnBlinded(IWitch* me, CBaseEntity* blinder) override;
private:
    void Growl(IWitch* me);

    CountdownTimers m_timer1;
    CountdownTimers m_timer2;
    float flCurentRage;

    ConVarRef z_witch_anger_rate;
    ConVarRef z_witch_relax_rate;
    ConVarRef z_witch_hostile_at_me_anger;
    ConVarRef z_difficulty;

};

// Дія Witch при блуканні
class WitchWander : public Action<IWitch>
{
public:
    WitchWander();
    virtual ~WitchWander() { }

    virtual ActionResult< IWitch >  OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch >  Update(IWitch* me, float interval) override;
    virtual ActionResult< IWitch >  OnSuspend(IWitch* me, Action< IWitch >* interruptingAction)	override;
    virtual ActionResult< IWitch >  OnResume(IWitch* me, Action< IWitch > *interruptingAction) override;

    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnAnimationEvent( IWitch *me, animevent_t *event) override;
    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
	virtual EventDesiredResult< IWitch > OnSound(IWitch* me, CBaseEntity* source, const Vector& pos, KeyValues* keys) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnMoveToSuccess(IWitch* me, const Path* path) override;
    virtual EventDesiredResult< IWitch > OnMoveToFailure(IWitch *me, const Path* path, MoveToFailureType reason) override;

    virtual const char* GetName(void) const override { return "WitchWander"; }

private:

    INavArea *FindWanderArea(IWitch *me);
    bool BuildWanderPath(IWitch *me);
    void UpdatePath(IWitch *me);

    IPathFollower           m_path;

    Vector                  m_lastPosition;

    CountdownTimers         m_growlTimer;
    CountdownTimers         m_pathRebuildTimer;
    CountdownTimers         m_Victim_sound;
    CHandle<ITerrorPlayer>  m_Victim;

    int                     m_WarningBuildCount;

    ConVarRef z_witch_speed;
};

// Дія Witch у стані спокою
class WitchIdle : public Action<IWitch>
{
public:
    WitchIdle() : 
        z_witch_personal_space("z_witch_personal_space"),
        z_witch_threat_normal_range("z_witch_threat_normal_range"),
        z_witch_threat_hostile_range("z_witch_threat_hostile_range")
    {}
    virtual ~WitchIdle() {}

    virtual const char* GetName() const override
    {
        return "WitchIdle";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch > OnResume(IWitch* me, Action< IWitch > *interruptingAction) override;

    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnBlinded(IWitch* me, CBaseEntity* blinder) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
	virtual EventDesiredResult< IWitch > OnSound(IWitch* me, CBaseEntity* source, const Vector& pos, KeyValues* keys) override;

private:

    CountdownTimers m_soundDespair;
    CountdownTimers m_target_set;
    CHandle<ITerrorPlayer> m_Target;
    ConVarRef z_witch_personal_space;
    ConVarRef z_witch_threat_normal_range;
    ConVarRef z_witch_threat_hostile_range;

};

// Дія Witch при нападі
class WitchAttack : public Action< IWitch >
{
public:
    WitchAttack(ITerrorPlayer* pAttacker, bool bSoundPlay = true);
    virtual ~WitchAttack() {}

    virtual const char *GetName() const override
    {
        return "WitchAttack";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnAnimationEvent( IWitch *me, animevent_t *event) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnMoveToSuccess(IWitch* me, const Path* path) override;
    virtual EventDesiredResult< IWitch > OnMoveToFailure(IWitch *me, const Path* path, MoveToFailureType reason) override;
    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
private:
    virtual ITerrorPlayer *GetVictim();
    virtual void SetVictim(ITerrorPlayer* victim);

    CHandle<ITerrorPlayer> hAttacker;
    int8_t cCharacter;
    InfectedChasePath m_path;

    CountdownTimers m_timer1;
    IntervalTimers m_interval1;
    CUtlVector<CHandle<IBaseEntity>> m_PlayerColect;
    IntervalTimers m_interval2;
    bool m_soundPlay;

    ConVarRef m_ChangeVictim;
    ConVarRef m_AttackRange;
};

// Дія Witch при зломленому нападі
class WitchEvilAttack : public Action< IWitch >
{
public:
    WitchEvilAttack(ITerrorPlayer* pAttacker, bool bSoundPlay = true);
    virtual ~WitchEvilAttack() {}

    virtual const char *GetName() const override
    {
        return "WitchAttack";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual void OnEnd(IWitch* me, Action< IWitch > *nextAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnAnimationEvent( IWitch *me, animevent_t *event) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< IWitch > OnMoveToSuccess(IWitch* me, const Path* path) override;
    virtual EventDesiredResult< IWitch > OnMoveToFailure(IWitch *me, const Path* path, MoveToFailureType reason) override;
    virtual EventDesiredResult< IWitch > OnCommandAttack(IWitch* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
private:
    virtual ITerrorPlayer *GetVictim();
    virtual void SetVictim(ITerrorPlayer* victim);

    CHandle<ITerrorPlayer> hAttacker;
    int8_t cCharacter;
    IChasePath m_path;

    CountdownTimers m_timer1;
    IntervalTimers m_interval1;
    CUtlVector<CHandle<IBaseEntity>> m_PlayerColect;
    IntervalTimers m_interval2;
    bool m_soundPlay;

    ConVarRef m_ChangeVictim;
    ConVarRef m_AttackRange;
};

// Дія Witch при штовханні
class WitchShoved : public Action<IWitch>
{
public:
    WitchShoved(IBaseEntity* hShover);
    WitchShoved(Action<IWitch> *pChangeActio, IBaseEntity* hShover);
    virtual ~WitchShoved() {}

    virtual const char* GetName() const override
    {
        return "WitchShoved";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;

    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnAnimationActivityComplete(IWitch* me, int activity) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;
    
    virtual bool IsAbleToBlockMovementOf(const INextBot* botInMotion) const override;

private:
    bool StartShovedActivity(IWitch *me);

    CHandle<ITerrorPlayer> m_hShower;
    Vector m_vecShow;
    Action<IWitch> *m_pChangeAction;
    ConVarRef z_shove_friend_speed;
};

// Дія Witch при підпалі
class WitchBurn : public Action<IWitch>
{
    float val1;
    CHandle<ITerrorPlayer> hAttacker;
public:
    WitchBurn(IBaseEntity *pAttacker);
    virtual ~WitchBurn() {}

    virtual const char* GetName() const override
    {
        return "WitchBurn";
    }

    virtual ActionResult< IWitch > OnStart(IWitch* me, Action<IWitch> *priorAction) override;
    virtual ActionResult< IWitch > Update(IWitch* me, float interval) override;

    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnBlinded(IWitch* me, CBaseEntity* blinder) override;
    virtual EventDesiredResult< IWitch > OnContact(IWitch* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< IWitch > OnShoved(IWitch* me, CBaseEntity* pusher) override;

};

// Основна поведінка Witch
class WitchBehavior : public Action<IWitch>
{
private:
    int m_witchId;
    ConVarRef z_witch_speed_inured;

public:
    WitchBehavior();
    virtual ~WitchBehavior();

    virtual const char* GetName() const override
    { 
        return "WitchBehavior";
    }

    virtual ActionResult< IWitch >      Update(IWitch* me, float interval) override;
    virtual ActionResult< IWitch >      OnStart(IWitch* me, Action<IWitch> *) override;

    virtual Action< IWitch >            *InitialContainedAction(IWitch* me) override;

    virtual EventDesiredResult< IWitch > OnIgnite(IWitch* me) override;
    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< IWitch > OnKilled(IWitch* me, const CTakeDamageInfo& info) override;
};

// Основна дія Witch
class WitchMainAction : public Action<IWitch>
{
    ConVarRef z_witch_burn_time;
public:
    WitchMainAction() : z_witch_burn_time("z_witch_burn_time") { }
    virtual ~WitchMainAction() { }

    virtual const char* GetName(void) const override
    {
        return "WitchMainAction";
    }

    virtual ActionResult< IWitch >	    Update(IWitch* me, float interval) override;

    virtual Action< IWitch >            *InitialContainedAction(IWitch* me) override;

    virtual EventDesiredResult< IWitch > OnInjured(IWitch* me, const CTakeDamageInfo& info) override;
};

// Основний намір Witch
class WitchIntention : public IIntention
{
public:
    WitchIntention(INextBot *me);
    virtual ~WitchIntention();
    virtual WitchMainAction* InitialAction(void);
    virtual bool IsAbleToBlokMovementOf(const INextBot*);
    virtual void Reset() override;
    virtual void Update( void ) override;
    virtual INextBotEventResponder *FirstContainedResponder( void ) const override
    {
        return m_behavior;
    }
    
    virtual INextBotEventResponder *NextContainedResponder( INextBotEventResponder *current ) const override
    {
        return nullptr;
    }

private:
    IWitch *m_me;                   // Закешований вказівник на Witch
    Behavior<IWitch> *m_behavior;   // Основна поведінка Witch
};


#endif //_HEADER_MY_WITCH_INCLUDE_