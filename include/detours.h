#ifndef _INCLUDE_SOURCEMOD_DETOURS_H_
#define _INCLUDE_SOURCEMOD_DETOURS_H_

#include "extension.h"
#include "../../sourcemod/public/jit/jit_helpers.h"
#include "../../sourcemod/public/jit/x86/x86_macros.h"
#include "detourhelpers.h"
#include "MemberFunctionWrapper.h"

#define DETOUR_MEMBER_CALL1(name, ...) name##_Actual(this, __VA_ARGS__)
#define DETOUR_MEMBER_CALL2(name) name##_Actual(this)

#define DETOUR_STATIC_CALL(name, ...) name##_Actual(__VA_ARGS__)

#define DETOUR_DECL_STATIC0(name, ret) \
MemberFunctionWrapper<ret> name##_Actual; \
ret member_##name(void)

#define DETOUR_DECL_STATIC1(name, ret, p1type, p1name) \
MemberFunctionWrapper<ret, p1type> name##_Actual; \
ret member_##name(p1type p1name)

#define DETOUR_DECL_STATIC2(name, ret, p1type, p1name, p2type, p2name) \
MemberFunctionWrapper<ret, p1type, p2type> name##_Actual; \
ret member_##name(p1type p1name, p2type p2name)

#define DETOUR_DECL_STATIC3(name, ret, p1type, p1name, p2type, p2name, p3type, p3name) \
MemberFunctionWrapper<ret, p1type, p2type, p3type> name##_Actual; \
ret member_##name(p1type p1name, p2type p2name, p3type p3name)

#define DETOUR_DECL_STATIC4(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name) \
MemberFunctionWrapper<ret, p1type, p2type, p3type, p4type> name##_Actual; \
ret member_##name(p1type p1name, p2type p2name, p3type p3name, p4type p4name)

#define DETOUR_DECL_STATIC5(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name) \
MemberFunctionWrapper<ret, p1type, p2type, p3type, p4type, p5type> name##_Actual; \
ret member_##name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name)

#define DETOUR_DECL_STATIC6(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name) \
MemberFunctionWrapper<ret, p1type, p2type, p3type, p4type, p5type, p6type> name##_Actual; \
ret member_##name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name)

#define DETOUR_DECL_MEMBER0(name, ret) \
class name##Class \
{ \
public: \
	ret name(); \
}; \
MemberClassFunctionWrapper<ret, name##Class> name##_Actual; \
ret name##Class::name()

#define DETOUR_DECL_MEMBER1(name, ret, p1type, p1name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type> name##_Actual; \
ret name##Class::name(p1type p1name)

#define DETOUR_DECL_MEMBER2(name, ret, p1type, p1name, p2type, p2name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name)

#define DETOUR_DECL_MEMBER3(name, ret, p1type, p1name, p2type, p2name, p3type, p3name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name, p3type p3name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type, p3type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name)

#define DETOUR_DECL_MEMBER4(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type, p3type, p4type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name)

#define DETOUR_DECL_MEMBER5(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type, p3type, p4type, p5type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name)

#define DETOUR_DECL_MEMBER6(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type, p3type, p4type, p5type, p6type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name)

#define DETOUR_DECL_MEMBER7(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name, p7type, p7name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type, p3type, p4type, p5type, p6type, p7type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name)

#define DETOUR_DECL_MEMBER8(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name, p7type, p7name, p8type, p8name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name, p8type p8name); \
}; \
MemberClassFunctionWrapper<ret, name##Class, p1type, p2type, p3type, p4type, p5type, p6type, p7type, p8type> name##_Actual; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name, p8type p8name)


#define GET_MEMBER_CALLBACK(name) (void *)GetCodeAddress(&name##Class::name)

#define GET_STATIC_CALLBACK(name) (void *)&member_##name

#define DETOUR_CREATE_MEMBER(name, gamedata) CDetourManager::CreateDetour(GET_MEMBER_CALLBACK(name), name##_Actual, gamedata);
#define DETOUR_CREATE_STATIC(name, gamedata) CDetourManager::CreateDetour(GET_STATIC_CALLBACK(name), name##_Actual, gamedata);
#define DETOUR_CREATE_STATIC_PTR(name, ptr) CDetourManager::CreateDetour(GET_STATIC_CALLBACK(name), name##_Actual, ptr);
#define DETOUR_CREATE_MEMBER_PTR(name, ptr) CDetourManager::CreateDetour(GET_MEMBER_CALLBACK(name), name##_Actual, ptr);


class GenericClass {};
typedef void (GenericClass::*VoidFunc)();

inline void *GetCodeAddr(VoidFunc mfp)
{
	return *(void **)&mfp;
}

#define GetCodeAddress(mfp) GetCodeAddr(reinterpret_cast<VoidFunc>(mfp))

class CDetourManager;

class CDetour
{
public:

	bool IsEnabled();

	void EnableDetour();
	void DisableDetour();

	void Destroy();

	friend class CDetourManager;

protected:
	CDetour(void *callbackfunction, void **trampoline, const char *signame);
	CDetour(void*callbackfunction, void **trampoline, void *pAddress);

	bool Init(ISourcePawnEngine *spengine, IGameConfig *gameconf);
private:

	bool CreateDetour();
	void DeleteDetour();

	bool enabled;
	bool detoured;

	patch_t detour_restore;
	void *detour_address;
	void *detour_trampoline;
	void *detour_callback;
	void **trampoline;

	const char *signame;
	ISourcePawnEngine *spengine;
	IGameConfig *gameconf;
};

class CDetourManager
{
public:

	static void Init(ISourcePawnEngine *spengine, IGameConfig *gameconf);

	static CDetour *CreateDetour(void *callbackfunction, void **trampoline, const char *signame);
	static CDetour *CreateDetour(void *callbackfunction, void **trampoline, void *pAddress);

	friend class CBlocker;
	friend class CDetour;

private:
	static ISourcePawnEngine *spengine;
	static IGameConfig *gameconf;
};

#endif // _INCLUDE_SOURCEMOD_DETOURS_H_
