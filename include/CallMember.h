#ifndef _INCLUDE_CALL_MEMBER_H_
#define _INCLUDE_CALL_MEMBER_H_
#include "MemberFunctionWrapper.h"

template<typename R, typename T, typename... ARGC>
class WrapperFuncClassToAddr
{
public:
	using Method = R(T::*)(ARGC...);

	WrapperFuncClassToAddr(const char* szSignature) : IsSupported(false)
	{
        void *addr = nullptr;
        if(g_pGameConf->GetMemSig(szSignature, &addr) && addr != nullptr)
        {
            SetFunc(addr);
            IsSupported = true;
        }
	}
	WrapperFuncClassToAddr(void *pFun) : IsSupported(true) { (*(void **)(&(pFunc))) = pFun; }
	WrapperFuncClassToAddr(Method pFun) : pFunc(pFun), IsSupported(true) { }
	WrapperFuncClassToAddr() : pFunc(nullptr), IsSupported(false) { }

	~WrapperFuncClassToAddr() = default;

	const bool Supported() const
	{
		return IsSupported;
	}

	WrapperFuncClassToAddr& operator=(const WrapperFuncClassToAddr& other)
	{
		if (this != &other)
		{
			pFunc = other.pFunc;
			IsSupported = other.IsSupported;
		}
		return *this;
	}

	WrapperFuncClassToAddr& operator=(void* other)
	{
		(*(void **)(&(pFunc))) = other;
		IsSupported = true;
		return *this;
	}

	WrapperFuncClassToAddr& operator=(Method other)
	{
		pFunc = other;
		IsSupported = true;
		return *this;
	}

	operator void**()
	{
		return reinterpret_cast<void**>(&pFunc);
	}

	template<typename... ARGS>
	R operator()(T* pThis, ARGS&&... args) const
	{
		return (pThis->*pFunc)(std::forward<ARGS>(args)...);
	}

private:
	void SetFunc(void *pAddr)
	{
		(*(void **)(&(pFunc))) = pAddr;
	}

	Method pFunc;
	bool IsSupported;
};

template<typename R, typename... ARGC>
class WrapperFuncToAddr
{
public:
	using Method = R(*)(ARGC...);

	WrapperFuncToAddr(const char* szSignature) : IsSupported(false)
	{
        void *addr = nullptr;
        if(g_pGameConf->GetMemSig(szSignature, &addr) && addr != nullptr)
        {
            SetFunc(addr);
            IsSupported = true;
        }
	}
	WrapperFuncToAddr(void *pFun) : IsSupported(true) { (*(void **)(&(pFunc))) = pFun; }
	WrapperFuncToAddr(Method pFun) : pFunc(pFun), IsSupported(true) { }
	WrapperFuncToAddr() : pFunc(nullptr), IsSupported(false) { }

	~WrapperFuncToAddr() = default;

	const bool Supported() const
	{
		return IsSupported;
	}

	WrapperFuncToAddr& operator=(const WrapperFuncToAddr& other)
	{
		if (this != &other)
		{
			pFunc = other.pFunc;
			IsSupported = other.IsSupported;
		}
		return *this;
	}

	WrapperFuncToAddr& operator=(void* other)
	{
		(*(void **)(&(pFunc))) = other;
		IsSupported = true;
		return *this;
	}

	WrapperFuncToAddr& operator=(Method other)
	{
		pFunc = other;
		IsSupported = true;
		return *this;
	}

	operator void**()
	{
		return reinterpret_cast<void**>(&pFunc);
	}

	template<typename... ARGS>
	R operator()(ARGS&&... args) const
	{
		return (pFunc)(std::forward<ARGS>(args)...);
	}

private:
	void SetFunc(void *pAddr)
	{
		(*(void **)(&(pFunc))) = pAddr;
	}

	Method pFunc;
	bool IsSupported;
};

#endif