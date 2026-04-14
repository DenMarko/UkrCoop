#ifndef _HEADER_CTOPMENUBAN_INCLUDE_
#define _HEADER_CTOPMENUBAN_INCLUDE_
#include "extension.h"

class TopMenuBan : public ITopMenuObjectCallbacks
{
public:
    TopMenuBan() {}
    
    unsigned int OnTopMenuDrawOption(ITopMenu *menu, int client, unsigned int object_id) override { return ITEMDRAW_DEFAULT; }
    void OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override;
    void OnTopMenuDisplayTitle(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override;
    void OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id) override { }
    void OnTopMenuObjectRemoved(ITopMenu *menu, unsigned int object_id) override { delete this; }
};

class TopMenuAddBan : public ITopMenuObjectCallbacks
{
public:
    TopMenuAddBan() {}
    
    unsigned int OnTopMenuDrawOption(ITopMenu *menu, int client, unsigned int object_id) override { return ITEMDRAW_DEFAULT; }
    void OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override;
    void OnTopMenuDisplayTitle(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override { }
    void OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id) override;
    void OnTopMenuObjectRemoved(ITopMenu *menu, unsigned int object_id) override { delete this; }
};

class TopMenuUnBan : public ITopMenuObjectCallbacks
{
public:
    TopMenuUnBan() {}
    
    unsigned int OnTopMenuDrawOption(ITopMenu *menu, int client, unsigned int object_id) override { return ITEMDRAW_DEFAULT; }
    void OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override;
    void OnTopMenuDisplayTitle(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override { }
    void OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id) override;
    void OnTopMenuObjectRemoved(ITopMenu *menu, unsigned int object_id) override { delete this; }
};

class CMenuAddBan : public IMenuHandler
{
public:
    CMenuAddBan(ITopMenu* pTopMenu) : m_pTopMenu(pTopMenu) {}

    void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason) override;
    void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason) override;
    void OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page) override;
private:
    ITopMenu *m_pTopMenu;
};

class CMenuUnBan : public IMenuHandler
{
public:
    CMenuUnBan(ITopMenu* pTopMenu) : m_pTopMenu(pTopMenu) {}

    void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason) override;
    void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason) override;
    void OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page) override;
private:
    ITopMenu *m_pTopMenu;
};

class CMenuTimeBans : public IMenuHandler
{
public:
    CMenuTimeBans(ITopMenu* pTopMenu, int target) : m_pTopMenu(pTopMenu), m_target(target) {}

    void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason) override;
    void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason) override;
    void OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page) override;
private:
    ITopMenu *m_pTopMenu;
    int m_target;
};

class CMenuReasonBans : public IMenuHandler
{
public:
    CMenuReasonBans(ITopMenu* pTopMenu, int target, int time) : m_pTopMenu(pTopMenu), m_target(target), m_time(time) {}

    void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason) override;
    void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason) override;
    void OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page) override;
private:
    ITopMenu *m_pTopMenu;
    int m_target;
    int m_time;
};
#endif // _HEADER_CTOPMENUBAN_INCLUDE_