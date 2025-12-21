#include "CCmdRunHook.h"
#include "HL2.h"
#include "usercmd.h"
#include "LuaBridge/LuaBridge.h"

CCmdRunHook g_CmdRunHook;
CCmdRunHook *g_pCmdRunHook = &g_CmdRunHook;
SH_DECL_MANUALHOOK2_void(RunCmdHook, 0, 0, 0, CUserCmd *, IMoveHelper *);
luabridge::LuaRef *RunCmd = nullptr;

CCmdRunHook::CCmdRunHook()
{
}

void CCmdRunHook::Initialize()
{
    int offset = 428;
    __SourceHook_FHM_ReconfigureRunCmdHook(offset, 0, 0);
    RunCmd = new luabridge::LuaRef(luabridge::getGlobal(g_Sample.GetLuaState(), "OnPlayerRunCmd"));
}

void CCmdRunHook::Shutdown()
{
    delete RunCmd;
}

void CCmdRunHook::OnClientPutInServer(edict_t *pEdict, const char *pName)
{
	IServerUnknown *pUnknown = pEdict->GetUnknown();
	if (!pUnknown)
	{
		return;
	}

	CBaseEntity *pEntity = pUnknown->GetBaseEntity();
	if (!pEntity)
	{
		return;
	}

	CVTableHook hook(pEntity);
	for (size_t i = 0; i < m_RunCmdHook.length(); ++i)
	{
		if (hook == m_RunCmdHook[i])
		{
			return;
		}
	}

	int hookid = SH_ADD_MANUALVPHOOK(RunCmdHook, pEntity, SH_MEMBER(this, &CCmdRunHook::PlayerCmdRun), false);
	hook.SetHookID(hookid);
	m_RunCmdHook.append(new CVTableHook(hook));
}

inline edict_t *BaseEntityToEdict(CBaseEntity *pEntity)
{
	IServerUnknown *pUnk = (IServerUnknown *)pEntity;
	IServerNetworkable *pNet = pUnk->GetNetworkable();

	if (!pNet)
	{
		return NULL;
	}

	return pNet->GetEdict();
}

void CCmdRunHook::PlayerCmdRun(CUserCmd *ucmd, IMoveHelper *moveHelper)
{
	CBaseEntity *pEntity = reinterpret_cast<CBaseEntity*>(g_SHPtr->GetIfacePtr());
	if (!pEntity)
	{
		RETURN_META(MRES_IGNORED);
	}

	edict_t *pEdict = BaseEntityToEdict(pEntity);
	if (!pEdict)
	{
		RETURN_META(MRES_IGNORED);
	}

	int client = g_Sample.IndexOfEdict(pEdict);
    luabridge::LuaRef RefRes(g_Sample.GetLuaState());

    Vector vec_velocity(ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);
    Vector mouse(ucmd->mousedx, ucmd->mousedy, 0);

    if(RunCmd->isFunction())
    {
        RefRes = (*RunCmd)(client, ucmd->buttons, ucmd->impulse, vec_velocity, ucmd->viewangles, ucmd->weaponselect, ucmd->weaponsubtype, ucmd->command_number, ucmd->tick_count, ucmd->random_seed, mouse);
    }

	if (RefRes.isNumber() && (int)RefRes == 3)
	{
        ucmd->forwardmove = vec_velocity.x;
        ucmd->sidemove = vec_velocity.y;
        ucmd->upmove = vec_velocity.z;

        ucmd->mousedx = mouse.x;
        ucmd->mousedy = mouse.y;

		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}
