#ifndef _HEADER_VTABLE_HOOK_HELPER_
#define _HEADER_VTABLE_HOOK_HELPER_

#include "extension.h"

class CVTableHook
{
public:
	CVTableHook(void *takenclass)
	{
		this->vtableptr = *reinterpret_cast<void ***>(takenclass);
		this->hookid = 0;
	}

	CVTableHook(void *vtable, int hook)
	{
		this->vtableptr = vtable;
		this->hookid = hook;
	}

	CVTableHook(CVTableHook &other)
	{
		this->vtableptr = other.vtableptr;
		this->hookid = other.hookid;

		other.hookid = 0;
	}

	CVTableHook(CVTableHook *other)
	{
		this->vtableptr = other->vtableptr;
		this->hookid = other->hookid;

		other->hookid = 0;
	}

	~CVTableHook()
	{
		if (this->hookid)
		{
			SH_REMOVE_HOOK_ID(this->hookid);
			this->hookid = 0;
		}
	}
public:
	void *GetVTablePtr(void)
	{
		return this->vtableptr;
	}

	void SetHookID(int hook)
	{
		this->hookid = hook;
	}

	bool IsHooked(void)
	{
		return (this->hookid != 0);
	}
public:
	bool operator == (CVTableHook &other)
	{
		return (this->GetVTablePtr() == other.GetVTablePtr());
	}

	bool operator == (CVTableHook *other)
	{
		return (this->GetVTablePtr() == other->GetVTablePtr());
	}

	bool operator != (CVTableHook &other)
	{
		return (this->GetVTablePtr() != other.GetVTablePtr());
	}

	bool operator != (CVTableHook *other)
	{
		return (this->GetVTablePtr() != other->GetVTablePtr());
	}
private:
	void *vtableptr;
	int hookid;
};


#endif  //_HEADER_VTABLE_HOOK_HELPER_