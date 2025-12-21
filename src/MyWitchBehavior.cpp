#include "MyWitch.h"

ConVar z_witch_evil("z_witch_evil", "0", FCVAR_CHEAT);

EventDesiredResult<IWitch> WitchBehavior::OnIgnite(IWitch *me)
{
    auto pEffectEntity = me->GetEffectEntity();
    IBaseEntity* pAttackr = nullptr;

    if(pEffectEntity)
    {
        if(pEffectEntity->ClassMatches("entityflame"))
        {
            IHANDLES &hAttacker = access_member<IHANDLES>(pEffectEntity, 230*4);
            if(hAttacker != nullptr)
            {
                pAttackr = hAttacker.Get();
            }
            me->SetHarasser(pAttackr);
        }
    }

    me->GetLocomotionInterface()->SetSpeedLimit(z_witch_speed_inured.GetFloat());

    return TrySuspendFor(new WitchBurn(pAttackr), RESULT_IMPORTANT, "Caught on fire!");
}

int GetRandomSurvivor()
{
	int iPlayerCount[MAX_PLAYERS + 1];
	int iCount = -1;
	for(int i(1); i <= g_pGlobals->maxClients; i++)
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(i);
		if(pClient && pClient->IsConnected() && pClient->IsInGame())
		{
			IPlayerInfo *pPlayer = pClient->GetPlayerInfo();
			if(pPlayer && pPlayer->GetTeamIndex() == 2)
			{
				if(!pPlayer->IsDead() && pPlayer->GetHealth() > 0)
				{
					iPlayerCount[++iCount] = i;
				}
			}
		}
	}

	return iCount == -1 ? -1 : (iCount > 0 ? iPlayerCount[RandomInt(0, iCount)] : iPlayerCount[0]);
}

WitchBehavior::WitchBehavior() : 
    m_witchId(-1), 
    z_witch_speed_inured("z_witch_speed_inured")
{
}

WitchBehavior::~WitchBehavior()
{
    IParticleSystem* pParticleLeye = nullptr;
    IParticleSystem* pParticleReye = nullptr;

    g_pWitchEvil->GetParticleData(m_witchId, &pParticleLeye, &pParticleReye);
    if(pParticleLeye)
        g_CallHelper->UTIL_Remove(pParticleLeye);

    if(pParticleReye)
        g_CallHelper->UTIL_Remove(pParticleReye);

    g_pWitchEvil->ClearData(m_witchId);
}

Action<IWitch> *WitchBehavior::InitialContainedAction(IWitch *me)
{
    if(me->ClassMatches("witch") && access_member<int>(me, 3668))
    {
        int iPlayer = GetRandomSurvivor();
        if(iPlayer >= 1 && iPlayer <= g_pGlobals->maxClients)
        {
            if(g_pWitchEvil->IsWitchEvil(me->entindex()))
            {
                return new WitchEvilAttack(GetVirtualClass<ITerrorPlayer>(iPlayer));
            }
            
            return new WitchAttack(GetVirtualClass<ITerrorPlayer>(iPlayer));
        }
    }

    if(g_Sample.my_bStrcmp("models/infected/witch_walk_ver3.mdl", me->GetModelName().ToCStr()))
        return new WitchWander;
    else
        return new WitchIdle;
}

EventDesiredResult<IWitch> WitchBehavior::OnKilled(IWitch *me, const CTakeDamageInfo &info)
{
    WitchKilledMusic killMusic;
    ForEachSurvivor(killMusic);
    return TryChangeTo(new WitchDying(info), RESULT_CRITICAL, "I am dead!");
}

ActionResult<IWitch> WitchBehavior::Update(IWitch *me, float interval)
{
    return Continue();
}

ActionResult<IWitch> WitchBehavior::OnStart(IWitch *me, Action<IWitch> *)
{
    m_witchId = me->entindex();
    return Continue();
}

EventDesiredResult<IWitch> WitchBehavior::OnInjured(IWitch *me, const CTakeDamageInfo &info)
{
    if((info.GetDamageType() & DMG_BLAST) != 0 && info.GetDamage() / (info.GetMaxDamage() + 0.001f) > 0.5f)
    {
        CBaseEntity *pAttacker = nullptr;

        me->GetLocomotionInterface()->SetSpeedLimit(z_witch_speed_inured.GetFloat());
        if((pAttacker = info.GetInflictor()) != nullptr)
        {
            me->GetBodyInterface()->AimHeadTowards(pAttacker, IBody::IMPORTANT);
        }

        me->EmitSound("WitchZombie.Rage");

        return TrySuspendFor( new WitchShoved((IBaseEntity *)pAttacker));
    }

    return TryContinue();
}
