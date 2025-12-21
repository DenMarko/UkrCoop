#ifndef _HEADER_CADMINMENU_INCLUDE_
#define _HEADER_CADMINMENU_INCLUDE_

#include "extension.h"

enum PlayerAliveType {
    IgnoreAlive = 0,
    Alive,
    Dead
};

enum PlayerIncapType {
    IgnoreIncap = 0,
    Incap,
    NotIncap
};

enum PlayerBotType {
    IgnoreBot = 0,
    Bot,
    NotBot
};

class CMenuAddItemPlayer
{
public:
    CMenuAddItemPlayer(IBaseMenu* pMenu, int client, PlayerAliveType Alive, PlayerBotType Bot, PlayerIncapType Incap, bool ignore_me = false) : 
    m_pMenu(pMenu), 
    m_client(client),
    m_ignore_me(ignore_me),
    m_alive(Alive), 
    m_incap(Incap), 
    m_bot(Bot) 
    { }

    bool operator ()(int client);
private:
    IBaseMenu* m_pMenu;
    int m_client;
    bool m_ignore_me;
    PlayerAliveType m_alive;
    PlayerIncapType m_incap;
    PlayerBotType m_bot;
};



#endif