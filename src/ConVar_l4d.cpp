#include "ConVar_l4d.h"
#include "LuaBridge/LuaBridge.h"

struct sCvarRefList
{
	sCvarRefList(luabridge::LuaRef &mRef) : pRef(mRef) {}
	~sCvarRefList(){}
	luabridge::LuaRef pRef;
};

struct ConVarInfo
{
	ConVarInfo()
	{}
	~ConVarInfo()
	{
		for(auto List : changeListers)
			delete List;

        changeListers.clear();
        changeListersLua.clear();
	}
	SourceHook::String C_Name;
	SourceHook::List<IConVarChangeListener *> changeListers;
	SourceHook::List<sCvarRefList> changeListersLua;
};
SourceHook::List<ConVarInfo> m_ComVarInfo;

ConVarInfo *FindConVarInfo(const char *CVar)
{
    auto iter = m_ComVarInfo.begin();

    while(iter != m_ComVarInfo.end())
    {
        if(g_Sample.my_strcmp((*iter).C_Name.c_str(), CVar) == 0)
        {
            return &(*iter);
        }
        iter++;
    }

    ConVarInfo pInfo;
    pInfo.C_Name.assign(CVar);
    m_ComVarInfo.push_back(pInfo);

    return &m_ComVarInfo.back();
}

ConVar_l4d::ConVar_l4d()
{
    luabridge::getGlobalNamespace(g_Sample.GetLuaState())
        .beginClass<ConVar_l4d>("ConVar")
            .addFunction("getConVarString", &ConVar_l4d::GetConVarString)
            .addFunction("getConVarFloat", &ConVar_l4d::GetConVarFloat)
            .addFunction("getConVarInt", &ConVar_l4d::GetConVarInt)
            .addFunction("setConVarString", &ConVar_l4d::SetConVarString)
            .addFunction("setConVarFloat", &ConVar_l4d::SetConVarFloat)
            .addFunction("setConVarInt", &ConVar_l4d::SetConVarInt)
            .addFunction("setBounds", &ConVar_l4d::SetBounds)
            .addFunction("AddHookChanged", std::function<void(ConVar_l4d*, const char*, luabridge::LuaRef)>([](ConVar_l4d* pThisPtr, const char* CVarName, luabridge::LuaRef sCallBack)
            {
                if(!sCallBack.isFunction())
                {
                    return;
                }

                ConVarInfo *pCvar = FindConVarInfo(CVarName);
                if(pCvar != nullptr)
                {
                    pCvar->changeListersLua.push_back(sCvarRefList(sCallBack));
                }
            }))
            .addFunction("RemoveHookChanged", std::function<void(ConVar_l4d*, const char*, luabridge::LuaRef)>([](ConVar_l4d* pThisPtr, const char* CVarName, luabridge::LuaRef sCallBack)
            {
                if(!sCallBack.isFunction())
                {
                    return;
                }

                ConVarInfo *pCvar = FindConVarInfo(CVarName);
                if(pCvar != nullptr)
                {
                    auto iter = pCvar->changeListersLua.begin();
                    while(iter != pCvar->changeListersLua.end())
                    {
                        if(iter->pRef == sCallBack)
                        {
                            pCvar->changeListersLua.erase(iter);
                            return;
                        }
                    }
                }
            }))
        .endClass();
    luabridge::setGlobal(g_Sample.GetLuaState(), this, "ConVar");
}

ConVar_l4d::~ConVar_l4d()
{
    m_ComVarInfo.clear();
}

void ConVar_l4d::OnConVarGanger(ConVar* pConVar, const char* oldValue, float flOldValue)
{
    if(g_Sample.my_strcmp(pConVar->GetString(), oldValue) == 0)
    {
        return;
    }
    
    ConVarInfo *pCvarInfo = FindConVarInfo(pConVar->GetName());

    if(pCvarInfo == nullptr)
    {
        return;
    }

    if(pCvarInfo->changeListers.size() != 0)
    {
        for(auto& pCallback : pCvarInfo->changeListers)
        {
            pCallback->OnConVarChanged(pConVar, oldValue, pConVar->GetString());
        }
    }

    if(pCvarInfo->changeListersLua.size())
    {
        for(auto& pCallBacks : pCvarInfo->changeListersLua)
        {
            if(pCallBacks.pRef.isFunction())
                pCallBacks.pRef(pConVar->GetName(), oldValue, pConVar->GetString());
        }
    }
    return;
}

void ConVar_l4d::AddHookConVarChanged(const char* CVarName, IConVarChangeListener *pCallBack)
{
    if(pCallBack == nullptr)
    {
        return;
    }

    ConVarInfo *pCvar = FindConVarInfo(CVarName);
    if(pCvar != nullptr)
    {
        pCvar->changeListers.push_back(pCallBack);
    }
}

void ConVar_l4d::RemoveConVarChanged(const char* CvarName, IConVarChangeListener *pCallBack)
{
    if(pCallBack == nullptr)
    {
        return;
    }

    ConVarInfo *pCvar = FindConVarInfo(CvarName);
    if(pCvar != nullptr)
    {
        pCvar->changeListers.remove(pCallBack);
    }
}

const char *ConVar_l4d::GetConVarString(const char *name)
{
    ConVar *Cvar = icvar->FindVar(name);
    if(Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", name);
        return nullptr;
    }

    return Cvar->GetString();
}

void ConVar_l4d::SetConVarString(const char *cvar, const char *values)
{
    ConVar *Cvar = icvar->FindVar(cvar);
    if(Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", cvar);
        return;
    }

    Cvar->SetValue(values);
    return;
}

float ConVar_l4d::GetConVarFloat(const char *cvar){
    ConVar *Cvar = icvar->FindVar(cvar);
    if(Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", cvar);
        return -1.0;
    }

    return Cvar->GetFloat();
}

void ConVar_l4d::SetConVarFloat(const char *cvar, float values){
    ConVar *Cvar = icvar->FindVar(cvar);
    if(Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", cvar);
        return;
    }

    Cvar->SetValue(values);
    return;
}

int ConVar_l4d::GetConVarInt(const char *cvar){
    ConVar *Cvar = icvar->FindVar(cvar);
    if(Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", cvar);
        return -1;
    }

    return Cvar->GetInt();
}

void ConVar_l4d::SetConVarInt(const char *cvar, int value){
    ConVar *Cvar = icvar->FindVar(cvar);
    if(Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", cvar);
        return;
    }

    Cvar->SetValue(value);
    return;
}

void ConVar_l4d::SetBounds(const char *cvar, bool haxMin, float valMin, bool haxMax, float valMax)
{
    ConVar *Cvar = icvar->FindVar(cvar);
    if (Cvar == nullptr)
    {
        Msg("ConVar is not found %s\n", cvar);
        return;
    }

    float MinVal = 0.f;
    float MaxVal = 0.f;

    Cvar->GetMin(MinVal);
    if(valMin != MinVal)
        Cvar->SetMin(haxMin, valMin);

    Cvar->GetMax(MaxVal);
    if (valMax != MaxVal)
        Cvar->SetMax(haxMax, valMax);
}
