#ifndef _HEADER_MY_TANK_INCLUDE_
#define _HEADER_MY_TANK_INCLUDE_
#include "extension.h"

#include "Interface/ITank.h"
#include "Interface/INextBotChasePath.h"
#include "Interface/INavLadder.h"
#include "Interface/IMusic.h"
#include "Interface/INextBotBehavior.h"

extern bool IsVersusMode();

enum FlowType {
    TOWARD_GOAL = 0,
    AWAY_FROM_START
};

// Вмикання музики атаки танка
class TankAttackMusic
{
public:
    virtual bool operator()(ITerrorPlayer *pPlayer)
    {
        if(!pPlayer->IsPlayer())
            return true;

        if(!pPlayer->IsConnected())
            return true;

        if( IsVersusMode() && pPlayer->GetTeamNumber() == 2)
        {
            IMusic &m_music = access_member<IMusic>(pPlayer, 10160);
            m_music.OnBossApproaching();
        }
        return true;
    }
};

// Пошук гравця з мініганом
class MinigunnerScan
{
public:
    MinigunnerScan() : pBestMinigun(nullptr) {}

    virtual bool operator()(ITerrorPlayer* pPlayer)
    {
        if(!pPlayer->IsPlayer())
            return true;

        if(!pPlayer->IsConnected())
            return true;

        if(pPlayer->GetTeamNumber() != 2)
            return true;

        if(pPlayer->GetMinigun() != nullptr)
        {
            pBestMinigun = pPlayer;
        }
        return true;
    }

    inline ITerrorPlayer *GetBestPlayer() const { return pBestMinigun; }

private:
    ITerrorPlayer* pBestMinigun;
};

// Виправлення блокування драбини танкам
class LadderBlockFix
{
    INextBot *m_pNextBot;
    ITank* m_me;

    IntervalTimers m_timer;
    CUtlVector<IHANDLES> m_PlayerCollect;
public:
    LadderBlockFix();
    void Init(ITank* me);

    void UpdateTimer();

    virtual bool operator()(ITerrorPlayer* pPlayer);
};

// Вартість шляху для переслідування інфікованого
class InfectedPathCost : public IPathCost
{
private:
    INextBot* m_pBot;
    IBaseCombatCharacter* m_pEnemy;
    ILocomotion* m_pLocomotion;
    float m_flRandomFactor;

public:
    InfectedPathCost(INextBot* bot);
    InfectedPathCost(ITank* bot);
    virtual float operator()(INavArea *area, INavArea *fromArea, const INavLadder *ladder, const CFuncElevator *elevator, float length) const;
};

template < typename Actor, typename PathCost >
class BehaviorMoveTo : public Action< Actor >
{
public:
	BehaviorMoveTo( const Vector &goal, Action< Actor > *successAction = NULL, Action< Actor > *failAction = NULL );

	virtual ActionResult< Actor > OnStart( Actor *me, Action< Actor > *priorAction );
	virtual ActionResult< Actor > Update( Actor *me, float interval );

	virtual EventDesiredResult< Actor > OnMoveToSuccess( Actor *me, const Path *path );
	virtual EventDesiredResult< Actor > OnMoveToFailure( Actor *me, const Path *path, MoveToFailureType reason );

	virtual bool ComputePath( Actor *me, const Vector &goal, IPathFollower *path );

	virtual const char *GetName( void ) const	{ return "BehaviorMoveTo"; }

private:
	Vector m_goal;
	IPathFollower m_path;
	Action< Actor > *m_successAction;
	Action< Actor > *m_failAction;
};

template < typename Actor, typename PathCost >
inline BehaviorMoveTo< Actor, PathCost >::BehaviorMoveTo( const Vector &goal, Action< Actor > *successAction, Action< Actor > *failAction )
{
	m_goal = goal;
	m_path.Invalidate();
	m_successAction = successAction;
	m_failAction = failAction;
}

template < typename Actor, typename PathCost >
inline bool BehaviorMoveTo< Actor, PathCost >::ComputePath( Actor *me, const Vector &goal, IPathFollower *path )
{
	PathCost cost( me );
	return (!me || !path->Compute( me, goal, cost ));
}

template < typename Actor, typename PathCost >
inline ActionResult< Actor > BehaviorMoveTo< Actor, PathCost >::OnStart( Actor *me, Action< Actor > *priorAction )
{
	if ( !this->ComputePath( me, m_goal, &m_path ) )
	{
		if ( m_failAction )
		{
			return this->ChangeTo( m_failAction, "No path to goal" );
		}

		return this->Done( "No path to goal" );
	}

	return this->Continue();
}

template < typename Actor, typename PathCost >
inline ActionResult< Actor > BehaviorMoveTo< Actor, PathCost >::Update( Actor *me, float interval )
{
	if ( !m_path.IsValid() )
	{
		if ( m_failAction )
		{
			return this->ChangeTo( m_failAction, "Path is invalid" );
		}

		return this->Done( "Path is invalid" );
	}

	m_path.Update( me );

	return this->Continue();
}

template < typename Actor, typename PathCost >
inline EventDesiredResult< Actor > BehaviorMoveTo< Actor, PathCost >::OnMoveToSuccess( Actor *me, const Path *path )
{
	if ( m_successAction )
	{
		return this->TryChangeTo( m_successAction, RESULT_CRITICAL, "OnMoveToSuccess" );
	}

	return this->TryDone( RESULT_CRITICAL, "OnMoveToSuccess" );
}

template < typename Actor, typename PathCost >
inline EventDesiredResult< Actor > BehaviorMoveTo< Actor, PathCost >::OnMoveToFailure( Actor *me, const Path *path, MoveToFailureType reason )
{
	if ( m_failAction )
	{
		return this->TryChangeTo( m_failAction, RESULT_CRITICAL, "OnMoveToFailure" );
	}

	return this->TryDone( RESULT_CRITICAL, "OnMoveToFailure" );
}

// Подія створення події запалення зомбі
class CZombieIgnite : public EVENTS::CBaseEvent
{
public:
    CZombieIgnite() : EVENTS::CBaseEvent("zombie_ignited") { }
    virtual ~CZombieIgnite() { }

    void Set(int _userid, int _entityid, const char *_victimname)
    {
        if(pEvent)
        {
            pEvent->SetInt("userid", _userid);
            pEvent->SetInt("entityid", _entityid);
            pEvent->SetString("victimname", _victimname);
        }
    }
};

class TankAttack : public Action< ITank >
{
public:
    TankAttack();
    virtual ~TankAttack();
    virtual const char* GetName() const override
    {
        return "TankAttack";
    }

    virtual EventDesiredResult< ITank > OnCommandAttack(ITank* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< ITank > OnMoveToSuccess(ITank* me, const Path* path) override;
    virtual EventDesiredResult< ITank > OnUnStuck(ITank* me) override;
    virtual EventDesiredResult< ITank > OnShoved(ITank* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< ITank > OnMoveToFailure(ITank* me, const Path* path, MoveToFailureType reason) override;
    virtual EventDesiredResult< ITank > OnStuck(ITank* me) override;
    virtual EventDesiredResult< ITank > OnContact(ITank* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
    virtual EventDesiredResult< ITank > OnInjured(ITank* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< ITank > OnCommandApproach(ITank* me, const Vector& pos, float maxDistance) override;

    virtual ActionResult< ITank > OnStart(ITank* me, Action<ITank> *priorAction) override;
    virtual ActionResult< ITank > Update(ITank* me, float interval) override;

private:
    void UpdateThrowAimError( void );
    void TryToThrowRock( ITank *me );
    ITerrorPlayer* GetTarget( void ) const;
    void SetTarget(ITerrorPlayer *pTarget);

    const int GetDifficulty() const;

    CHandle<ITerrorPlayer> m_hPlayer;       // Поточна цільова гравець
    DirectChasePath m_path;                 // Шлях переслідування

    IntervalTimers m_attackTimer;           // Таймер атаки (використовується в OnStart та TryToThrowRock)
    CountdownTimers m_unusedTimer;          // Або можна видалити
    CountdownTimers m_throwCooldown;        // Кулдаун кидання каменя
    IntervalTimers m_stuckTimer;            // Таймер для відстеження застрягання
    IntervalTimers m_stasisTimer;           // Таймер стазису перед самогубством
    float m_throwAimError;                  // Похибка прицілювання при киданні
    
    LadderBlockFix m_blockLader;            // Виправлення блокування драбини

    ConVarRef cv_stuckFailsafe;             // tank_stuck_failsafe
    ConVarRef cv_stasisSuicideTime;         // tank_stasis_time_suicide
    ConVarRef cv_stuckNewTargetTime;        // tank_stuck_time_choose_new_target
    ConVarRef cv_stuckNewTargetVisTol;      // tank_stuck_visibility_tolerance_choose_new_target
    ConVarRef cv_stuckSuicideTime;          // tank_stuck_time_suicide
    ConVarRef cv_stuckSuicideVisTol;        // tank_stuck_visibility_tolerance_suicide
    ConVarRef cv_visSuicideTol;             // tank_visibility_tolerance_suicide
    ConVarRef cv_allowAiAbilities;          // z_allow_ai_to_use_abilities
    ConVarRef cv_throwLoftRate;             // tank_throw_loft_rate
    ConVarRef cv_throwMaxLoft;              // tank_throw_max_loft_angle
    ConVarRef cv_throwForce;                // z_tank_throw_force
    ConVarRef cv_throwLeadFactor;           // tank_throw_lead_time_factor
    ConVarRef cv_throwAimError;             // tank_throw_aim_error
    ConVarRef cv_throwRange;                // tank_throw_allow_range
    ConVarRef cv_throwInterval;             // tank_throw_min_interval
};


class TankIdle : public Action<ITank>
{
public:
    TankIdle();
    virtual ~TankIdle();
    virtual const char* GetName() const override
    {
        return "TankIdle";
    }

    virtual ActionResult< ITank > Update(ITank* me, float interval) override;
    virtual ActionResult< ITank > OnStart(ITank* me, Action<ITank> *priorAction) override;

    virtual EventDesiredResult< ITank > OnCommandAttack(ITank* me, CBaseEntity* victim) override;
    virtual EventDesiredResult< ITank > OnCommandApproach(ITank* me, const Vector& pos, float maxDistance) override;
    virtual EventDesiredResult< ITank > OnInjured(ITank* me, const CTakeDamageInfo& info) override;
    virtual EventDesiredResult< ITank > OnShoved(ITank* me, CBaseEntity* pusher) override;
    virtual EventDesiredResult< ITank > OnContact(ITank* me, CBaseEntity* other, CGameTrace *result = nullptr) override;
};

class TankBehavior : public Action<ITank>
{
public:
    TankBehavior();

    virtual ~TankBehavior();

    virtual const char* GetName() const override
    { 
        return "TankBehavior";
    }

    virtual ActionResult< ITank >               Update(ITank* me, float interval) override;
    virtual EventDesiredResult< ITank >         OnContact(ITank* me, CBaseEntity* other, CGameTrace *result = nullptr) override;

    virtual Action< ITank >                     *InitialContainedAction(ITank* me) override;

private:
    ConVarRef tank_ground_pound_duration;
    ConVarRef tank_windup_time;
    ConVarRef tank_swing_duration;
};

class TankIntention : public IIntention
{
public:
    TankIntention(INextBot *me);
    virtual ~TankIntention();

    virtual void Reset() override;
    virtual void Update( void ) override;
    virtual INextBotEventResponder *FirstContainedResponder( void ) const override
    {
        return m_behavior;
    }

private:
    ITank *m_me;                    // Збережений вказівник на ITank
    Behavior<ITank> *m_behavior;    // Поведінковий об'єкт
};

#endif //_HEADER_MY_TANK_INCLUDE_