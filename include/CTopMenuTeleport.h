#ifndef _HEADER_CTOPMENU_TELEPORT_INCLUDE_
#define _HEADER_CTOPMENU_TELEPORT_INCLUDE_

#include "extension.h"

bool ClientTeleport(int client, int target);

class CMenuTeleport : public IMenuHandler
{
public:
    CMenuTeleport(ITopMenu* pTopMenu) : m_pTopMenu(pTopMenu) {}

    void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason) override;
    void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason) override;
    void OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page) override;
private:
    ITopMenu *m_pTopMenu;
};

class TopMenuTeleport : public ITopMenuObjectCallbacks
{
public:
    TopMenuTeleport() {}

    unsigned int OnTopMenuDrawOption(ITopMenu *menu, int client, unsigned int object_id) override { return ITEMDRAW_DEFAULT; }
    void OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override;
    void OnTopMenuDisplayTitle(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override { }
    void OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id) override;
    void OnTopMenuObjectRemoved(ITopMenu *menu, unsigned int object_id) override { delete this; }
};


#endif // _HEADER_CTOPMENU_TELEPORT_INCLUDE_