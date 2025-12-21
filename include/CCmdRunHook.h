#ifndef _INCLUDE_CMD_RUN_HOOK_PROPER_H_
#define _INCLUDE_CMD_RUN_HOOK_PROPER_H_
class CUserCmd;

#include "vtable_hook_helper.h"
#include "amtl/am-vector.h"

class CCmdRunHook
{
public:
    CCmdRunHook();

    void Initialize();
    void Shutdown();
    void OnClientPutInServer(edict_t *pEdict, const char *pName);
    void PlayerCmdRun(CUserCmd *ucmd, IMoveHelper *moveHelper);
private:
    ke::Vector<CVTableHook *> m_RunCmdHook;
};

extern CCmdRunHook *g_pCmdRunHook;

#endif