#ifndef _LUA_LIST_THINCK_H_
#define _LUA_LIST_THINCK_H_
#include "extension.h"

namespace luabridge
{
    class LuaRef;
}

class LuaListThink
{
public:
	LuaListThink(CBaseEntity *ptr);
	LuaListThink(const LuaListThink &other);
	LuaListThink(const LuaListThink *other);
	
	~LuaListThink();

public:
	void SetLuaRefPost(const luabridge::LuaRef &rCalls)
	{
		rCallPost.push_back(rCalls);
	}
	void SetLuaRefPre(const luabridge::LuaRef &rCalls)
	{
		rCallPre.push_back(rCalls);
	}

	CBaseEntity *GetEntityPtr() const
	{
		return pEntity;
	}

	int GetEntityId() const
	{
		return iEntity;
	}

	const SourceHook::List<luabridge::LuaRef> &GetListPost() const
	{
		return rCallPost;
	}

	const SourceHook::List<luabridge::LuaRef> &GetListPre() const
	{
		return rCallPre;
	}

	int GetHookPostId() const
	{
		return iHookIdPost;
	}
	void SetHookPostId(int id)
	{
		iHookIdPost = id;
	}

	int GetHookPreId() const
	{
		return iHookIdPre;
	}
	void SetHookPreId(int id)
	{
		iHookIdPre = id;
	}

public:
	bool operator == (LuaListThink &other) const
	{
		return (this->GetEntityPtr() == other.GetEntityPtr());
	}
	bool operator == (LuaListThink *other) const
	{
		return (this->GetEntityPtr() == other->GetEntityPtr());
	}

	bool operator != (LuaListThink &other) const
	{
		return (this->GetEntityPtr() != other.GetEntityPtr());
	}
	bool operator != (LuaListThink *other) const
	{
		return (this->GetEntityPtr() != other->GetEntityPtr());
	}

	bool operator == (CBaseEntity *other) const
	{
		return (this->GetEntityPtr() == other);
	}
	bool operator != (CBaseEntity *other) const
	{
		return (this->GetEntityPtr() != other);
	}

	bool operator == (int other) const
	{
		return (this->GetEntityId() == other);
	}
	bool operator != (int other) const
	{
		return (this->GetEntityId() != other);
	}

	LuaListThink operator = (const LuaListThink &other);
	LuaListThink operator = (const LuaListThink *other);

private:
	CBaseEntity *pEntity;
	int iEntity;
	int iHookIdPost;
	int iHookIdPre;
	SourceHook::List<luabridge::LuaRef> rCallPost;
	SourceHook::List<luabridge::LuaRef> rCallPre;
};


#endif