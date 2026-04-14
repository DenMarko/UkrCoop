#include "CTopMenuBan.h"
#include "HL2.h"
#include "IBaseBans.h"
#include "Interface/ITerrorPlayer.h"
#include "CAdminMenu.h"

void TopMenuBan::OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength)
{
    g_HL2->Translate(buffer, maxlength, "%T", 2, nullptr, "ban_menu", &client);
}

void TopMenuBan::OnTopMenuDisplayTitle(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength)
{
    g_HL2->Translate(buffer, maxlength, "%T:", 2, nullptr, "ban_menu", &client);
}

void TopMenuAddBan::OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength)
{
    g_HL2->Translate(buffer, maxlength, "%T", 2, nullptr, "ban_add_ban", &client);
}

void CreateMenuAddBan(ITopMenu *menu, int client)
{
    IMenuStyle* pStyle = menus->GetDefaultStyle();

    IMenuHandler *pCallBack = new CMenuAddBan(menu);
    IBaseMenu *pMenu = pStyle->CreateMenu(pCallBack, myself->GetIdentity());
    if(!pMenu)
    {
        delete pCallBack;
        return;
    }

    auto AddFlags = [&pMenu](unsigned int flags)
    {
        unsigned int old_flags = pMenu->GetMenuOptionFlags();
        old_flags |= flags;
        pMenu->SetMenuOptionFlags(old_flags);
    };

    char szTitle[64];
    g_HL2->Translate(szTitle, sizeof(szTitle), "%T", 2, nullptr, "SelectPlayer", &client);
    pMenu->SetDefaultTitle(szTitle);

    CMenuAddItemPlayer AddItems(
        pMenu, 
        client, 
        IgnoreAlive, 
        NotBot, 
        IgnoreIncap, 
        true);

    g_Sample.ForEachPlayers(AddItems);

    AddFlags(MENUFLAG_BUTTON_EXITBACK);
    pMenu->Display(client, MENU_TIME_FOREVER);
}

void TopMenuAddBan::OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);
    CreateMenuAddBan(menu, client);
    playerhelpers->SetReplyTo(old_reply);
}

void CMenuAddBan::OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    switch (reason)
    {
    case MenuCancel_ExitBack:
        if(m_pTopMenu)
        {
            m_pTopMenu->DisplayMenu(client, MENU_TIME_FOREVER, TopMenuPosition_LastCategory);
        }
        break;
    }

    playerhelpers->SetReplyTo(old_reply);
}

void CMenuAddBan::OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
{
    HandleSecurity sec;
    sec.pIdentity = nullptr;
    sec.pOwner = myself->GetIdentity();

    handlesys->FreeHandle(menu->GetHandle(), &sec);
}

void CMenuAddBan::OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    ItemDrawInfo dr;
    const char* info = menu->GetItemInfo(item, &dr);
    int target = atoi(info);

    if(target > 0 && target <= g_pGlobals->maxClients)
    {
        IMenuStyle *pStyle = menus->GetDefaultStyle();
        IMenuHandler *pHandler = new CMenuTimeBans(m_pTopMenu, target);
        IBaseMenu *pMenu = pStyle->CreateMenu(pHandler, myself->GetIdentity());
        if(!pMenu)
        {
            delete pHandler;
            playerhelpers->SetReplyTo(old_reply);
            return;
        }

        auto AddFlags = [&pMenu](unsigned int flags)
        {
            unsigned int old_flags = pMenu->GetMenuOptionFlags();
            old_flags |= flags;
            pMenu->SetMenuOptionFlags(old_flags);
        };

        pMenu->AppendItem("0", ItemDrawInfo("Permanent"));
        pMenu->AppendItem("10", ItemDrawInfo("10 minutes"));
        pMenu->AppendItem("30", ItemDrawInfo("30 Minutes"));
        pMenu->AppendItem("60", ItemDrawInfo("1 Hour"));
        pMenu->AppendItem("240", ItemDrawInfo("4 Hour"));
        pMenu->AppendItem("1440", ItemDrawInfo("1 Day"));
        pMenu->AppendItem("10080", ItemDrawInfo("1 Week"));

        AddFlags(MENUFLAG_BUTTON_EXITBACK);
        pMenu->Display(client, MENU_TIME_FOREVER);
    }

    playerhelpers->SetReplyTo(old_reply);
}

void CMenuTimeBans::OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);
    
    switch(reason)
    {
        case MenuCancel_ExitBack:
        {
            if(m_pTopMenu)
            {
                m_pTopMenu->DisplayMenu(client, MENU_TIME_FOREVER, TopMenuPosition_LastCategory);
            }
            break;
        }
    }
    
    playerhelpers->SetReplyTo(old_reply);
}

void CMenuTimeBans::OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
{
    HandleSecurity sec;
    sec.pIdentity = nullptr;
    sec.pOwner = myself->GetIdentity();

    handlesys->FreeHandle(menu->GetHandle(), &sec);
}

void CMenuTimeBans::OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    ItemDrawInfo dr;
    const char* info = menu->GetItemInfo(item, &dr);
    int time = atoi(info);

    if(time >= 0)
    {
        IMenuStyle *pStyle = menus->GetDefaultStyle();
        IMenuHandler *pHandler = new CMenuReasonBans(m_pTopMenu, m_target, time);
        IBaseMenu *pMenu = pStyle->CreateMenu(pHandler, myself->GetIdentity());
        if(!pMenu)
        {
            delete pHandler;
            playerhelpers->SetReplyTo(old_reply);
            return;
        }

        auto AddFlags = [&pMenu](unsigned int flags)
        {
            unsigned int old_flags = pMenu->GetMenuOptionFlags();
            old_flags |= flags;
            pMenu->SetMenuOptionFlags(old_flags);
        };

        ReasonList *begin = g_ListReason;
        while (begin)
        {
            pMenu->AppendItem(begin->info, ItemDrawInfo(begin->display));
            begin = begin->m_next;
        }

        AddFlags(MENUFLAG_BUTTON_EXITBACK);
        pMenu->Display(client, MENU_TIME_FOREVER);
    }

    playerhelpers->SetReplyTo(old_reply);
}

void CMenuReasonBans::OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    switch(reason)
    {
        case MenuCancel_ExitBack:
        {
            if(m_pTopMenu)
            {
                m_pTopMenu->DisplayMenu(client, MENU_TIME_FOREVER, TopMenuPosition_LastCategory);
            }
            break;
        }
    }

    playerhelpers->SetReplyTo(old_reply);
}

void CMenuReasonBans::OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
{
    HandleSecurity sec;
    sec.pIdentity = nullptr;
    sec.pOwner = myself->GetIdentity();

    handlesys->FreeHandle(menu->GetHandle(), &sec);
}

void CMenuReasonBans::OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    ItemDrawInfo dr;
    const char *szReason = menu->GetItemInfo(item, &dr);

    if(g_pBaseBans)
    {
        if(g_pBaseBans->AddBan(m_target, client, m_time, szReason))
        {
            g_HL2->TextMsg(client, DEST::CHAT, "Client is banned");
        }
    }

    auto pPlayer = playerhelpers->GetGamePlayer(client);
    if(pPlayer && (pPlayer->IsConnected() && pPlayer->IsInGame()) && !pPlayer->IsInKickQueue() && m_pTopMenu)
    {
        CreateMenuAddBan(m_pTopMenu, client);
    }

    playerhelpers->SetReplyTo(old_reply);
}

template<typename Func>
void ForEachActiveBans(Func &func)
{
    IQuery* pQuery = nullptr;
    if((pQuery = g_pBaseBans->GetActiveBans()) != nullptr)
    {
        IResultSet* pResult = nullptr;
        if((pResult = g_pBaseBans->GetResultSet(pQuery)) != nullptr)
        {
            bool bLoop = true;
            while(bLoop)
            {
                SBanInfo* pInfo = new SBanInfo();
                if(g_pBaseBans->GetBanInfo(pResult, pInfo)) {
                    if(!func(pInfo)) {
                        bLoop = false;
                    }
                } else {
                    bLoop = false;
                }
                delete pInfo;
            }
        }
        pQuery->Destroy();
    }
}

class CAddItemActiveBans
{
public:
    CAddItemActiveBans(IBaseMenu* menu) : m_pMenu(menu) {}

    bool operator() (SBanInfo* info)
    {
        if(!info) {
            return true;
        }

        m_pMenu->AppendItem(info->authId, ItemDrawInfo(info->szName[0] == '\0' ? info->authId : info->szName));
        return true;
    }
private:
    IBaseMenu* m_pMenu;
};

void CreateMenuUnBan(ITopMenu* pTopMenu, int client)
{
    IMenuStyle* pStyle = menus->GetDefaultStyle();
    IMenuHandler *pMenuHandler = new CMenuUnBan(pTopMenu);
    IBaseMenu *pMenu = pStyle->CreateMenu(pMenuHandler, myself->GetIdentity());
    if(!pMenu)
    {
        delete pMenuHandler;
        return;
    }

    auto AddFlags = [&pMenu](unsigned int flags)
    {
        unsigned int old_flags = pMenu->GetMenuOptionFlags();
        old_flags |= flags;
        pMenu->SetMenuOptionFlags(old_flags);
    };

    char szTitle[64];
    g_HL2->Translate(szTitle, sizeof(szTitle), "%T", 2, nullptr, "SelectPlayer", &client);
    pMenu->SetDefaultTitle(szTitle);

    CAddItemActiveBans addItems(pMenu);
    ForEachActiveBans(addItems);

    AddFlags(MENUFLAG_BUTTON_EXITBACK);
    pMenu->Display(client, MENU_TIME_FOREVER);
}

void TopMenuUnBan::OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength)
{
    g_HL2->Translate(buffer, maxlength, "%T", 2, nullptr, "ban_un_ban", &client);
}

void TopMenuUnBan::OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);
    CreateMenuUnBan(menu, client);
    playerhelpers->SetReplyTo(old_reply);
}

void CMenuUnBan::OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    switch(reason)
    {
        case MenuCancel_ExitBack:
        {
            if(m_pTopMenu)
            {
                m_pTopMenu->DisplayMenu(client, MENU_TIME_FOREVER, TopMenuPosition_LastCategory);
            }
            break;
        }
    }

    playerhelpers->SetReplyTo(old_reply);
}

void CMenuUnBan::OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
{
    HandleSecurity sec;
    sec.pIdentity = nullptr;
    sec.pOwner = myself->GetIdentity();

    handlesys->FreeHandle(menu->GetHandle(), &sec);
}

void CMenuUnBan::OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    ItemDrawInfo dr;
    const char *authID = menu->GetItemInfo(item, &dr);

    if(g_pBaseBans)
    {
        if(g_pBaseBans->UnBan(authID, client, "UnBan is admin menu"))
        {
            g_HL2->TextMsg(client, CHAT, "Player is unbanned!");
        }
    }

    auto pPlayer = playerhelpers->GetGamePlayer(client);
    if(pPlayer && (pPlayer->IsConnected() && pPlayer->IsInGame()) && !pPlayer->IsInKickQueue() && m_pTopMenu)
    {
        CreateMenuUnBan(m_pTopMenu, client);
    }

    playerhelpers->SetReplyTo(old_reply);
}
