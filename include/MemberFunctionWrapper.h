#ifndef _INCLUDE_MEMBER_FUNCTION_WRAPPER_
#define _INCLUDE_MEMBER_FUNCTION_WRAPPER_

#include <bits/move.h>

template<typename R, typename T, typename... ARGC>
class MemberClassFunctionWrapper
{
public:
	using Method = R(T::*)(ARGC...);

	MemberClassFunctionWrapper(void *pFun)
	{
		(*(void **)(&(pFunc))) = pFun;
	}
	MemberClassFunctionWrapper(Method pFun) : pFunc(pFun) {}
	MemberClassFunctionWrapper() : pFunc(nullptr) {}

	~MemberClassFunctionWrapper() = default;

	MemberClassFunctionWrapper& operator=(const MemberClassFunctionWrapper& other)
	{
		if (this != &other)
		{
			pFunc = other.pFunc;
		}
		return *this;
	}

	MemberClassFunctionWrapper& operator=(void* other)
	{
		(*(void **)(&(pFunc))) = other;
		return *this;
	}

	MemberClassFunctionWrapper& operator=(Method other)
	{
		pFunc = other;
		return *this;
	}

	operator void**()
	{
		return reinterpret_cast<void**>(&pFunc);
	}

	template<typename... ARGS>
	R operator()(T* pThis, ARGS&&... args) const
	{
		if (pFunc || pThis)
		{
			return (pThis->*pFunc)(std::forward<ARGS>(args)...);
		}
		return R();
	}

private:
	Method pFunc;
};

template<typename R, typename... ARGC>
class MemberFunctionWrapper
{
public:
	using Method = R(*)(ARGC...);

	MemberFunctionWrapper(void *pFun)
	{
		(*(void **)(&(pFunc))) = pFun;
	}
	MemberFunctionWrapper(Method pFun) : pFunc(pFun) {}
	MemberFunctionWrapper() : pFunc(nullptr) {}

	~MemberFunctionWrapper() = default;

	MemberFunctionWrapper& operator=(const MemberFunctionWrapper& other)
	{
		if (this != &other)
		{
			pFunc = other.pFunc;
		}
		return *this;
	}

	MemberFunctionWrapper& operator=(void* other)
	{
		(*(void **)(&(pFunc))) = other;
		return *this;
	}

	MemberFunctionWrapper& operator=(Method other)
	{
		pFunc = other;
		return *this;
	}

	bool operator != (const void *other)
	{
		return ((*(void **)(&(pFunc))) != other);
	}

	bool operator == (const void *other)
	{
		return ((*(void **)(&(pFunc))) == other);
	}

	operator void**()
	{
		return reinterpret_cast<void**>(&pFunc);
	}

	template<typename... ARGS>
	R operator()(ARGS&&... args) const
	{
		if (pFunc)
		{
			return (*pFunc)(std::forward<ARGS>(args)...);
		}
		return R();
	}

private:
	Method pFunc;
};

#endif