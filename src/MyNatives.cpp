#include "Interface/ISurvivorBot.h"
#include "Interface/IBaseCombatWeapon.h"
#include "Interface/IGameRules.h"
#include "Interface/ISurvivorRescue.h"
#include "Interface/IMusic.h"
#include "Interface/IProps.h"

#include "log_messege.h"
#include "CAddAbility.h"
#include "TickRegen.h"
#include "ConVar_l4d.h"
#include "fmtstr.h"
#include "CTrace.h"
#include "CAmmoDef.h"
#include "MyNatives.h"
#include "CHandleOpen.h"

#define BEGIN_NATIVES(prefix) const sp_nativeinfo_t g_##prefix##Natives[] = \
	{

#define STRINGIFY(string) #string
#define ADD_NATIVE(prefix) {STRINGIFY(prefix), native_##prefix},

#define ADD_NEW_SINTASIS_NATIVE(name, prefix) {STRINGIFY(name), native_##prefix},

#define END_NATIVES() {NULL, NULL} \
	};

#define FUNC_NATIVE(name_func) static cell_t native_##name_func(IPluginContext *pContext, const cell_t *param)

FUNC_NATIVE(LogMessegToFile)
{
	g_pSM->SetGlobalTarget(0);
	char meseg[1024];

	g_pSM->FormatString(meseg, sizeof(meseg), pContext, param, 1);

	if(pContext->GetLastNativeError() != 0)
	{
		return 0;
	}
	IPlugin *pPlugins = plsys->FindPluginByContext(pContext->GetContext());
	return m_sLog->LogToFileEx(false, "[%s] %s", pPlugins->GetFilename(), meseg);
}

FUNC_NATIVE(BotCreater)
{
	ISurvivorBot* pBot = (ISurvivorBot*)g_pBotCreator->SpawnSurvivor();
	if(pBot) {
		return pBot->entindex();
	}
	return -1;
}

FUNC_NATIVE(Respawn)
{
    int client = param[1];

	IGamePlayer *pClient = nullptr;
	if((client >= 1) && (client <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(client);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", client);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if(client <= 0 || client > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", client);
	}

	edict_t *pEdict = g_Sample.PEntityOfEntIndex(client);
	if(!pEdict || pEdict->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");

	GetVirtualClass<ITerrorPlayer>(client)->RoundRespawn();

    return 1;
}

FUNC_NATIVE(l4d_staggered)
{
	if(param[1] <= 0 || param[1] > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", param[1]);
	}

	if(param[2] <= 0 || param[2] > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", param[2]);
	}

	IGamePlayer *pClient = nullptr;
	IGamePlayer *pTarget = nullptr;
	if((param[1] >= 1) && (param[1] <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(param[1]);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", param[1]);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if((param[2] >= 1) && (param[2] <= playerhelpers->GetMaxClients()))
	{
		pTarget = playerhelpers->GetGamePlayer(param[2]);
		if(!pTarget)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", param[2]);
		}
		if(!pTarget->IsConnected() && !pTarget->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	edict_t *p_eClient = g_Sample.PEntityOfEntIndex(param[1]);
	if(!p_eClient || p_eClient->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");
	

	edict_t *p_eTarget = g_Sample.PEntityOfEntIndex(param[2]);
	if(!p_eTarget || p_eTarget->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");
	

    cell_t *sVector;
    Vector *s_Vector = NULL, Vector_s;

    pContext->LocalToPhysAddr(param[3], &sVector);

    if(sVector != pContext->GetNullRef(SP_NULL_VECTOR))
    {
        Vector_s[0] = sp_ctof(sVector[0]);
        Vector_s[1] = sp_ctof(sVector[1]);
        Vector_s[2] = sp_ctof(sVector[2]);
        s_Vector = &Vector_s;
    }

	GetVirtualClass<ITerrorPlayer>(param[1])->OnStaggered((IBaseEntity*)p_eTarget->GetUnknown()->GetBaseEntity(), s_Vector);
    return 1;
}

FUNC_NATIVE(PVomitUpon)
{
    int client, target;
    int val;

    client = param[1];
    target = param[2];
	val = param[3] ? 1 : 0;

	IGamePlayer *pClient = nullptr;
	IGamePlayer *pTarget = nullptr;
	if((client >= 1) && (client <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(client);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", client);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if((target >= 1) && (target <= playerhelpers->GetMaxClients()))
	{
		pTarget = playerhelpers->GetGamePlayer(target);
		if(!pTarget)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", target);
		}
		if(!pTarget->IsConnected() && !pTarget->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if(client <= 0 || client > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", client);
	}

	if(target <= 0 || target > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", target);
	}

	edict_t *p_eClient = g_Sample.PEntityOfEntIndex(client);
	if(!p_eClient || p_eClient->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");


	edict_t *p_eTarget = g_Sample.PEntityOfEntIndex(target);
	if(!p_eTarget || p_eTarget->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");


    g_CallHelper->PlayerVomitUpon(p_eClient->GetUnknown()->GetBaseEntity(), p_eTarget->GetUnknown()->GetBaseEntity(), val);
    return 1;
}

FUNC_NATIVE(TakeOverBots)
{
	int client = param[1];
	int bot = param[2];

	IGamePlayer *pClient = nullptr;
	IGamePlayer *pBot = nullptr;
	if((client >= 1) && (client <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(client);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", client);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if((bot >= 1) && (bot <= playerhelpers->GetMaxClients()))
	{
		pBot = playerhelpers->GetGamePlayer(bot);
		if(!pBot)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", bot);
		}
		if(!pBot->IsConnected() && !pBot->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if(client <= 0 || client > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", client);
	}

	if(bot <= 0 || bot > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", bot);
	}

	edict_t *p_eClient = g_Sample.PEntityOfEntIndex(client);
	if(!p_eClient || p_eClient->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");


	edict_t *p_eBot = g_Sample.PEntityOfEntIndex(bot);
	if(!p_eBot || p_eBot->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");


	g_CallHelper->setHumansSpec(p_eBot->GetUnknown()->GetBaseEntity(), p_eClient->GetUnknown()->GetBaseEntity());
	g_CallHelper->takeOversBot(p_eClient->GetUnknown()->GetBaseEntity(), true);

	return 1;
}

FUNC_NATIVE(SetHumSpec)
{
	int bot = param[1];
	int client = param[2];

	IGamePlayer *pClient = nullptr;
	IGamePlayer *pBot = nullptr;
	if((client >= 1) && (client <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(client);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", client);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if((bot >= 1) && (bot <= playerhelpers->GetMaxClients()))
	{
		pBot = playerhelpers->GetGamePlayer(bot);
		if(!pBot)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", bot);
		}
		if(!pBot->IsConnected() && !pBot->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if(client <= 0 || client > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", client);
	}

	if(bot <= 0 || bot > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", bot);
	}

	edict_t *p_eClient = g_Sample.PEntityOfEntIndex(client);
	if(!p_eClient || p_eClient->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");


	edict_t *p_eBot = g_Sample.PEntityOfEntIndex(bot);
	if(!p_eBot || p_eBot->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");

	
	g_CallHelper->setHumansSpec(p_eBot->GetUnknown()->GetBaseEntity(), p_eClient->GetUnknown()->GetBaseEntity());

	return 1;
}

FUNC_NATIVE(l4dTakeOverBot)
{
	bool switchs = param[2] ? true : false;

	edict_t *pEdict = nullptr;
	IGamePlayer *pClient = nullptr;
	if((param[1] >= 1) && (param[1] <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(param[1]);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", param[1]);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}
	
	if(param[1] <= 0 || param[1] > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", param[1]);
	}

	pEdict = g_Sample.PEntityOfEntIndex(param[1]);
	if(!pEdict || pEdict->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");


	g_CallHelper->takeOversBot(pEdict->GetUnknown()->GetBaseEntity(), switchs);

	return 1;
}

FUNC_NATIVE(Revive)
{
	edict_t *pEdict = nullptr;
	IGamePlayer *pClient = nullptr;
	if((param[1] >= 1) && (param[1] <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(param[1]);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", param[1]);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if(param[1] <= 0 || param[1] > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", param[1]);
	}

	pEdict = g_Sample.PEntityOfEntIndex(param[1]);
	if(!pEdict || pEdict->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");

	GetVirtualClass<ITerrorPlayer>(param[1])->OnRevived();

	return 1;
}

FUNC_NATIVE(OnTakeHealth)
{
	edict_t *pEdict = nullptr;
	IGamePlayer *pClient = nullptr;
	if((param[1] >= 1) && (param[1] <= playerhelpers->GetMaxClients()))
	{
		pClient = playerhelpers->GetGamePlayer(param[1]);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", param[1]);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}

	if(param[1] <= 0 || param[1] > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", param[1]);
	}

	pEdict = g_Sample.PEntityOfEntIndex(param[1]);
	if(!pEdict || pEdict->IsFree())
		return pContext->ThrowNativeError("edict is not valid or is free");
	
	return GetVirtualClass<ITerrorPlayer>(param[1])->TakeHealth(sp_ctof(param[2]), 0);
}

FUNC_NATIVE(RegenExtra)
{
	int client = param[1];

	if((client >= 1) && (client <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(client);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", client);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
	}
	else if(client <= 0 || client > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", client);
	}
	else
	{
		edict_t *pClient = g_Sample.PEntityOfEntIndex(client);
		if(!pClient || pClient->IsFree())
			return pContext->ThrowNativeError("edict is not valid or is free");
	}
	g_pRegen->SetExtraExec(client, param[2] ? true : false, param[3], sp_ctof(param[4]));

	return 1;
}

FUNC_NATIVE(IsRescueLiveSurvInside)
{
	CBaseEntity *gRescue = gamehelpers->ReferenceToEntity(param[1]);
	if(gRescue == nullptr)
	{
		return pContext->ThrowNativeError("Rescue Entity is not valid");
	}

	return g_CallHelper->IsLiveSurInside(gRescue);
}

FUNC_NATIVE(IsRescueBehinClosesDoor)
{
	ISurvivorRescue *gRescue = (ISurvivorRescue *)gamehelpers->ReferenceToEntity(param[1]);
	if(gRescue == nullptr)
	{
		return pContext->ThrowNativeError("Rescue Entity is not valid");
	}

	if(!gRescue->ClassMatches("info_survivor_rescue"))
	{
		return pContext->ThrowNativeError("Entity is not a \"CSurvivorRescue\", Entity Class Name: %s(%s)", gRescue->GetClassname().ToCStr(), gRescue->GetDebugName());
	}

	return static_cast<int>(gRescue->IsBehindClosedDoor());
}

FUNC_NATIVE(RescueCloseDoors)
{
	ISurvivorRescue *gRescue = (ISurvivorRescue *)gamehelpers->ReferenceToEntity(param[1]);
	if(gRescue == nullptr)
	{
		return pContext->ThrowNativeError("Rescue Entity is not valid");
	}

	if(!gRescue->ClassMatches("info_survivor_rescue"))
	{
		return pContext->ThrowNativeError("Entity is not a \"CSurvivorRescue\", Entity Class Name: %s(%s)", gRescue->GetClassname().ToCStr(), gRescue->GetDebugName());
	}
	return static_cast<int>(gRescue->CloseDoors());
}

FUNC_NATIVE(RescueSpawn)
{
	ISurvivorRescue *gRescue = (ISurvivorRescue *)gamehelpers->ReferenceToEntity(param[1]);
	if(gRescue == nullptr)
	{
		return pContext->ThrowNativeError("Rescue Entity is not valid");
	}

	if(!gRescue->ClassMatches("info_survivor_rescue"))
	{
		return pContext->ThrowNativeError("Entity is not a \"CSurvivorRescue\", Entity Class Name: %s(%s)", gRescue->GetClassname().ToCStr(), gRescue->GetDebugName());
	}
	
	gRescue->Spawn();
	return 1;
}

FUNC_NATIVE(IsRescueOpen)
{
	ISurvivorRescue *gRescue = reinterpret_cast<ISurvivorRescue *>(gamehelpers->ReferenceToEntity(param[1]));
	if(gRescue == nullptr)
	{
		return pContext->ThrowNativeError("Rescue Entity is not valid");
	}

	if(!gRescue->ClassMatches("info_survivor_rescue"))
	{
		return pContext->ThrowNativeError("Entity is not a \"CSurvivorRescue\", Entity Class Name: %s(%s)", gRescue->GetClassname().ToCStr(), gRescue->GetDebugName());
	}

	return gRescue->GetSurvivorHere();
}

FUNC_NATIVE(SurvivorLeftSafeArea)
{
	//return g_CallHelper->GetHasAnySurvivorLeftSafeArea();

	void *pDirector = g_HL2->GetDirector();
	return access_member<bool>(pDirector, 356);
}

FUNC_NATIVE(MobOnRash)
{
	g_CallHelper->OnMobRash();
	return 0;
}

FUNC_NATIVE(SpawnMobIT)
{
    int mobMin = g_pConVar->GetConVarInt("z_mob_spawn_min_size");
    int mobMax = g_pConVar->GetConVarInt("z_mob_spawn_max_size");
	cell_t sCount = param[1];

    if (sCount <= mobMin)
    {
        sCount = mobMin;
    }

    if (sCount >= mobMax)
    {
        sCount = mobMax;
    }

    g_CallHelper->OnSpawnITMob(sCount);
    return 0;
}

FUNC_NATIVE(OnSaveFromLedgHang)
{
	IBaseEntity *gPlayers = GetVirtualClass<IBaseEntity>(param[1]);
	if(!gPlayers) {
		return pContext->ThrowNativeError("Entity is not valid");
	}

	IMusic &m_music = access_member<IMusic>(gPlayers, 10160);
	m_music.OnSavedFromLedgeHang();
	return 1;
}

FUNC_NATIVE(MyStrStr)
{
	char *str, *substr;

	pContext->LocalToString(param[1], &str);
	pContext->LocalToString(param[2], &substr);

	const char *pos = g_Sample.my_strstr(str, substr);
	if(pos)
	{
		return (pos - str);
	}

	return -1;
}

FUNC_NATIVE(GetWorldSpaceCenter)
{
	auto sp_ftoc = [](float f) {
		return *(cell_t *)&f;
	};

	ITerrorPlayer *pEntity = GetVirtualClass<ITerrorPlayer>(param[1]);
	if(pEntity == nullptr)
	{
		return pContext->ThrowNativeError("Entity is not valid");
	}

	Vector vPos = pEntity->WorldSpaceCenter();

	cell_t *vec;
	pContext->LocalToPhysAddr(param[4], &vec);

	vec[0] = sp_ftoc(vPos.x);
	vec[1] = sp_ftoc(vPos.y);
	vec[2] = sp_ftoc(vPos.z);

	return 1;
}

FUNC_NATIVE(SetDefaultAbility)
{
	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}
		GiveDefaultAbility(gamehelpers->ReferenceToEntity(iClient));
	}

	return 1;
}

FUNC_NATIVE(GiveAbility)
{
	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}

		char *abilityName;
		char *weaponName;

		pContext->LocalToString(param[2], &abilityName);
		pContext->LocalToString(param[3], &weaponName);

		AddAbility(gamehelpers->ReferenceToEntity(iClient), abilityName, weaponName);
	}

	return 1;
}

NOINLINE int IsWeaponValid(IPluginContext *pContext, int client, CBaseEntity *pWeapon)
{
	IServerUnknown *pWeaponUnk = (IServerUnknown *)pWeapon;
	IServerNetworkable *pWeaponNet = pWeaponUnk->GetNetworkable();
	if(!g_HL2->UTIL_ContainsDataTable(pWeaponNet->GetServerClass()->m_pTable, "DT_BaseCombatWeapon"))
	{
		return pContext->ThrowNativeError("Entity index %d is not a weapon", gamehelpers->EntityToBCompatRef(pWeapon));
	}

	sm_sendprop_info_t spi;
	if(!gamehelpers->FindSendPropInfo("CBaseCombatWeapon", "m_hOwnerEntity", &spi))
	{
		return pContext->ThrowNativeError("Invalid entity index %d for weapon", gamehelpers->EntityToBCompatRef(pWeapon));
	}

	CBaseHandle &hndl = access_member<CBaseHandle>(pWeapon, spi.actual_offset);
	if(client != hndl.GetEntryIndex())
	{
		return pContext->ThrowNativeError("Entity index %d is not the owner of weapon", gamehelpers->EntityToBCompatRef(pWeapon));
	}
	return 1;
}

FUNC_NATIVE(WeaponDrops)
{
	ITerrorPlayer *pEntity = GetVirtualClass<ITerrorPlayer>(param[1]);
	if(!pEntity)
	{
		return pContext->ThrowNativeError("Entity is not valid");
	}

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(gamehelpers->ReferenceToIndex(param[1]));
	if(!pPlayer || !pPlayer->IsInGame())
	{
		return pContext->ThrowNativeError("Player is not valid");
	}

	CBaseEntity *pWeapon = gamehelpers->ReferenceToEntity(param[2]);
	if(!pWeapon)
	{
		return pContext->ThrowNativeError("Invalid entity index %d for weapon", param[2]);
	}

	if(!IsWeaponValid(pContext, param[1], pWeapon))
	{
		return 0;
	}

	Vector vForward;
	AngleVectors(pEntity->EyeAngles(), &vForward, nullptr, nullptr);
	Vector vTossPos = pEntity->WorldSpaceCenter();

	vTossPos = vTossPos + vForward * 64;

	pEntity->Weapon_Drop((CBaseCombatWeapon *)pWeapon, &vTossPos);
	return 1;
}

enum TypeDifficulty
{
	ERROR = 0,
	EASY,
	NORMAL,
	HARD,
	IMPOSSIBLE,
};

FUNC_NATIVE(GetGameDifficulty)
{
	ConVarRef difficulty("z_difficulty");
	//const char *sDifficulty = g_pConVar->GetConVarString("z_difficulty");

	if(g_Sample.my_strcmp(difficulty.GetString(), "Easy") == 0) {
		return TypeDifficulty::EASY;
	} else if(g_Sample.my_strcmp(difficulty.GetString(), "Normal") == 0) {
		return TypeDifficulty::NORMAL;
	} else if(g_Sample.my_strcmp(difficulty.GetString(), "Hard") == 0) {
		return TypeDifficulty::HARD;
	} else if(g_Sample.my_strcmp(difficulty.GetString(), "Impossible") == 0) {
		return TypeDifficulty::IMPOSSIBLE;
	}
	return TypeDifficulty::ERROR;
}

Vector GetBodyDirect3D(IBaseEntity* pClient)
{
	QAngle vecAng = pClient->GetAbsAngles();

	Vector vBodyDir;
	AngleVectors(vecAng, &vBodyDir);
	return vBodyDir;
}

Vector GetBodyDirect2D(IBaseEntity *pClient)
{
	Vector vecDir2D = GetBodyDirect3D(pClient);
	vecDir2D.z = 0;
	vecDir2D.AsVector2D().NormalizeInPlace();
	return vecDir2D;
}

Vector VecTossCheck(IBaseEntity *pEntity, ITraceFilter *pFilter, Vector vecSpot1, Vector vecSpot2, float flHeightMaxRatio, float flGravityAdj, bool bRandomize, Vector *vecMins, Vector *vecMaxs)
{
	trace_t			tr;
	Vector			vecMidPoint;
	Vector			vecApex;
	Vector			vecScale;
	Vector			vecTossVel;
	Vector			vecTemp;
	float			flGravity = g_pConVar->GetConVarFloat("sv_gravity") * flGravityAdj;

	if (vecSpot2.z - vecSpot1.z > 500)
	{
		return vec3_origin;
	}

	Vector forward, right;
	AngleVectors( pEntity->GetLocalAngles(), &forward, &right, NULL );

	if (bRandomize)
	{
		vecSpot2 += right * ( ::RandomFloat(-8,8) + ::RandomFloat(-16,16) );
		vecSpot2 += forward * ( ::RandomFloat(-8,8) + ::RandomFloat(-16,16) );
	}

	vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	util_TraceLine(vecMidPoint, vecMidPoint + Vector(0, 0, 300), MASK_SOLID_BRUSHONLY, pFilter, &tr);
	vecMidPoint = tr.endpos;

	if( tr.fraction != 1.0 )
	{
		vecMidPoint.z -= 15;
	}

	if (flHeightMaxRatio != -1)
	{
		float flHeightMax = flHeightMaxRatio * (vecSpot2 - vecSpot1).Length();
		float flHighestEndZ = MAX(vecSpot1.z, vecSpot2.z);
		if ((vecMidPoint.z - flHighestEndZ) > flHeightMax)
		{
			vecMidPoint.z = flHighestEndZ + flHeightMax;
		}
	}

	if (vecMidPoint.z < vecSpot1.z || vecMidPoint.z < vecSpot2.z)
	{
		return vec3_origin;
	}

	float distance1 = (vecMidPoint.z - vecSpot1.z);
	float distance2 = (vecMidPoint.z - vecSpot2.z);

	float time1 = sqrt( distance1 / (0.5 * flGravity) );
	float time2 = sqrt( distance2 / (0.5 * flGravity) );

	if (time1 < 0.1)
	{
		return vec3_origin;
	}

	vecTossVel = (vecSpot2 - vecSpot1) / (time1 + time2);
	vecTossVel.z = flGravity * time1;

	vecApex  = vecSpot1 + vecTossVel * time1;
	vecApex.z = vecMidPoint.z;

	util_TraceLine(vecSpot1, vecApex, (MASK_SOLID&(~CONTENTS_GRATE)), pFilter, &tr);
	if (tr.fraction != 1.0)
	{
		return vec3_origin;
	}

	util_TraceLine(vecSpot2, vecApex, (MASK_SOLID_BRUSHONLY & (~CONTENTS_GRATE)), pFilter, &tr); 
	if (tr.fraction != 1.0)
	{
		return vec3_origin;
	}

	if ( vecMins && vecMaxs )
	{
		util_TraceHull( vecSpot1, vecApex, *vecMins, *vecMaxs, (MASK_SOLID & (~CONTENTS_GRATE)), pFilter, &tr);		
		if ( tr.fraction < 1.0 )
			return vec3_origin;
	}

	return vecTossVel;

}

Vector VecCheckToss(edict_t *pEnt, Vector vecSpot1, Vector vecSpot2, float flHeightMaxRatio, float flGravityAdj, bool bRandomize, Vector *vecMins, Vector *vecMaxs)
{
	CTraceFilterSimples tr(pEnt->GetNetworkable()->GetEntityHandle(), COLLISION_GROUP_NONE);
	return VecTossCheck((IBaseEntity *)pEnt->GetNetworkable()->GetBaseEntity(), &tr, vecSpot1, vecSpot2, flHeightMaxRatio, flGravityAdj, bRandomize, vecMins, vecMaxs);
}

FUNC_NATIVE(ThroweWeapon)
{
	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}

		IBaseCombatWeapon *pWeapon = GetVirtualClass<IBaseCombatWeapon>(param[2]);
		if(!pWeapon)
		{
			return pContext->ThrowNativeError("Invalid entity index %d for weapon", param[2]);
		}

		ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(iClient);
		Vector vThroePos = pPlayer->EyePosition() - Vector(0, 0, 12);
		pWeapon->SetAbsOrigin(vThroePos);
		
		QAngle gunAngle;
		VectorAngles(GetBodyDirect2D(pPlayer), gunAngle);
		pWeapon->SetAbsAngles(gunAngle);

		Vector vForward;
		AngleVectors(pPlayer->EyeAngles(), &vForward, nullptr, nullptr);
		Vector vTossPos = pPlayer->WorldSpaceCenter();

		vTossPos = vTossPos + vForward * 64;
		Vector vecThrow = VecCheckToss(pPlayer->edict(), pWeapon->GetAbsOrigin(), vTossPos, 0.2, 1.0, false, nullptr, nullptr);
		if(vecThrow.IsZero())
		{
			vecThrow = GetBodyDirect3D(pPlayer) * 500.f;
		}

		pWeapon->Drop(vecThrow);
	}
	return 1;
}

FUNC_NATIVE(GiveItems)
{
	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}

		char *item_name;
		pContext->LocalToString(param[2], &item_name);

		ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(iClient);
		CBaseEntity *pEnt = pPlayer->GiveNamedItem(item_name, 0, false);

		return gamehelpers->EntityToBCompatRef(pEnt);
	}
	return -1;
}

FUNC_NATIVE(GiveAmmo)
{
	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}

		ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(iClient);
		IBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

		if(pWeapon != nullptr)
		{
			_CAmmoDef *ptr = nullptr;
			if((ptr = (_CAmmoDef *)g_CallHelper->GetAmmoDef()) == nullptr)
			{
				return pContext->ThrowNativeError("CAmmoDef is null");
			}

			if(pWeapon->UsesPrimaryAmmo())
			{
				int ammoIndex = pWeapon->GetPrimaryAmmoType();
				if(ammoIndex != -1)
				{
					int giveAmount = MaxCarry(ptr, ammoIndex);
					Ammo_t *szName = GetAmmoOfIndex(ptr, ammoIndex);
					pPlayer->GiveAmmo(giveAmount, szName->pName);
				}
			}

			if(pWeapon->UsesSecondaryAmmo() && pWeapon->HasSecondaryAmmo())
			{
				int ammoIndex = pWeapon->GetSecondaryAmmoType();
				if(ammoIndex != -1)
				{
					int giveAmount = MaxCarry(ptr, ammoIndex);
					Ammo_t *szName = GetAmmoOfIndex(ptr, ammoIndex);
					pPlayer->GiveAmmo(giveAmount, szName->pName);
				}
			}
		}
	}
	return 1;
}

FUNC_NATIVE(DamageRadius)
{
	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}
	}

	auto ctof = [](cell_t f) {
		return *(float *)&f;
	};

	CBaseEntity *pAttacker = nullptr;
	g_HL2->IndexToAThings(iClient, &pAttacker, nullptr);
	Vector vecOrigin;
	cell_t *vec;

	pContext->LocalToPhysAddr(param[2], &vec);

	vecOrigin.x = ctof(vec[0]);
	vecOrigin.y = ctof(vec[1]);
	vecOrigin.z = ctof(vec[2]);

	g_HL2->DamageRadius(pAttacker, vecOrigin, ctof(param[4]), ctof(param[3]));
	return 1;
}

#include "Interface/IDynamicLight.h"
FUNC_NATIVE(MakeLightDynamic)
{
	int client = param[2];
	IGamePlayer *pClient = playerhelpers->GetGamePlayer(client);
	IPlayerInfo *pPlayer = nullptr;
	if(pClient && (pPlayer = pClient->GetPlayerInfo()) != nullptr)
	{
		if(pClient->IsConnected() && pClient->IsInGame())
		{
			IDynamicLight *pEntity = (IDynamicLight *)servertools->CreateEntityByName("light_dynamic");
			if(pEntity == nullptr)
			{
				return pContext->ThrowNativeError("Create entity \"light_dynamic\" is failed");
			}

			auto ctof = [](cell_t f) {
				return *(float *)&f;
			};

			pEntity->SetRenderColor(255, 255, 255);
			pEntity->SetExponent(1);
			pEntity->SetSpotRadius(32.f);
			pEntity->SetRadius(ctof(param[1]));
			pEntity->SetLightStyle(0);

			servertools->DispatchSpawn(pEntity);

			pEntity->Set_Flags(pEntity->GetActualFlags());
			pEntity->SetOn(true);

			variant_t var;
			var.SetString(castable_string_t("!activator"));
			pEntity->AcceptInput("SetParent", gamehelpers->ReferenceToEntity(client), nullptr, var, 0);

			if(pPlayer->GetTeamIndex() == 2 && !pPlayer->IsDead())
			{
				IBaseEntity *m_pParent = pEntity->GetParent();
				if(m_pParent)
				{
					IBaseAnimating* pAnim = nullptr;
					if((pAnim = (IBaseAnimating *)m_pParent->GetBaseAnimating()) != nullptr)
					{
						int iAttachment = pAnim->LookupAttachment("grenade");
						if(iAttachment)
						{
							pEntity->m_iParentAttachment = iAttachment;
							pEntity->SetParent((CBaseEntity *)m_pParent, iAttachment);
							pEntity->SetMoveType(MOVETYPE_NONE);
						}
					}
				}
			}

			cell_t *vec;
			pContext->LocalToPhysAddr(param[3], &vec);
			Vector vecOrigin(ctof(vec[0]), ctof(vec[1]), ctof(vec[2]));
			pEntity->SetLocalOrigin(vecOrigin);

			vec = nullptr;
			pContext->LocalToPhysAddr(param[4], &vec);
			QAngle vecAngle(ctof(vec[0]), ctof(vec[1]), ctof(vec[2]));
			pEntity->SetLocalAngles(vecAngle);

			auto EHandle = pEntity->GetRefEHandle();
			if(EHandle.GetEntryIndex() >= MAX_EDICTS)
				return (EHandle.ToInt() | (1<<31));
			else
				return EHandle.GetEntryIndex(); 
		}
	}
	
	return pContext->ThrowNativeError("Client (%d) is not valid\n", client);
}

FUNC_NATIVE(PropDynamicFlashLight)
{
	int client = param[2];
	IGamePlayer *pClient = playerhelpers->GetGamePlayer(client);
	IPlayerInfo *pPlayer = nullptr;
	if(pClient && (pPlayer = pClient->GetPlayerInfo()) != nullptr)
	{
		if(pClient->IsConnected() && pClient->IsInGame())
		{
			IDynamicProp *pEntity = (IDynamicProp *)servertools->CreateEntityByName("prop_dynamic");
			if(pEntity == nullptr)
			{
				return pContext->ThrowNativeError("Create entity \"prop_dynamic\" is failed");
			}
			char *szModelName;
			pContext->LocalToString(param[1], &szModelName);
			pEntity->SetModel(szModelName);
			servertools->DispatchSpawn(pEntity);

			variant_t var;
			var.SetString(castable_string_t("!activator"));
			pEntity->AcceptInput("SetParent", gamehelpers->ReferenceToEntity(client), nullptr, var, 0);

			if(pPlayer->GetTeamIndex() == 2 && !pPlayer->IsDead())
			{
				IBaseEntity *m_pParent = nullptr;
				if((m_pParent = pEntity->GetParent()) != nullptr)
				{					
					IBaseAnimating* pAnim = nullptr;
					if((pAnim = (IBaseAnimating *)m_pParent->GetBaseAnimating()) != nullptr)
					{
						int iAttachment = pAnim->LookupAttachment("grenade");
						if(iAttachment)
						{
							pEntity->m_iParentAttachment = iAttachment;
							pEntity->SetParent((CBaseEntity *)m_pParent, iAttachment);
							pEntity->SetMoveType(MOVETYPE_NONE);
						}
					}
				}
			}

			auto ctof = [](cell_t f) {
				return *(float *)&f;
			};
			cell_t *vec;
			pContext->LocalToPhysAddr(param[3], &vec);
			pEntity->SetLocalOrigin(Vector(ctof(vec[0]), ctof(vec[1]), ctof(vec[2])));

			vec = nullptr;
			pContext->LocalToPhysAddr(param[4], &vec);
			pEntity->SetLocalAngles(QAngle(ctof(vec[0]), ctof(vec[1]), ctof(vec[2])));

			auto EHandle = pEntity->GetRefEHandle();
			if(EHandle.GetEntryIndex() >= MAX_EDICTS)
				return (EHandle.ToInt() | (1<<31));
			else
				return EHandle.GetEntryIndex(); 
		}
	}
	return pContext->ThrowNativeError("Client (%d) is not valid\n", client);
}

FUNC_NATIVE(GetRandomPlayers)
{
	int iPlayerCount[MAX_PLAYERS + 1];
	int iCount = -1;
	for(int i(1); i <= g_pGlobals->maxClients; i++)
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(i);
		if(pClient && pClient->IsConnected() && pClient->IsInGame())
		{
			IPlayerInfo *pPlayer = pClient->GetPlayerInfo();
			if(pPlayer && pPlayer->GetTeamIndex() != 1)
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

HandleType_t g_iEntSphereHandle = 0;
class CEntitySphereIsHandle : public IHandleTypeDispatch, public CAppSystem
{
public:
	CEntitySphereIsHandle() : CAppSystem() {}

	void OnLoad() override
	{
		HandleError err;
		TypeAccess EntSphereAccess;
		handlesys->InitAccessDefaults(&EntSphereAccess, NULL);
		EntSphereAccess.ident = myself->GetIdentity();
		EntSphereAccess.access[HTypeAccess_Create] = true;
		EntSphereAccess.access[HTypeAccess_Inherit] = true;

		g_iEntSphereHandle = handlesys->CreateType("EntityInSphere", this, 0, &EntSphereAccess, NULL, myself->GetIdentity(), &err);
		if(!g_iEntSphereHandle)
		{
			m_sLog->LogToFileEx(false, "Could not create Entity in Sphere handle type (err: %d)", err);
		}
	}

	void OnUnload() override
	{
		handlesys->RemoveType(g_iEntSphereHandle, myself->GetIdentity());
	}

	void OnHandleDestroy(HandleType_t type, void *object) override
	{
		if(type == g_iEntSphereHandle)
		{
			CEntitySphereQuery_ *v = (CEntitySphereQuery_*)object;
			delete v;
		}
	}

	bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize) override
	{
		if(type == g_iEntSphereHandle)
		{
			*pSize = sizeof(CEntitySphereQuery_);
		}
		return true;
	}	
};
CEntitySphereIsHandle *g_pEntSphereGlobal = new CEntitySphereIsHandle;

FUNC_NATIVE(EntityInSphere)
{
	auto ctof = [](cell_t f) {
		return *(float *)&f;
	};

	cell_t *vec;
	pContext->LocalToPhysAddr(param[1], &vec);
	Vector vecCenter(ctof(vec[0]), ctof(vec[1]), ctof(vec[2]));
	CEntitySphereQuery_ *mSphere = new CEntitySphereQuery_(vecCenter, ctof(param[2]), param[3]);
	
	HandleError hErr;
	Handle_t hndl = handlesys->CreateHandle(g_iEntSphereHandle, mSphere, pContext->GetIdentity(), myself->GetIdentity(), &hErr);
	if(!hndl)
	{
		delete mSphere;
		return pContext->ThrowNativeError("Unable to Create a new Entity Sphere handle (Error %d)", hErr);
	}
	return hndl;
}

FUNC_NATIVE(GetEntitySphere)
{
	HandleOpen<CEntitySphereQuery_> pSphere(pContext, param[1], g_iEntSphereHandle);

	if(pSphere.OK())
	{
		return gamehelpers->EntityToBCompatRef(pSphere->GetCurrentEntity());
	}
	return -1;
}

FUNC_NATIVE(NextEntitySphere)
{
	HandleOpen<CEntitySphereQuery_> pSphere(pContext, param[1], g_iEntSphereHandle);
	if(pSphere.OK())
	{
		pSphere->NextEntity();
	}
	return 1;
}

#ifdef IS_VOTE_ENABLE
#include "Interface/IVoteBase.h"

class CVote : public IBaseTerrorIssue
{
public:
	CVote(const char* szTypeName, const char* szDisplayString, const char* szPassedString) : IBaseTerrorIssue(szTypeName)
	{
		_strcpy_safe(m_szDisplayString, szDisplayString);
		_strcpy_safe(m_szPassedString, szPassedString);

		func_CanCallVote = nullptr;
		func_ExecuteCommand = nullptr;
		func_OnVoteFailed = nullptr;
		func_OnVotePassed = nullptr;
		func_OnVoteStarted = nullptr;
	}

	virtual void ExecuteCommand(void) override
	{
		if(func_ExecuteCommand)
		{
			func_ExecuteCommand->PushCell(hndl);
			func_ExecuteCommand->Execute(NULL);
		}
	}

	virtual bool CanCallVote(int iEntIndex, const char *pszCommand, const char *pszDetails) override
	{
		if(!IBaseTerrorIssue::CanCallVote(iEntIndex, pszCommand, pszDetails))
		{
			return false;
		}

		if(!IsEnable())
		{
			V_strncpy((char *)pszDetails, "This survey is disabled.", 0x60);
			return false;
		}

		if(func_CanCallVote)
		{
			cell_t cellResult = 0;
			func_CanCallVote->PushCell(hndl);
			func_CanCallVote->PushCell(iEntIndex);
			func_CanCallVote->PushString(pszCommand);
			func_CanCallVote->PushStringEx((char *)pszDetails, 96, SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
			func_CanCallVote->PushCell(96);
			func_CanCallVote->Execute(&cellResult);
			
			return static_cast<bool>(cellResult);
		}

		return true;
	}
	virtual const char* GetDisplayString(void) override
	{
	    return m_szDisplayString;
	}
	virtual void ListIssueDetails( IBasePlayer *pForWhom ) override
	{
		if(!IsEnable())
			return;

		ListStandardNoArgCommand(pForWhom, GetTypeString());
	}
	virtual const char* GetVotePassedString(void) override
	{
		return m_szPassedString;
	}
    virtual void OnVotePassed(void) override
	{
		IBaseIssue::OnVotePassed();

		if(func_OnVotePassed)
		{
			func_OnVotePassed->PushCell(hndl);
			func_OnVotePassed->Execute(NULL);
		}
	}
	virtual void OnVoteFailed( void ) override
	{
		IBaseIssue::OnVoteFailed();

		if(func_OnVoteFailed)
		{
			func_OnVoteFailed->PushCell(hndl);
			func_OnVoteFailed->Execute(NULL);
		}
	}
	virtual void OnVoteStarted( void ) override
	{
		IBaseIssue::OnVoteStarted();

		if(func_OnVoteStarted)
		{
			func_OnVoteStarted->PushCell(hndl);
			func_OnVoteStarted->Execute(NULL);
		}
	}
	bool IsEnable(void)
	{
		return ukr_issue_vote_menu.GetBool();
	}
private:
	char m_szDisplayString[64];
	char m_szPassedString[64];

public:
	IPluginFunction *func_ExecuteCommand;
	IPluginFunction *func_CanCallVote;
	IPluginFunction *func_OnVotePassed;
	IPluginFunction *func_OnVoteFailed;
	IPluginFunction *func_OnVoteStarted;
	Handle_t hndl;
};

HandleType_t g_iVoteHandle = 0;
bool OnRemoveVote(CVote *pVote)
{
	void *pVoteController = g_HL2->GetVoteController();
	if(pVoteController)
	{
		CUtlVector<IBaseIssue *> &m_potentialIssues = access_member<CUtlVector<IBaseIssue *>>(pVoteController, 287 * 4);
		int nIndex = m_potentialIssues.Find(pVote);
		if(nIndex != -1)
		{
			delete pVote;
			m_potentialIssues.Remove(nIndex);
			return true;
		}
	}

	return false;
}

class CVoteHandle :	public IHandleTypeDispatch,	public CAppSystem
{
public:
	CVoteHandle() : CAppSystem() { }

	void OnLoad() override
	{
		HandleError err;
		TypeAccess VoteAccess;
		handlesys->InitAccessDefaults(&VoteAccess, NULL);
		VoteAccess.ident = myself->GetIdentity();
		VoteAccess.access[HTypeAccess_Create] = true;
		VoteAccess.access[HTypeAccess_Inherit] = true;

		g_iVoteHandle = handlesys->CreateType("VoteHandles", this, 0, &VoteAccess, NULL, myself->GetIdentity(), &err);
		if(!g_iVoteHandle)
		{
			m_sLog->LogToFileEx(false, "Could not create Vote handle type (err: %d)", err);
		}
	}

	void OnUnload() override
	{
		handlesys->RemoveType(g_iVoteHandle, myself->GetIdentity());
	}

	void OnHandleDestroy(HandleType_t type, void *object) override
	{
		if(type == g_iVoteHandle)
		{
			OnRemoveVote((CVote *)object);
		}
	}

	bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize) override
	{
		if(type == g_iVoteHandle)
		{
			*pSize = sizeof(CVote);
		}
		return true;
	}
};
CVoteHandle *g_pVoteHandle = new CVoteHandle;

FUNC_NATIVE(CreateVote)
{
	char *szType, *pszDispley, *pszPassed;
	pContext->LocalToString(param[1], &szType);
	pContext->LocalToString(param[2], &pszDispley);
	pContext->LocalToString(param[3], &pszPassed);

	CVote *pVote = new CVote(szType, pszDispley, pszPassed);
	HandleError hErr;
	pVote->hndl = handlesys->CreateHandle(g_iVoteHandle, pVote, pContext->GetIdentity(), myself->GetIdentity(), &hErr);
	if(!pVote->hndl)
	{
		OnRemoveVote(pVote);
		return pContext->ThrowNativeError("Unable to Create a new Vote handle (Error %d)", hErr);
	}
	return pVote->hndl;
}

FUNC_NATIVE(SetFuncCallBack)
{
	HandleOpen<CVote> pVote(pContext, param[1], g_iVoteHandle);

	if(pVote.OK())
	{
		if(param[2])
			pVote->func_CanCallVote = pContext->GetFunctionById((funcid_t)param[2]);
		if(param[3])
			pVote->func_ExecuteCommand = pContext->GetFunctionById((funcid_t)param[3]);
		if(param[4])
			pVote->func_OnVoteFailed = pContext->GetFunctionById((funcid_t)param[4]);
		if(param[5])
			pVote->func_OnVotePassed = pContext->GetFunctionById((funcid_t)param[5]);
		if(param[6])
			pVote->func_OnVoteStarted = pContext->GetFunctionById((funcid_t)param[6]);
	}
	return 1;
}

FUNC_NATIVE(SetCanCallVote)
{
	HandleOpen<CVote> pVote(pContext, param[1], g_iVoteHandle);
	if(pVote.OK())
	{
		pVote->func_CanCallVote = pContext->GetFunctionById((funcid_t)param[2]);
	}
	return 1;
}

FUNC_NATIVE(SetExecuteComand)
{
	HandleOpen<CVote> pVote(pContext, param[1], g_iVoteHandle);
	if(pVote.OK())
	{
		pVote->func_ExecuteCommand = pContext->GetFunctionById((funcid_t)param[2]);
	}
	return 1;
}

FUNC_NATIVE(SetOnVotePassed)
{
	HandleOpen<CVote> pVote(pContext, param[1], g_iVoteHandle);
	if(pVote.OK())
	{
		pVote->func_OnVotePassed = pContext->GetFunctionById((funcid_t)param[2]);
	}
	return 1;
}

FUNC_NATIVE(SetOnVoteFailed)
{
	HandleOpen<CVote> pVote(pContext, param[1], g_iVoteHandle);
	if(pVote.OK())
	{
		pVote->func_OnVoteFailed = pContext->GetFunctionById((funcid_t)param[2]);
	}
	return 1;
}

FUNC_NATIVE(SetOnVoteStarted)
{
	HandleOpen<CVote> pVote(pContext, param[1], g_iVoteHandle);
	if(pVote.OK())
	{
		pVote->func_OnVoteStarted = pContext->GetFunctionById((funcid_t)param[2]);
	}
	return 1;
}

#endif

FUNC_NATIVE(FollowMeEntity)
{
	auto PrecacheModel = [](const char *szName, bool isPreload = true)
	{
		if(!szName)
		{
			return 0;
		}
		return engine->PrecacheModel(szName, isPreload);
	};

	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}
	}

	char *szModel = nullptr;
	pContext->LocalToString(param[2], &szModel);

	if(!engine->IsModelPrecached(szModel))
	{
		if(!PrecacheModel(szModel))
		{
			m_sLog->LogToFileEx(false, "Model is not precache");
			return 0;
		}
	}

	char *szBoneName = nullptr;
	pContext->LocalToString(param[3], &szBoneName);

	if(!szBoneName)
	{
		m_sLog->LogToFileEx(false, "Bone name is null");
		return 0;
	}

	IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(iClient);
	int iBoneId = pPlayer->LookupBone(szBoneName);
	if(iBoneId == -1)
	{
		m_sLog->LogToFileEx(false, "Bone index is -1");
		return 0;
	}

	CHandle<IDynamicProp> m_hEntity = (IDynamicProp*)servertools->CreateEntityByName("prop_dynamic");
	if(m_hEntity)
	{
		m_hEntity->SetModel(szModel);
		Vector pos; QAngle ang;
		pPlayer->GetBonePosition(iBoneId, pos, ang);
		m_hEntity->SetAbsOrigin(pos);
		m_hEntity->SetAbsAngles(ang);
		m_hEntity->FollowEntity(pPlayer);

		if(m_hEntity.GetEntryIndex() >= MAX_EDICTS)
			return (m_hEntity.ToInt() | (1<<31));
		else
			return m_hEntity.GetEntryIndex(); 
	}
	return pContext->ThrowNativeError("Error create entity by name");
}

FUNC_NATIVE(FollowEntityToMe)
{
	auto PrecacheModel = [](const char *szName, bool isPreload = true)
	{
		if(!szName)
		{
			return 0;
		}
		return engine->PrecacheModel(szName, isPreload);
	};

	int iClient = param[1];
	if((iClient >= 1) && (iClient <= playerhelpers->GetMaxClients()))
	{
		IGamePlayer *pClient = playerhelpers->GetGamePlayer(iClient);
		if(!pClient)
		{
			return pContext->ThrowNativeError("Client index %d is not valid", iClient);
		}
		if(!pClient->IsConnected() && !pClient->IsInGame())
		{
			return pContext->ThrowNativeError("Client is not connected or is not in game");
		}
		if(pClient->GetPlayerInfo()->IsDead())
		{
			return pContext->ThrowNativeError("Client is dead");
		}
	}

	char *szModel = nullptr;
	pContext->LocalToString(param[2], &szModel);

	if(!engine->IsModelPrecached(szModel))
	{
		if(!PrecacheModel(szModel))
		{
			m_sLog->LogToFileEx(false, "Model is not precache");
			return 0;
		}
	}

	char *szBoneName = nullptr;
	pContext->LocalToString(param[3], &szBoneName);

	if(!szBoneName)
	{
		m_sLog->LogToFileEx(false, "Attachment is null");
		return 0;
	}

	IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(iClient);
	int iBoneId = pPlayer->LookupAttachment(szBoneName);
	if(iBoneId == -1)
	{
		m_sLog->LogToFileEx(false, "Attachment index is -1");
		return 0;
	}

	CHandle<IDynamicProp> m_hEntity = (IDynamicProp*)servertools->CreateEntityByName("prop_dynamic");
	if(m_hEntity)
	{
		m_hEntity->SetModel(szModel);
		Vector pos; QAngle ang;
		pPlayer->GetAttachment(iBoneId, pos, ang);
		m_hEntity->SetAbsOrigin(pos);
		m_hEntity->SetAbsAngles(ang);
		m_hEntity->FollowEntity(pPlayer);

		if(m_hEntity.GetEntryIndex() >= MAX_EDICTS)
			return (m_hEntity.ToInt() | (1<<31));
		else
			return m_hEntity.GetEntryIndex(); 
	}
	return pContext->ThrowNativeError("Error create entity by name");
}

FUNC_NATIVE(OnAdminMenuReadi)
{
	Handle_t* hTopMenuType = nullptr;
    IExtension* pOwner = nullptr;

    Dl_info info;
    if(dladdr(g_pTopMenu, &info) != 0)
    {
        void *hndl = dlopen(info.dli_fname, RTLD_NOW);
        if(hndl)
        {
            hTopMenuType = (Handle_t*)memutils->ResolveSymbol(hndl, "hTopMenuType");
            pOwner =  *(IExtension **)memutils->ResolveSymbol(hndl, "myself");
            dlclose(hndl);
        }
    }

	ITopMenu *topMenu = nullptr;
    if(hTopMenuType && pOwner)
    {
        if(param[1] > 0 )
        {
            HandleError err;
            HandleSecurity sec(myself->GetIdentity(), pOwner->GetIdentity());
            if((err = handlesys->ReadHandle(param[1], *hTopMenuType, &sec, (void**)&topMenu)) != HandleError_None)
            {
                m_sLog->LogToFileEx(false, "Invalid Handle %x (error: %d)", param[1], err);
                topMenu = nullptr;
            }
        }
    }

	extern void OnAdminMenuAdd(ITopMenu *pTopMenu);
	OnAdminMenuAdd(topMenu);
	return 1;
}

#include "Interface/INavMesh.h"
#include "CTempEntity.h"

enum MobLocationType
{
	SPAWN_NO_PREFERENCE = -1,
	SPAWN_ANYWHERE,
	SPAWN_BEHIND_SURVIVORS,
	SPAWN_NEAR_IT_VICTIM,
	SPAWN_SPECIALS_IN_FRONT_OF_SURVIVORS,
	SPAWN_SPECIALS_ANYWHERE,
	SPAWN_FAR_AWAY_FROM_SURVIVORS,
	SPAWN_ABOVE_SURVIVORS,
	SPAWN_IN_FRONT_OF_SURVIVORS,
	SPAWN_VERSUS_FINALE_DISTANCE,
	SPAWN_LARGE_VOLUME,
	SPAWN_NEAR_POSITION,
};

FUNC_NATIVE(CreatePlayerBot)
{
	int iClient = param[1];
	if(iClient <= 0 && iClient > g_pGlobals->maxClients)
	{
		return pContext->ThrowNativeError("Client index is nullptr");
	}

	IBasePlayer *me = (IBasePlayer*)gamehelpers->ReferenceToEntity(iClient);
	if(me == nullptr)
		return pContext->ThrowNativeError("IBasePlayer is nullptr");

	Vector forvard;
	me->EyeVectors(&forvard);

	trace_t res;
	util_TraceLine(
		me->EyePosition(), 
		me->EyePosition() + 999999.9f * forvard, 
		(MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE), 
		me->GetNetworkable()->GetEntityHandle(), 
		COLLISION_GROUP_NONE, 
		&res);
	
	if(res.DidHit())
	{
		IBaseEntity *pEntity = nullptr;
		QAngle randAng = QAngle(0.f, RandomFloat(0.f, 360.f), 0.f);

		INavMesh *pMesh = g_HL2->GetTheNavMesh();
		if(!pMesh)
		{
			return pContext->ThrowNativeError("TheNavMesh is nullptr");
		}
		INavArea* pNavArea = pMesh->GetNearestNavArea(res.endpos, false, 1000.f);

		switch (param[2])
		{
		case ZombieClassCommon:
			if(pNavArea)
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnCommon(pNavArea, randAng);
			else
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnCommon(res.endpos + Vector(0, 0, 15.f), randAng);
			break;

		case ZombieClassSmoker:
			if(pNavArea)
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnSmoker(pNavArea, randAng);
			else
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnSmoker(res.endpos + Vector(0, 0, 15.f), randAng);
			break;

		case ZombieClassBommer:
			if(pNavArea)
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnBoomer(pNavArea, randAng);
			else
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnBoomer(res.endpos + Vector(0, 0, 15.f), randAng);
			break;

		case ZombieClassHunter:
			if(pNavArea)
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnHunter(pNavArea, randAng);
			else
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnHunter(res.endpos + Vector(0, 0, 15.f), randAng);
			break;

		case ZombieClassWitch:
			if(pNavArea)
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnWitch(pNavArea, randAng);
			else
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnWitch(res.endpos + Vector(0, 0, 15.f), randAng);
			break;

		case ZombieClassTank:
			if(pNavArea)
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnTank(pNavArea, randAng);
			else
				pEntity = (IBaseEntity *)g_pBotCreator->SpawnTank(res.endpos + Vector(0, 0, 15.f), randAng);
			break;
		
		default:
			return 0;
		}

		return pEntity->entindex();
	}
	return 0;
}

#include "IBaseBans.h"

FUNC_NATIVE(AddBan)
{
	int client = param[1];
	if(client <= 0 && client > g_pGlobals->maxClients)
	{
		return pContext->ThrowNativeError("Client index is nullptr");
	}

	int adminId = param[2];
	if(adminId < 0 && adminId > g_pGlobals->maxClients)
	{
		return pContext->ThrowNativeError("Admin index is nullptr");
	}

	int time = param[3];
	if(time < 0)
	{
		return pContext->ThrowNativeError("Time is nullptr");
	}

	char *szReason = nullptr;
	pContext->LocalToString(param[4], &szReason);
	if(!szReason)
	{
		return pContext->ThrowNativeError("Reason is nullptr");
	}

	m_sLog->LogToFileEx(false, "[native_AddBan] param: <%d><%d><%d><%s>", client, adminId, time, szReason);
	if(g_pBaseBans)
	{
		return g_pBaseBans->AddBan(client, adminId, time, szReason) ? 1 : 0;
	}

	return pContext->ThrowNativeError("g_pBaseBans is nullptr");
}

FUNC_NATIVE(AddBanId)
{
	char *szId = nullptr;
	pContext->LocalToString(param[1], &szId);
	if(!szId)
	{
		return pContext->ThrowNativeError("ID is nullptr");
	}

	int adminId = param[2];
	if(adminId < 0 && adminId > g_pGlobals->maxClients)
	{
		return pContext->ThrowNativeError("Admin index is nullptr");
	}

	int time = param[3];
	if(time < 0)
	{
		return pContext->ThrowNativeError("Time is nullptr");
	}

	char *szReason = nullptr;
	pContext->LocalToString(param[4], &szReason);
	if(!szReason)
	{
		return pContext->ThrowNativeError("Reason is nullptr");
	}

	m_sLog->LogToFileEx(false, "[native_AddBanId] param: <%s><%d><%d><%s>", szId, adminId, time, szReason);
	if(g_pBaseBans)
	{
		return g_pBaseBans->AddBan(szId, adminId, time, szReason) ? 1 : 0;
	}
	return pContext->ThrowNativeError("g_pBaseBans is nullptr");
}

FUNC_NATIVE(UnBan)
{
	char *szId = nullptr;
	pContext->LocalToString(param[1], &szId);
	if(!szId)
	{
		return pContext->ThrowNativeError("ID is nullptr");
	}

	int adminId = param[2];
	if(adminId < 0 && adminId > g_pGlobals->maxClients)
	{
		return pContext->ThrowNativeError("Admin index is nullptr");
	}

	char *szReason = nullptr;
	pContext->LocalToString(param[3], &szReason);
	if(!szReason)
	{
		return pContext->ThrowNativeError("Reason is nullptr");
	}

	m_sLog->LogToFileEx(false, "[native_AddBan] param: <%s><%d><%s>", szId, adminId, szReason);
	if(g_pBaseBans)
	{
		return g_pBaseBans->UnBan(szId, adminId, szReason) ? 1 : 0;
	}
	return pContext->ThrowNativeError("g_pBaseBans is nullptr");
}

extern bool Disolved(CBaseEntity* pThisPtr, const char* pMaterialName, bool bNPCOnly, int nDissolveType, Vector vDissolveOrigin, int nMagnitude);
FUNC_NATIVE(Disolve)
{
	if(param[1] <= 0 || param[1] > g_pGlobals->maxEntities)
	{
		return pContext->ThrowNativeError("Entity index is not valid %d", param[1]);
	}

	IBaseEntity *pEntity = GetVirtualClass<IBaseEntity>(param[1]);
	IBaseAnimating* pAnim = (IBaseAnimating*)pEntity->GetBaseAnimating();

	if(!pAnim) return pContext->ThrowNativeError("Entity %s does not have a CBaseAnimating class", pEntity->GetClassname().ToCStr());

	char *szMaterialName = nullptr;
	pContext->LocalToString(param[2], &szMaterialName);

	int iDisolveType = param[3];
	if(iDisolveType < 0 || iDisolveType >= 3)
		iDisolveType = ENTITY_DISSOLVE_NORMAL;

	auto ctof = [](cell_t f) {
		return *(float *)&f;
	};
	cell_t *vec;
	pContext->LocalToPhysAddr(param[4], &vec);
	Vector vecOrigion(ctof(vec[0]), ctof(vec[1]), ctof(vec[2]));

	return Disolved(pEntity->GetBaseEntity(), szMaterialName, false, iDisolveType, vecOrigion, param[5]) == true ? 1 : 0;
}

BEGIN_NATIVES(UkrCoop)

	ADD_NATIVE(LogMessegToFile)
	ADD_NATIVE(BotCreater)
	ADD_NATIVE(Respawn)
	ADD_NATIVE(l4d_staggered)
	ADD_NATIVE(TakeOverBots)
	ADD_NATIVE(SetHumSpec)
	ADD_NATIVE(l4dTakeOverBot)
	ADD_NATIVE(Revive)
	ADD_NATIVE(RegenExtra)
	ADD_NATIVE(PVomitUpon)
	ADD_NATIVE(IsRescueLiveSurvInside)
	ADD_NATIVE(IsRescueBehinClosesDoor)
	ADD_NATIVE(RescueCloseDoors)
	ADD_NATIVE(OnSaveFromLedgHang)
	ADD_NATIVE(RescueSpawn)
	ADD_NATIVE(IsRescueOpen)
	ADD_NATIVE(MyStrStr)
	ADD_NATIVE(SurvivorLeftSafeArea)
	ADD_NATIVE(MobOnRash)
	ADD_NATIVE(SpawnMobIT)
	ADD_NATIVE(GetGameDifficulty)
	ADD_NATIVE(GetWorldSpaceCenter)
	ADD_NATIVE(WeaponDrops)
	ADD_NATIVE(OnTakeHealth)
	ADD_NATIVE(GiveAbility)
	ADD_NATIVE(SetDefaultAbility)
	ADD_NATIVE(ThroweWeapon)
	ADD_NATIVE(GiveItems)
	ADD_NATIVE(GiveAmmo)
	ADD_NATIVE(DamageRadius)
	ADD_NATIVE(MakeLightDynamic)
	ADD_NATIVE(PropDynamicFlashLight)
	ADD_NATIVE(GetRandomPlayers)
	ADD_NATIVE(FollowMeEntity)
	ADD_NATIVE(FollowEntityToMe)
	ADD_NATIVE(CreatePlayerBot)

	ADD_NATIVE(EntityInSphere)
	ADD_NATIVE(GetEntitySphere)
	ADD_NATIVE(NextEntitySphere)

	ADD_NATIVE(OnAdminMenuReadi)

	ADD_NATIVE(AddBan)
	ADD_NATIVE(AddBanId)
	ADD_NATIVE(UnBan)

	ADD_NEW_SINTASIS_NATIVE(Sphere.Sphere,	EntityInSphere)
	ADD_NEW_SINTASIS_NATIVE(Sphere.GetEntity, GetEntitySphere)
	ADD_NEW_SINTASIS_NATIVE(Sphere.Next, NextEntitySphere)

#ifdef IS_VOTE_ENABLE
	ADD_NATIVE(CreateVote)
	ADD_NATIVE(SetFuncCallBack)

	ADD_NEW_SINTASIS_NATIVE(Votes.Votes, CreateVote)
	ADD_NEW_SINTASIS_NATIVE(Votes.OnCanCallVote.set, SetCanCallVote)
	ADD_NEW_SINTASIS_NATIVE(Votes.OnExecuteCommand.set, SetExecuteComand)
	ADD_NEW_SINTASIS_NATIVE(Votes.OnVotePassed.set, SetOnVotePassed)
	ADD_NEW_SINTASIS_NATIVE(Votes.OnVoteFailed.set, SetOnVoteFailed)
	ADD_NEW_SINTASIS_NATIVE(Votes.OnVoteStarted.set, SetOnVoteStarted)
#endif

	ADD_NATIVE(Disolve)
END_NATIVES()

