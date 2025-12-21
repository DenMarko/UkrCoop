#ifndef _HEADER_HANDLE_OPEN_INCLUDE_
#define _HEADER_HANDLE_OPEN_INCLUDE_

template<typename T>
class HandleOpen
{
public:
    HandleOpen() : pObject(), iError(HandleError_None) {}
    explicit HandleOpen(T*t) :pObject(t), iError(HandleError_None) {}
    HandleOpen(const HandleOpen &other) : pObject(other.pObject), iError(other.iError) {}
    HandleOpen(IPluginContext *p_cx, Handle_t handle, HandleType_t type) : pObject(nullptr)
    {
        HandleSecurity sec(p_cx->GetIdentity(), myself->GetIdentity());
        iError = handlesys->ReadHandle(handle, type, &sec, (void**)&pObject);
        if(iError != HandleError_None)
        {
            p_cx->ThrowNativeError("Invalid Handle %x (error %d)", handle, iError);
        }
    }

    bool OK() const
    {
        return (pObject && iError == HandleError_None);
    }

	operator T *() const
    {
		assert(Ok());
		return pObject;
	}

	T *operator *() const
    {
		assert(Ok());
		return pObject;
	}

	T *operator ->() const
    {
		assert(Ok());
		return pObject;
	}

	HandleOpen &operator =(T *t)
    {
		pObject = t;
		iError = HandleError_None;
		return *this;
	}

	HandleOpen &operator =(const HandleOpen &other)
    {
		pObject = other.pObject;
		iError = other.iError;
		return *this;
	}
private:
    T *pObject;
    HandleError iError;
};

#endif