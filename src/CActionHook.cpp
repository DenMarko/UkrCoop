#include "CActionHook.h"

CActionManager m_ActionManager;
CActionManager *g_pActionManager = &m_ActionManager;

CActionManager::CActionManager()
{
	m_action.init();
	m_PreActionList.init();
	m_PostActionList.init();
}

CActionManager::~CActionManager()
{
	{
		auto iter = m_PostActionList.iter();
		while(!iter.empty())
		{
			for(int i = 0; i < iter->value->Count(); i++)
				delete iter->value->Element(i);

			iter.next();
		}
		m_PostActionList.clear();
	}
	{
		auto iter = m_PreActionList.iter();
		while(!iter.empty())
		{
			for(int i = 0; i < iter->value->Count(); i++)
				delete iter->value->Element(i);

			iter.next();
		}
		m_PreActionList.clear();
	}

}

void CActionManager::Add(INextBotAction_ptr const pAction)
{
	if(IsValidAction(pAction))
	{
		return;
	}

	m_action.add(pAction);
	OnActionCreate(pAction);
}

void CActionManager::Remove(INextBotAction_ptr const pAction)
{
	if(!IsValidAction(pAction))
	{
		return;
	}

	OnAcdionDestroy(pAction);
	m_action.removeIfExists(pAction);
}

void CActionManager::OnActionCreate(INextBotAction_ptr pAction)
{
	BeginActionProcessing(pAction);
}

void CActionManager::OnAcdionDestroy(INextBotAction_ptr pAction)
{
	StopActionProcessing(pAction);
}

CUtlVector<IHandleAction*> *CActionManager::GetCallBackList(INextBotAction_ptr const pAction, bool post_list)
{
	if(post_list)
	{
		auto r = m_PostActionList.find(pAction);
		if(r.found())
			return r->value;
	} else {
		auto r = m_PreActionList.find(pAction);
		if(r.found())
			return r->value;
	}

    return nullptr;
}

bool CActionManager::AddHook(INextBotAction_ptr const pAction, IHandleAction *pCallBack, bool bPost)
{
	if(!pAction)
		return false;

	if(bPost)
	{
		auto res = m_PostActionList.find(pAction);
		if(!res.found())
		{
			auto r = m_PostActionList.findForAdd(pAction);
			if(r.found())
			{
				Error("[CActionManager::AddHook] post Action List called two times!\n");
				return false;
			}
			CUtlVector<IHandleAction*> *l = new CUtlVector<IHandleAction*>();
			l->AddToTail(pCallBack);
			m_PostActionList.add(r, pAction, l);
			this->Add(pAction);
			return true;
		}

		res->value->AddToTail(pCallBack);
	}
	else
	{
		auto res = m_PreActionList.find(pAction);
		if(!res.found())
		{
			auto r = m_PreActionList.findForAdd(pAction);
			if(r.found())
			{
				Error("[CActionManager::AddHook] Pre Action List called two times!\n");
				return false;
			}
			CUtlVector<IHandleAction*> *l = new CUtlVector<IHandleAction*>();
			l->AddToTail(pCallBack);
			m_PreActionList.add(r, pAction, l);
			this->Add(pAction);
			return true;
		}

		res->value->AddToTail(pCallBack);
	}

	return true;
}

void CActionManager::RemoveHook(INextBotAction_ptr const pAction)
{
	this->Remove(pAction);

	{
		auto r = m_PostActionList.find(pAction);
		if(r.found())
		{
			for(int i = 0; i < r->value->Count(); i++)
			{
				delete r->value->Element(i);
			}

			m_PostActionList.remove(r);
		}
	}

	{
		auto r = m_PreActionList.find(pAction);
		if(r.found())
		{
			for(int i = 0; i < r->value->Count(); i++)
			{
				delete r->value->Element(i);
			}

			m_PreActionList.remove(r);
		} 
	}
}
