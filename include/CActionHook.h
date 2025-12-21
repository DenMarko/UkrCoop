#ifndef _HEADER_ACTION_HOOK_INCLUDE_
#define _HEADER_ACTION_HOOK_INCLUDE_

#include "CActionImpl.h"
#include "CActionHandle.h"
#include "amtl/am-hashset.h"

class CActionManager
{
private:
	using ActionContainer = ke::HashSet<INextBotAction_ptr, ke::PointerPolicy<INextBotAction>>;
	using ActionList = ke::HashMap<INextBotAction_ptr, CUtlVector<IHandleAction*>*, ke::PointerPolicy<INextBotAction>>;

public:
	CActionManager();
	~CActionManager();

	bool IsValidAction(INextBotAction_ptr const pAction);
	CUtlVector<IHandleAction*> *GetCallBackList(INextBotAction_ptr const pAction, bool post_list);

	bool AddHook(INextBotAction_ptr const pAction, IHandleAction *pCallBack, bool bPost = false);
	void RemoveHook(INextBotAction_ptr const pAction);

private:
	void Add(INextBotAction_ptr const pAction);
	void Remove(INextBotAction_ptr const pAction);

protected:
	virtual void OnActionCreate(INextBotAction_ptr pAction);
	virtual void OnAcdionDestroy(INextBotAction_ptr pAction);

private:
	ActionContainer m_action;
	ActionList m_PreActionList;
	ActionList m_PostActionList;
};

inline bool CActionManager::IsValidAction(INextBotAction_ptr const pAction)
{
	if(!pAction)
    	return false;

	if(!m_action.find(pAction).found())
		return false;

	return true;
}

extern CActionManager *g_pActionManager;

#endif