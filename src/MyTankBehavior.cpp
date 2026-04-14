/**
 * Декомпільований код.
 * Реалізація поведінки танка
 */

#include "MyTank.h"

class PressFireButtonReply : public INextBotReply
{
public:
    virtual void OnSuccess(INextBot *bot) override
    {
        ITank *pTank = access_dynamic_cast<ITank>(reinterpret_cast<IBaseCombatCharacter*>(bot->GetEntity()), "Tank");
        if(pTank)
        {
            pTank->PressFireButton();
        }
    }
} g_mPressFireButtonReply;

TankBehavior::TankBehavior() : 
    tank_ground_pound_duration("tank_ground_pound_duration"), 
    tank_windup_time("tank_windup_time"), 
    tank_swing_duration("tank_swing_duration")
{
}

TankBehavior::~TankBehavior()
{
}

ActionResult<ITank> TankBehavior::Update(ITank *me, float interval)
{
    if(!access_member<bool>(me, (11973)))
    {
        IVision *pVision = me->GetVisionInterface();
        ITerrorPlayer* pEntity = (ITerrorPlayer*)pVision->GetPrimaryRecognizedThreat();

        if(!me->GetLocomotionInterface()->IsUsingLadder() 
        && pEntity 
        && me->MyNextBotPointer()->IsRangeLessThan((CBaseEntity*)pEntity, 50.f) 
        && pVision->IsLineOfSightClearToEntity((CBaseEntity*)pEntity))
        {
            me->GetBodyInterface()->AimHeadTowards(
                (CBaseEntity*)pEntity, 
                IBody::IMPORTANT, 
                pEntity->IsIncapacitated() 
                    ? tank_ground_pound_duration.GetFloat() : 
                    (tank_windup_time.GetFloat() + tank_swing_duration.GetFloat()) );

            me->PressFireButton();
        }
    }

    return Continue();
}

EventDesiredResult<ITank> TankBehavior::OnContact(ITank *me, CBaseEntity *other, CGameTrace *result)
{
    static INextBotReply *pReply = nullptr;
    if(!pReply)
        pReply = &g_mPressFireButtonReply;

    if(other)
    {
        if(g_CallHelper->IsBreakableEntity(other, me->GetTeamNumber() == 3 ? me->GetClass() == ZombieClassTank : false, true) ||
            reinterpret_cast<IBaseEntity*>(other)->ClassMatches("prop_door*") || 
            reinterpret_cast<IBaseEntity*>(other)->ClassMatches("func_door*"))
        {
            if(!result || result->fraction < 0.8f)
            {
                me->GetBodyInterface()->AimHeadTowards(reinterpret_cast<IBaseEntity*>(other)->WorldSpaceCenter(), IBody::CRITICAL, 1.f, pReply, "Looking at door I'm trying to break");
            }
        }
        else if(me->GetTeamNumber() == 3 
        && me->GetClass() == ZombieClassTank 
        && access_dynamic_cast<IBaseEntity*>((IBaseEntity*)other, "CPhysicsProp"))
        {
            me->PressFireButton();
        }
        else if(reinterpret_cast<IBaseEntity*>(other)->MyCombatCharacterPointer())
        {
            me->GetBodyInterface()->AimHeadTowards(reinterpret_cast<IBaseEntity*>(other)->WorldSpaceCenter(), IBody::IMPORTANT, 1.f, pReply, "Looking at actor I'm trying to attack");
        }
    }

    return TryContinue();
}

bool IsVersusMode()
{
    ConVarRef mp_gamemode("mp_gamemode");
    const char *gamemode = nullptr;
    if(mp_gamemode.IsFlagSet(FCVAR_NEVER_AS_STRING))
    {
        gamemode = "FCVAR_NEVER_AS_STRING";
    }
    else
    {
        if(mp_gamemode.GetString())
            gamemode = mp_gamemode.GetString();
        else
            gamemode = "";
    }


    if(!Q_stricmp(gamemode, "versus"))
        return true;

    if(!Q_stricmp(gamemode, "teamversus"))
        return true;

    return false;
}

Action<ITank> *TankBehavior::InitialContainedAction(ITank *me)
{
    void *pDirector = g_HL2->GetDirector();
    if(access_member<bool>(pDirector, 358) 
    || access_member<bool>(pDirector, 85)
    || IsVersusMode())
    {
        return new TankAttack();
    } else {
        if(access_member<bool>(me, (11972)))
            return new TankAttack();
        else
            return new TankIdle();
    }
}
