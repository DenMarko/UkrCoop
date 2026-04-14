#ifndef _HEADER_ACTION_VTABLE_SWAP_INLUDE_
#define _HEADER_ACTION_VTABLE_SWAP_INLUDE_

#include "CActionShared.h"

using vtable_ptr                        = uintptr_t*;

struct __internal_data
{
    vtable_ptr vptr00;
    vtable_ptr vptr01;
};

ke::HashMap<INextBotAction_ptr, __internal_data, ke::PointerPolicy<INextBotAction>> g_mVirtualMap;

void InitVirtualMap()
{
    g_mVirtualMap.init();
}

template<typename T1, typename T2>
inline void vTable_swap(T1* l, T2* r)
{
	__internal_data* sl = reinterpret_cast<__internal_data*>(l);
	__internal_data* sr = reinterpret_cast<__internal_data*>(r);

	sl->vptr00 = sr->vptr00;
	sl->vptr01 = sr->vptr01;
}

template<typename T1>
inline void vTable_swap(T1* l, __internal_data* r)
{
	__internal_data* sl = reinterpret_cast<__internal_data*>(l);

	sl->vptr00 = r->vptr00;
	sl->vptr01 = r->vptr01;
}

template<typename T1>
inline vtable_ptr vTable_get00(T1* base)
{
	return reinterpret_cast<__internal_data*>(base)->vptr00;
}

template<typename T1>
inline vtable_ptr vTable_get01(T1* base)
{
	return reinterpret_cast<__internal_data*>(base)->vptr01;
}


#endif