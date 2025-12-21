#include <typeinfo>

#ifndef HEDER_CONFIGS
#define HEDER_CONFIGS

#define IS_VOTE_ENABLE
//#define _NET_PROP_DUMP_
#define TEST

#define ENTFLAG_ONGROUND			    (1 << 0)
#define ENTFLAG_DUCKING				    (1 << 1)
#define ENTFLAG_WATERJUMP			    (1 << 2)
#define ENTFLAG_ONTRAIN				    (1 << 3)
#define ENTFLAG_INRAIN				    (1 << 4)
#define ENTFLAG_FROZEN				    (1 << 5)
#define ENTFLAG_ATCONTROLS			    (1 << 6)
#define ENTFLAG_CLIENT				    (1 << 7)
#define ENTFLAG_FAKECLIENT			    (1 << 8)
#define ENTFLAG_INWATER				    (1 << 9)
#define ENTFLAG_FLY					    (1 << 10)
#define ENTFLAG_SWIM				    (1 << 11)
#define ENTFLAG_CONVEYOR			    (1 << 12)
#define ENTFLAG_NPC					    (1 << 13)
#define ENTFLAG_GODMODE				    (1 << 14)
#define ENTFLAG_NOTARGET			    (1 << 15)
#define ENTFLAG_AIMTARGET			    (1 << 16)
#define ENTFLAG_PARTIALGROUND		    (1 << 17)
#define ENTFLAG_STATICPROP			    (1 << 18)
#define ENTFLAG_GRAPHED				    (1 << 19)
#define ENTFLAG_GRENADE				    (1 << 20)
#define ENTFLAG_STEPMOVEMENT		    (1 << 21)
#define ENTFLAG_DONTTOUCH			    (1 << 22)
#define ENTFLAG_BASEVELOCITY		    (1 << 23)
#define ENTFLAG_WORLDBRUSH			    (1 << 24)
#define ENTFLAG_OBJECT				    (1 << 25)
#define ENTFLAG_KILLME				    (1 << 26)
#define ENTFLAG_ONFIRE				    (1 << 27)
#define ENTFLAG_DISSOLVING			    (1 << 28)
#define ENTFLAG_TRANSRAGDOLL		    (1 << 29)
#define ENTFLAG_UNBLOCKABLE_BY_PLAYER	(1 << 30)
#define ENTFLAG_FREEZING			    (1 << 31)
#define ENTFLAG_EP2V_UNKNOWN1		    (1 << 31)

#define ARGS_BUFFER_LENGTH	8192

#define MAX_COORD_INTEGER (16384)
#define MIN_COORD_INTEGER (-MAX_COORD_INTEGER)

#define DEBUG_LOG(...) \
if(ukr_cvar_debug.GetBool()) \
	m_sLog->LogToFileEx(__VA_ARGS__);

#define RELEASE(iface) \
if(iface != nullptr) \
{ \
	delete iface; \
	iface = nullptr; \
}

#define IS_VALID_CLIENT(client) \
edict_t *pEdict##client = nullptr; \
IGamePlayer *pClient##client = nullptr; \
if((client >= 1) && (client <= playerhelpers->GetMaxClients())) \
{ \
    pClient##client = playerhelpers->GetGamePlayer(client); \
    if(!pClient##client) \
    { \
        return pContext->ThrowNativeError("Client index %d is not valid", client); \
    } \
    if(!pClient##client->IsConnected() && !pClient##client->IsInGame()) \
    { \
        return pContext->ThrowNativeError("Client is not connected or is not in game"); \
    } \
} \
else if(client <= 0 || client > g_pGlobals->maxEntities) \
{ \
    return pContext->ThrowNativeError("Entity index %d is not valid", client); \
} \
else \
{ \
    pEdict##client = g_Sample.PEntityOfEntIndex(client); \
    if(!pEdict##client || pEdict##client->IsFree()) \
        return pContext->ThrowNativeError("edict is not valid or is free"); \
}

template<typename R, typename T>
R& access_member(T* obj, unsigned int offset)
{
	return *reinterpret_cast<R*>(reinterpret_cast<uint8_t*>(obj) + offset);
}

template<typename R>
R* access_array_member(void* obj, unsigned int offset)
{
    return reinterpret_cast<R*>(reinterpret_cast<uint8_t*>(obj) + offset);
}

template<typename R>
R& access_array_member(void* obj, unsigned int baseOffset, size_t index)
{
    return *reinterpret_cast<R*>(reinterpret_cast<uint8_t*>(obj) + baseOffset + sizeof(R) * index);
}


#define MY_VOID_SH_MANUALHOOK_RECONFIGURE_NOPARAM(baseclass, hookname)	\
	using namespace ::SourceHook;	\
	MemFuncInfo ms_MFI = {true, -1, 0, 0};	\
	GetFuncInfo((static_cast<void (baseclass::*)() > (&baseclass::hookname)), ms_MFI);	\
	__SourceHook_FHM_Reconfigure ## hookname(ms_MFI.vtblindex, ms_MFI.vtbloffs, ms_MFI.thisptroffs);	\


#define MY_VOID_SH_MANUALHOOK_RECONFIGURE_4_PARAM(baseclass, hookname, param1, param2, param3, param4)	\
	using namespace ::SourceHook;	\
	MemFuncInfo ms_MFI = {true, -1, 0, 0};	\
	GetFuncInfo((static_cast<void (baseclass::*)(param1, param2, param3, param4) > (&baseclass::hookname)), ms_MFI);	\
	__SourceHook_FHM_Reconfigure ## hookname(ms_MFI.vtblindex, ms_MFI.vtbloffs, ms_MFI.thisptroffs);	\

#endif