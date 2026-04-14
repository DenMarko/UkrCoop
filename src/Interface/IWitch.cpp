#include "IWitch.h"
#include "ITerrorPlayer.h"
#include "extension.h"

class WitchHarraserSet : public EVENTS::CBaseEvent
{
public:
    WitchHarraserSet() : EVENTS::CBaseEvent("witch_harasser_set") { }
    virtual ~WitchHarraserSet() { }

    void Set(int _userid, int _witchid)
    {
        if(pEvent)
        {
            pEvent->SetInt("userid", _userid);
            pEvent->SetInt("witchid", _witchid);
        }
    }
};

void IWitch::SetHarasser(IBaseEntity *pEnt)
{
    IHANDLES &pAttacker = access_member<IHANDLES>(this, 910*4);
    if(pEnt)
    {
        pAttacker = pEnt->GetRefEHandle();
        if(pEnt->IsPlayer())
        {
            WitchHarraserSet harasser;
            harasser.Set(engine->GetPlayerUserId(pEnt->edict()), entindex());
        }
    }
    else
    {
        pAttacker.Term();
    }
    return;
}

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
                    if(!func(pPlayer))
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

class DispatchResponseToSurvivor
{
    const char* szStr1;
    const char* szmodifiers;
    ITerrorPlayer* pIgnore;
public:
    DispatchResponseToSurvivor(const char* Str1, const char *modifiers = NULL, ITerrorPlayer* IgnorePlayer = nullptr) : szStr1(Str1), szmodifiers(modifiers), pIgnore(IgnorePlayer)
    { }

    virtual bool operator()(ITerrorPlayer* pPlayer)
    {
        if(!pPlayer 
        || !pPlayer->IsPlayer() 
        || pPlayer == pIgnore 
        || pPlayer->GetTeamNumber() != 2)
        {
            return true;
        }

        CAI_Concept _concept(szStr1);
        pPlayer->SpeakIfAllowed(_concept, SPEECH_PRIORITY_NORMAL, szmodifiers, 0, 0, 0);
        return true;
    }
};

void IWitch::ChangeRageLevel(float flRale)
{
    float &m_rage = access_member<float>(this, 3632);
    float oldRage = m_rage;

    float newRage = clamp(flRale + m_rage, 0.0f, 1.0f);

    if(newRage != m_rage)
    {
        NetworkStateChanged(3632);
        m_rage = newRage;
    }

    if(oldRage < 0.5f && newRage >= 0.5f)
    {
        DispatchResponseToSurvivor dispatch("WitchGettingAngry");
        ForEachSurvivor(dispatch);
    }
}

bool IWitch::IsHostileToMe(ITerrorPlayer *pPlayer)
{
    ConVarRef witch_personal_space("z_witch_personal_space");
    ConVarRef witch_flashlight_range("z_witch_flashlight_range");

    static constexpr int TEAM_INFECTED = 3;

    if(!pPlayer || !pPlayer->IsPlayer())
        return false;

    if(pPlayer->GetTeamNumber() == TEAM_INFECTED)
        return false;

    auto pBot = MyNextBotPointer();
    if(!pBot) return false;

    auto IsHostileDueToFlashLight = [](ITerrorPlayer* pPlayers, IWitch* my, INextBot* pBots, float flashLightRange)
    {
        static constexpr float FLASHLIGHT_LOOK_THRESHOLD = 0.98f;

        return  pPlayers->FlashlightIsOn() &&
                pPlayers->IsLookingTowards((CBaseEntity*)my, FLASHLIGHT_LOOK_THRESHOLD) &&
                pBots->IsRangeLessThan((CBaseEntity*)pPlayers, flashLightRange) && 
                my->GetVisionInterface()->IsAbleToSee((CBaseEntity*)pPlayers, IVision::USE_FOV);
    };

    // Перевіряємо ворожість через ліхтарик (найвища пріоритетність)
    if(IsHostileDueToFlashLight(pPlayer, this, pBot, witch_flashlight_range.GetFloat())) return true;

    auto IsHostileDueToWeaponFire = [](ITerrorPlayer* pPlayers)
    {
        if(!pPlayers->IsFiringWeapon()) return false;
        
        static constexpr int FL_SPECIAL_IMMUNITY = 4;
        const char playerFlags = access_member<char>(pPlayers, 11078);
        const bool hasSpecialImmuniti = (playerFlags & FL_SPECIAL_IMMUNITY) != 0;

        return !hasSpecialImmuniti;
    };

    // Перевіряємо ворожість через стрільбу
    if(IsHostileDueToWeaponFire(pPlayer)) return true;

    // Перевіряємо ворожість через вторгнення в персональний простір
    return pBot->IsRangeLessThan((CBaseEntity*)pPlayer, witch_personal_space.GetFloat());
}

void CalculateMeleeDamageForce( CTakeDamageInfo *info, const Vector &vecMeleeDir, const Vector &vecForceOrigin, float flScale )
{
    auto ImpScale = [](float flTargetMass, float flDesiredSpeed)
    {
        return (flTargetMass * flDesiredSpeed);
    };

    ConVarRef phys_pushscale("phys_pushscale");
	info->SetDamagePosition( vecForceOrigin );

	// Calculate an impulse large enough to push a 75kg man 4 in/sec per point of damage
	float flForceScale = info->GetBaseDamage() * ImpScale( 75, 4 );
	Vector vecForce = vecMeleeDir;
	VectorNormalize( vecForce );
	vecForce *= flForceScale;
	vecForce *= phys_pushscale.GetFloat();
	vecForce *= flScale;
	info->SetDamageForce( vecForce );
}

class WitchSlashEnumerate : public IPartitionEnumerator
{
private:
    IWitch *m_witch;                    // 4
    ITerrorPlayer *m_pVictim;           // 8 - конкретна ціль для атаки
    Vector m_vecForward;                // 12
    bool m_bHitSomething;               // 24
    bool m_bHitSurvivor;                // 25

    INextBot *m_pNextBot;
    IWitchVision* m_pVision;
    ConVarRef m_attackRange;

public:
    WitchSlashEnumerate(IWitch *me, ITerrorPlayer* pVictim, const Vector &vecForvard) : 
    m_attackRange("z_witch_attack_range")
    {
        m_witch = me;
        m_pVictim = pVictim;
        m_vecForward = vecForvard;

        m_pNextBot = m_witch->MyNextBotPointer();
        m_pVision = m_witch->GetVisionInterface();

        m_bHitSomething = false;
        m_bHitSurvivor = false;
    }

    virtual IterationRetval_t EnumElement(IHandleEntity *entityHandle) override
    {
        ITerrorPlayer *entity = GetPlayerFromHandle(entityHandle->GetRefEHandle());
        if (!entity || entity == (IBaseEntity*)m_witch)
            return ITERATION_CONTINUE;

        if (entity->IsPlayer())
        {
            if(m_pVictim)
            {
                if(entity->entindex() == m_pVictim->entindex())
                {
                    if(!entity->IsIncapacitated())
                    {
                        if(IsInAttackDirection(entity))
                        {
                            return ProcessUniversalChecks(entity, m_attackRange.GetFloat());
                        }
                    }
                }
            }
            return ITERATION_CONTINUE;
        }

        return ProcessUniversalChecks(entity, m_attackRange.GetFloat());
    }

    inline bool WasAttackPerformed() { return m_bHitSomething; }
    inline bool WasSecondaryAttackUsed() { return m_bHitSurvivor; }

    // Додаткові методи для дебагу
    inline ITerrorPlayer* GetCurrentTarget() const { return m_pVictim; }
    inline bool HasSpecificTarget() const { return m_pVictim != nullptr; }

private:
    
    bool IsInAttackDirection(ITerrorPlayer* entity) const
    {
        Vector toTarget = entity->GetAbsOrigin() - m_witch->WorldSpaceCenter();
        return toTarget.Dot(m_vecForward) >= 0.0f;
    }
    
    IterationRetval_t ProcessUniversalChecks(ITerrorPlayer* entity, float flAttackRange)
    {
        if (!m_pNextBot->IsRangeLessThan((CBaseEntity*)entity, flAttackRange))
            return ITERATION_CONTINUE;

        if (!m_pVision->IsLineOfSightClearToEntity((CBaseEntity*)entity))
            return ITERATION_CONTINUE;

        if (!entity->MyCombatCharacterPointer())
        {
            ProcessStaticObject(entity);
            return ITERATION_CONTINUE;
        }
        else
        {
            if(entity->GetTeamNumber() == 2)
            {
                if(m_bHitSurvivor) return ITERATION_CONTINUE;

                m_bHitSurvivor = true;
            }

            ProcessCombatTarget(entity);
            return ITERATION_CONTINUE;
        }
    }

    void ProcessStaticObject(ITerrorPlayer* entity)
    {
        float flDamage = 0.f;
        if (entity->ClassMatches("func_door_rotating") || entity->ClassMatches("prop_door*"))
        {
            IBaseEntity *pEnt = access_dynamic_cast<IBaseEntity>(entity, "CBasePropDoor");
            if (pEnt) {
                pEnt->EmitSound(access_member<string_t>(pEnt, 442 * 4).ToCStr());
            } else {
                entity->EmitSound("WoodenDoor.Pound");
            }
            flDamage = 1000.f;
        }
        else
        {
            m_witch->EmitSound("Zombie.ClawScrape");
            flDamage = 100.f;
        }
        InflictDamage(entity, flDamage);
        m_bHitSomething = true;
    }

    void ProcessCombatTarget(ITerrorPlayer* entity)
    {
        entity->EmitSound("WitchZombie.ShredVictim");
        SendBloodSplatterMessage();
        InflictDamage(entity, 100.0f);
        m_bHitSomething = true;
    }

    void InflictDamage(ITerrorPlayer *entity, float damageAmount)
    {
        Vector vec_eye = m_witch->EyePosition();

        CTakeDamageInfoHack damage(m_witch, m_witch, damageAmount, DMG_SLASH);
        auto vecEyePos = (vec_eye - entity->WorldSpaceCenter());
        VectorNormalize(vecEyePos);
        CalculateMeleeDamageForce(&damage, vecEyePos, vec_eye);
        entity->TakeDamage(damage);
    }

    void SendBloodSplatterMessage()
    {
        Vector vec_worldSpace = m_witch->WorldSpaceCenter();

        IPVSFilter filter(vec_worldSpace);
        filter.MakeReliable();
        CUserMessage msg(filter, "WitchBloodSplatter");
        msg.MsgWriteVec3Coord(vec_worldSpace);
    }

    NOINLINE ITerrorPlayer *GetPlayerFromHandle(const CBaseHandle &pHandleEntity)
    {
        IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity.Get();
        if (pUnk)
            return reinterpret_cast<ITerrorPlayer*>(pUnk->GetBaseEntity());

        return nullptr;
    }
};

bool IWitch::DoAttack(ITerrorPlayer *pVictim)
{
    ConVarRef z_witch_attack_range("z_witch_attack_range");
    Vector vecForvard;
    AngleVectors(EyeAngles(), &vecForvard);

    WitchSlashEnumerate ShereEnum(this, pVictim, vecForvard);
    g_pPartition->EnumerateElementsInSphere(PARTITION_ENGINE_SOLID_EDICTS, WorldSpaceCenter(), z_witch_attack_range.GetFloat(), false, &ShereEnum);

    if(!ShereEnum.WasAttackPerformed())
    {
        EmitSound("Zombie.AttackMiss");
        return false;
    }
    return true;
}
