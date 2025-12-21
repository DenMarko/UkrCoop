// декомпилированный код для підрахунку живих вцілілих гравців, 
// доданий для використання разом з плаігіном selfheal для не завершення раунду, 
// якщо у вцілілих гравців є таблетки.

#include "extension.h"
#include "detours.h"
#include "Interface/ITerrorPlayer.h"

class CLiveSurvivorCount
{
public:

    CLiveSurvivorCount(unsigned char flags) : m_flags(flags), m_count(0) {}

    bool operator() (ITerrorPlayer* pPlayer)
    {
        if(!pPlayer->IsAlive()) return true;
        if(pPlayer->GetTeamNumber() != 2) return true;

        if((m_flags & 4) == 0 || !pPlayer->IsBot())
        {
            if(!pPlayer->IsIncapacitated() || (m_flags & 1) == 0)
            {
                if(pPlayer->IsIncapacitated() || (m_flags & 2) == 0)
                {
                    ++m_count;
                    return true;
                }
                return true;
            }

            if(pPlayer->IsIncapacitated() || (m_flags & 2) == 0)
            {
                if(pPlayer->Weapon_GetSlot(4))
                {
                    ++m_count;
                    return true;
                }

                if(access_member<bool>(pPlayer, 0x2711))
                {
                    if((access_member<unsigned char>(pPlayer, 0x2B45) & 0x08) != 0)
                    {
                        ++m_count;
                        return true;
                    }
                }
                else
                {
                    if((access_member<unsigned char>(pPlayer, 0x2B45) & 0x10) != 0)
                    {
                        ++m_count;
                        return true;
                    }
                }
            }
        }
        return true;
    }

    unsigned char m_flags;
    int m_count;
};

DETOUR_DECL_STATIC1(CSurvivorCount, bool, CLiveSurvivorCount&, info)
{
    return ForEachTerrorPlayer(info);
}

class MyLiveSurvivorCountHook : public CAppSystem
{
public:
    MyLiveSurvivorCountHook() : CAppSystem()
    {
        m_pHook = nullptr;
    }
    virtual ~MyLiveSurvivorCountHook() {}

    virtual void OnLoad()
    {
        m_pHook = DETOUR_CREATE_STATIC(CSurvivorCount, "LiveSurvivorCount");
        if(m_pHook)
        {
            m_pHook->EnableDetour();
        }
    }

    virtual void OnUnload()
    {
        if(m_pHook)
        {
            m_pHook->Destroy();
            m_pHook = nullptr;
        }
    }

private:
    CDetour *m_pHook;
};

MyLiveSurvivorCountHook *gpLiveSurvivorHook = new MyLiveSurvivorCountHook();