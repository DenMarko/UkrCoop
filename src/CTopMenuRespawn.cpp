#include "CTopMenuRespawn.h"
#include "CAdminMenu.h"
#include "Interface/ITerrorPlayer.h"
#include "Interface/INavMesh.h"
#include "HL2.h"

bool ClientRespawn(int client, int target)
{
    IGamePlayer *pTarget = playerhelpers->GetGamePlayer(target);
    if(!pTarget || !pTarget->IsConnected() || !pTarget->IsInGame() || !pTarget->GetPlayerInfo()->IsDead())
    {
        return false;
    }

    ITerrorPlayer *pBasePlayer = GetVirtualClass<ITerrorPlayer>(client);
    if(pBasePlayer)
    {
        Vector forvard;
        pBasePlayer->EyeVectors(&forvard);
        trace_t tr;
        util_TraceLine(
            pBasePlayer->EyePosition(), 
            pBasePlayer->EyePosition() + 999999.9f * forvard, 
            MASK_L4D_VISIBLE, 
            pBasePlayer->GetNetworkable()->GetEntityHandle(), 
            COLLISION_GROUP_NONE, 
            &tr);
        if(tr.DidHit())
        {
            QAngle angRand = QAngle(0.f, RandomFloat(0.f, 360.f), 0.f);
            Vector vecSpot1 = tr.endpos;
            INavMesh *pMesh = g_HL2->GetTheNavMesh();
            if(pMesh)
            {
                INavArea* pNavArea = pMesh->GetNearestNavArea(tr.endpos, false, 1000.f);
                if(pNavArea)
                {
                    vecSpot1 = pNavArea->FindRandomSpot();
                }
            }

            auto pTarget = GetVirtualClass<ITerrorPlayer>(target);
            pTarget->RoundRespawn();
            pTarget->Teleport(&vecSpot1, &angRand, nullptr);
            return true;
        }
    }
    return false;
}

void CreateMenuRespawn(int client, ITopMenu* menu)
{
    auto AddFlags = [](IBaseMenu* pMenu, unsigned int flags)
    {
        unsigned int old_flags = pMenu->GetMenuOptionFlags();
        old_flags |= flags;
        pMenu->SetMenuOptionFlags(old_flags);
    };

    IMenuStyle* pStyle = menus->GetDefaultStyle();

    IMenuHandler *pCallBack = new CMenuRespawn(menu);
    IBaseMenu *pMenu = pStyle->CreateMenu(pCallBack, myself->GetIdentity());
    if(!pMenu)
    {
        delete pCallBack;
        return;
    }

    char szTitle[64];
    g_HL2->Translate(szTitle, sizeof(szTitle), "%T", 2, nullptr, "SelectPlayer", &client);
    pMenu->SetDefaultTitle(szTitle);

    // for(int i = 1; i <= g_pGlobals->maxClients; i++)
    // {
    //     IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(i);
    //     if( !pPlayer )
    //     {
    //         continue;
    //     }

    //     if( !pPlayer->IsConnected() && !pPlayer->IsInGame())
    //     {
    //         continue;
    //     }

    //     auto pPlayerInfo = pPlayer->GetPlayerInfo();
    //     if( !pPlayerInfo || !pPlayerInfo->IsDead() )
    //     {
    //         continue;
    //     }

    //     char szName[MAX_PLAYER_NAME_LENGTH + 1];
    //     char user_id[16];

    //     g_Sample.UTIL_Format(szName, sizeof(szName), "%s (%d)", pPlayer->GetName(), i);
    //     g_Sample.UTIL_Format(user_id, sizeof(user_id), "%d", i);

    //     pMenu->AppendItem(user_id, ItemDrawInfo(szName));
    // }

    CMenuAddItemPlayer AddItems(
        pMenu, 
        client, 
        PlayerAliveType::Dead, 
        PlayerBotType::IgnoreBot, 
        PlayerIncapType::IgnoreIncap);

    g_Sample.ForEachPlayers(AddItems);

    AddFlags(pMenu, MENUFLAG_BUTTON_EXITBACK);
    pMenu->Display(client, MENU_TIME_FOREVER);
}

void CMenuRespawn::OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason)
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

void CMenuRespawn::OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
{
    HandleSecurity sec;
    sec.pIdentity = nullptr;
    sec.pOwner = myself->GetIdentity();

    handlesys->FreeHandle(menu->GetHandle(), &sec);
}
void CMenuRespawn::OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    ItemDrawInfo dr;
    const char* info = menu->GetItemInfo(item, &dr);
    int target = atoi(info);

    if(target > 0 && target <= g_pGlobals->maxClients)
    {
        auto pTarget = playerhelpers->GetGamePlayer(target);
        if(pTarget)
        {
            if(pTarget->IsConnected() && pTarget->IsInGame())
            {
                if( pTarget->GetPlayerInfo()->IsDead() )
                    ClientRespawn(client, target);
                else
                    g_HL2->TextMsg(client, CHAT, "\x01[\x05SM\x01] Player is not dead!");
            }
            else
            {
                g_HL2->TextMsg(client, CHAT, "\x01[\x05SM\x01] Player not connected!");
            }
        }
        else
        {
            g_HL2->TextMsg(client, CHAT, "\x01[\x05SM\x01] Player not found!");
        }
    }
    else
    {
        g_HL2->TextMsg(client, CHAT, "\x01[\x05SM\x01] Invalid player id!");
    }

    auto pPlayer = playerhelpers->GetGamePlayer(client);
    if(pPlayer && (pPlayer->IsConnected() && pPlayer->IsInGame()) && !pPlayer->IsInKickQueue() && m_pTopMenu)
    {
        CreateMenuRespawn(client, m_pTopMenu);
    }

    playerhelpers->SetReplyTo(old_reply);
}

void TopMenuRespawn::OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength)
{
    g_HL2->Translate(buffer, maxlength, "%T", 2, nullptr, "Respawn", &client);
}

void TopMenuRespawn::OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id)
{
    unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);

    CreateMenuRespawn(client, menu);

    playerhelpers->SetReplyTo(old_reply);
}