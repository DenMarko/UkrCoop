#include "CAdminMenu.h"
#include "CTopMenuRespawn.h"
#include "CTopMenuTeleport.h"
#include "CTopMenuBan.h"

#include "Interface/ITerrorPlayer.h"
#include "Interface/INavMesh.h"
#include "log_messege.h"
#include "HL2.h"

void OnAdminMenuAdd(ITopMenu *pTopMenu)
{
    if(!pTopMenu)
    {
        m_sLog->LogToFileEx(false, "CAdminMenu::OnAdminMenuAdd: pTopMenu is NULL");
        return;
    }

    ITopMenuObjectCallbacks* pCallBackBanCategory = new TopMenuBan();
    unsigned int ban_category = pTopMenu->AddToMenu2(
        "BanMenu", 
        TopMenuObject_Category, 
        pCallBackBanCategory, 
        myself->GetIdentity(), 
        "BanMenu", 
        0, 
        0, 
        "");
    if(ban_category != 0)
    {
        ITopMenuObjectCallbacks *pCallBackAddBan = new TopMenuAddBan();
        if(!pTopMenu->AddToMenu2(
            "AddBan", 
            TopMenuObject_Item, 
            pCallBackAddBan, 
            myself->GetIdentity(), 
            "AddBan", 
            ADMFLAG_BAN, 
            ban_category, 
            ""
        ))
        {
            delete pCallBackAddBan;
        }

        ITopMenuObjectCallbacks* pUnBan = new TopMenuUnBan();
        if(!pTopMenu->AddToMenu2(
            "UnBan",
            TopMenuObject_Item,
            pUnBan,
            myself->GetIdentity(),
            "UnBan",
            ADMFLAG_BAN,
            ban_category,
            ""
        ))
        {
            delete pUnBan;
        }
    }

    unsigned int player_command_category = pTopMenu->FindCategory("PlayerCommands");
    if(player_command_category != 0)
    {
        ITopMenuObjectCallbacks *pCallBackTeleport = new TopMenuTeleport();
        if(!pTopMenu->AddToMenu2(
            "teleport", 
            TopMenuObject_Item, 
            pCallBackTeleport, 
            myself->GetIdentity(), 
            "teleport", 
            ADMFLAG_CHEATS, 
            player_command_category, 
            "Teleport player to position"
        ))
        { 
            delete pCallBackTeleport;
        }
        
        ITopMenuObjectCallbacks *pCallBackRespawn = new TopMenuRespawn();
        if(!pTopMenu->AddToMenu2(
            "respawn", 
            TopMenuObject_Item, 
            pCallBackRespawn, 
            myself->GetIdentity(), 
            "respawn", 
            ADMFLAG_CHEATS, 
            player_command_category, 
            "Respawn player and teleport to cursor position"
        ))
        { 
            delete pCallBackRespawn;
        }
    }
}

class CAdminMenu : public CAppSystem
{
public:
    CAdminMenu() : CAppSystem() {}

    ResultType OnClientCommand(int client, const CCommand &args) override
    {
        IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
        if(!pPlayer ||
           !pPlayer->IsConnected() || 
           !pPlayer->IsInGame())
        {
            return Pl_Continue;
        }

        if(g_Sample.my_bStrcmp(args[0], "teleport"))
        {
            if(IsAdminAccess(pPlayer->GetAdminId(), ADMFLAG_CHEATS))
            {
                if(args.ArgC() >= 2)
                {
                    int target = atoi(args[1]);
                    if(target > 0 || target <= g_pGlobals->maxClients)
                    {
                        ClientTeleport(client, target);
                    }
                    else
                        g_HL2->TextMsg(client, CONSOLE, "[SM] To use the command type \"teleport\" or \"teleport <player_id>\"");
                }
                else
                {
                    ClientTeleport(client, client);
                }
            }

            return Pl_Handled;
        }
        else if(g_Sample.my_bStrcmp(args[0], "respawn"))
        {
            if(IsAdminAccess(pPlayer->GetAdminId(), ADMFLAG_CHEATS))
            {
                if(args.ArgC() >= 2)
                {
                    int target = atoi(args[1]);
                    if(target > 0 || target <= g_pGlobals->maxClients)
                    {
                        ClientRespawn(client, target);
                    }
                    else
                        g_HL2->TextMsg(client, CONSOLE, "[SM] To use the command type \"respawn\" or \"respawn <player_id>\"");
                }
                else
                {
                    ClientRespawn(client, client);
                }
            }

            return Pl_Handled;
        }

        return Pl_Continue;
    }
};

CAdminMenu *g_CBaseBans = new CAdminMenu();

bool CMenuAddItemPlayer::operator()(int client)
{
    if(m_ignore_me)
    {
        if(m_client == client)
        {
            return true;
        }
    }

    IGamePlayer *pPlayer = nullptr;
    if((pPlayer = playerhelpers->GetGamePlayer(client)) == nullptr)
    {
        return true;
    }

    if(!pPlayer->IsConnected() && !pPlayer->IsInGame())
    {
        return true;
    }

    if(m_bot)
    {
        if(m_bot == Bot ? !pPlayer->IsFakeClient() : pPlayer->IsFakeClient())
        {
            return true;
        }
    }

    if(m_alive)
    {
        IPlayerInfo* pPlayerInfo = nullptr;
        if((pPlayerInfo = pPlayer->GetPlayerInfo()) != nullptr)
        {
            if(m_alive == Alive ? pPlayerInfo->IsDead() : !pPlayerInfo->IsDead())
            {
                return true;
            }
        }
    }
    
    if(m_incap)
    {
        ITerrorPlayer* pBPlayer = nullptr;
        if((pBPlayer = GetVirtualClass<ITerrorPlayer>(client)) != nullptr)
        {
            if(m_incap == Incap ? !pBPlayer->IsIncapacitated() : pBPlayer->IsIncapacitated())
            {
                return true;
            }
        }
    }

    char szName[MAX_PLAYER_NAME_LENGTH + 1];
    char user_id[16];

    g_Sample.UTIL_Format(szName, sizeof(szName), "%s (%d)", pPlayer->GetName(), client);
    g_Sample.UTIL_Format(user_id, sizeof(user_id), "%d", client);

    m_pMenu->AppendItem(user_id, ItemDrawInfo(szName));
    
    return true;
}

