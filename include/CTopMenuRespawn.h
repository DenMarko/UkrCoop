#ifndef _HEADER_RESPAWNMENU_INCLUDE_
#define _HEADER_RESPAWNMENU_INCLUDE_

#include "extension.h"

bool ClientRespawn(int client, int target);

class CMenuRespawn : public IMenuHandler
{
public:
    CMenuRespawn(ITopMenu* pTopMenu) : m_pTopMenu(pTopMenu) {}

    void OnMenuCancel(IBaseMenu *menu, int client, MenuCancelReason reason) override;
    void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason) override;
    void OnMenuSelect2(IBaseMenu *menu, int client, unsigned int item, unsigned int item_on_page) override ;
private:
    ITopMenu *m_pTopMenu;
};

class TopMenuRespawn : public ITopMenuObjectCallbacks
{
public:
    TopMenuRespawn() {}

    void OnTopMenuDisplayOption(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override;
    void OnTopMenuDisplayTitle(ITopMenu *menu, int client, unsigned int object_id, char buffer[], size_t maxlength) override { }
    void OnTopMenuSelectOption(ITopMenu *menu, int client, unsigned int object_id) override;
    void OnTopMenuObjectRemoved(ITopMenu *menu, unsigned int object_id) override { delete this; }
};


#endif