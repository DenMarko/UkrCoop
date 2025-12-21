#include "LuaListthink.h"
#include "LuaBridge/LuaBridge.h"

LuaListThink::LuaListThink(CBaseEntity *ptr)
{
    pEntity = ptr;
    iEntity = gamehelpers->ReferenceToIndex(gamehelpers->EntityToBCompatRef(ptr));
    iHookIdPost = -1;
    iHookIdPre = -1;
}

LuaListThink::LuaListThink(const LuaListThink &other)
{
    this->pEntity = other.pEntity;
    this->iHookIdPost = other.iHookIdPost;
    this->iHookIdPre = other.iHookIdPre;
    this->iEntity = other.iEntity;

    if(!other.rCallPost.empty())
    {
        this->rCallPost.clear();
        for(auto item : other.rCallPost)
        {
            this->rCallPost.push_back(item);
        }
    }

    if(!other.rCallPre.empty())
    {
        this->rCallPre.clear();
        for(auto item : other.rCallPre)
        {
            this->rCallPre.push_back(item);
        }
    }
}

LuaListThink::LuaListThink(const LuaListThink *other)
{
    this->pEntity = other->pEntity;
    this->iHookIdPost = other->iHookIdPost;
    this->iHookIdPre = other->iHookIdPre;
    this->iEntity = other->iEntity;

    if(!other->rCallPost.empty())
    {
        this->rCallPost.clear();
        for(auto item : other->rCallPost)
        {
            this->rCallPost.push_back(item);
        }
    }
    
    if(!other->rCallPre.empty())
    {
        this->rCallPre.clear();
        for(auto item : other->rCallPre)
        {
            this->rCallPre.push_back(item);
        }
    }
}

LuaListThink::~LuaListThink()
{
    rCallPost.clear();
    rCallPre.clear();	
}

LuaListThink LuaListThink::operator=(const LuaListThink &other)
{
    this->pEntity = other.pEntity;
    this->iHookIdPost = other.iHookIdPost;
    this->iHookIdPre = other.iHookIdPre;
    this->iEntity = other.iEntity;

    if(!other.rCallPost.empty())
    {
        this->rCallPost.clear();
        for(auto item : other.rCallPost)
        {
            this->rCallPost.push_back(item);
        }
    }

    if(!other.rCallPre.empty())
    {
        this->rCallPre.clear();
        for(auto item : other.rCallPre)
        {
            this->rCallPre.push_back(item);
        }
    }

    return *this;
}

LuaListThink LuaListThink::operator=(const LuaListThink *other)
{
    this->pEntity = other->pEntity;
    this->iHookIdPost = other->iHookIdPost;
    this->iHookIdPre = other->iHookIdPre;
    this->iEntity = other->iEntity;

    if(!other->rCallPost.empty())
    {
        this->rCallPost.clear();
        for(auto item : other->rCallPost)
        {
            this->rCallPost.push_back(item);
        }
    }

    if(!other->rCallPre.empty())
    {
        this->rCallPre.clear();
        for(auto item : other->rCallPre)
        {
            this->rCallPre.push_back(item);
        }
    }
    return *this;
}
