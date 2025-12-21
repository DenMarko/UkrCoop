#include "CActionImpl.h"
#include "CActionVtableSwap.h"
#include "CActionHook.h"
#include "detours.h"

CDetour *g_pDestructLock = nullptr;

void __action_swap_vtable(void* action)
{
	auto r = g_mVirtualMap.find((INextBotAction_ptr)action);

	if (!r.found())
	{
		Error("Autoswap couldn't find vtable");
		return;
	}

	__internal_data data = r->value;
	vTable_swap(action, &data);
}

void __action_unswap_vtable(void* action)
{
	vTable_swap(action, g_pActionProcess);
}

class CAction_Propcoss_DestructorClass
{
public:
    using FuncOrig = MemberClassFunctionWrapper<void, CAction_Propcoss_DestructorClass>;
    void CAction_Propcoss_Destructor();
    static MemberClassFunctionWrapper<void, CAction_Propcoss_DestructorClass> CAction_Propcoss_Destructor_Actual;
};
CAction_Propcoss_DestructorClass::FuncOrig CAction_Propcoss_DestructorClass::CAction_Propcoss_Destructor_Actual;

void CAction_Propcoss_DestructorClass::CAction_Propcoss_Destructor()
{
	INextBotAction_ptr action = (INextBotAction_ptr)this;
	if (action == (INextBotAction_ptr)g_pActionProcess)
	{
		CAction_Propcoss_Destructor_Actual(this);
		return;
	}
	g_pActionManager->RemoveHook(action);
    delete action;
}

bool BeginActionProcessing(INextBotAction_ptr action)
{
	if (!g_pDestructLock)
	{
		g_pDestructLock = CDetourManager::CreateDetour(GetCodeAddress(&CAction_Propcoss_DestructorClass::CAction_Propcoss_Destructor), 
        CAction_Propcoss_DestructorClass::CAction_Propcoss_Destructor_Actual, (void*)(vTable_get00(g_pActionProcess)[1]));

		if (!g_pDestructLock)
		{
			Error("Failed to create ActionProcessor__Destructor detour");
			return false;
		}

		g_pDestructLock->EnableDetour();
	}

	auto r = g_mVirtualMap.findForAdd(action);
	if (r.found())
	{
		Error("BeginActionProcessing called two times!");
		return false;
	}

	__internal_data data = { vTable_get00(action), vTable_get01(action) };
	g_mVirtualMap.add(r, action, data);
	vTable_swap(action, g_pActionProcess);
	return true;
}


bool StopActionProcessing(INextBotAction_ptr action)
{
	auto r = g_mVirtualMap.find(action);

	if (!r.found())
	{
		Error("StopActionProcessing failed to find action!");
		return false;
	}

	__internal_data data = r->value;
	vTable_swap(action, &data);
	g_mVirtualMap.remove(r);
	return true;
}

void StopActionProcessing()
{
	auto iter = g_mVirtualMap.iter();

	while (!iter.empty())
	{
		StopActionProcessing(iter->key);
		iter.next();
	}

	if (g_pDestructLock)
	{
		g_pDestructLock->Destroy();
		g_pDestructLock = nullptr;
	}
}

Autoswap::Autoswap(const void* action)
{
	m_action = const_cast<void*>(action);
	__action_swap_vtable(m_action);
}

Autoswap::~Autoswap()
{
	__action_unswap_vtable(m_action);
}