#ifndef _INCLUDE_HL2_H_
#define _INCLUDE_HL2_H_

#include "log_messege.h"
#include "dt_send.h"
#include "server_class.h"

//#define _DEST_HINTTEXT  1
//#define _DEST_CONSOLE	2
//#define _DEST_CHAT		3
//#define _DEST_CENTER	4

#define ptr_size sizeof(void *)
#define int_size sizeof(int)

#define _ptr void *

struct CallHelper {
	CallHelper() : call(nullptr), supported(false), setup(false) {}
	void Shutdown() {
		if (call) {
			call->Destroy();
			call = nullptr;
			supported = false;
		}
	}
	ICallWrapper *call;
	bool supported;
	bool setup;
};

class HL2
{
private:
	int msgid[sizeof(DEST)+1];

public:

	void GetVelocity(CBaseEntity *pEntity, Vector *velocity, AngularImpulse *angvelocity);
	void Teleport(CBaseEntity *pEntity, Vector *origin, QAngle *ang, Vector *velocity);

	/*CTerrorPlayer::RoundRespawn(void)*/
	void PlayerRespawn(CBaseEntity *pEntity);
	/*CTerrorPlayer::OnStaggered(CBaseEntity *pEntity, Vector *pVector)*/
	void PlayerStartget(CBaseEntity *pEntity, CBaseEntity *pTarget, Vector *pVector);
	/*CTerrorPlayer::OnVomitedUpon(CTerrorPlayer *, bool, bool)*/
	void PlayerVomitUpon(CBaseEntity *pEntity, CBaseEntity *aEntity, cell_t param);

	bool TextMsg(int client, DEST dest, const char *msg);
	bool Translate(char *buffer, size_t maxlength, const char *format, unsigned int numparams, size_t *pOutLength, ...);
	void PrintToConsole(edict_t *m_pEdict, const char* msg, ...);
	void PrintToConsole(int client, const char* msg, ...);

	/*-----------------ENTITIES-----------------------*/

	int GetEntProp(cell_t entity, PropTypes type, const char *prop, int size = 4, int element = 0);
	int SetEntProp(cell_t entity, PropTypes type, const char *prop, int value, int size = 4, int element = 0);

	int GetEntPropEnt(cell_t entity, PropTypes type, const char *prop, int element = 0);
	int SetEntPropEnt(cell_t entity, PropTypes type, const char* prop, cell_t other, int element = 0);

public:

	void SetMsgId(int HintText, int TextMsg)
	{
		msgid[DEST::HINTTEXT] = HintText;
		msgid[DEST::CENTER] = msgid[DEST::CHAT] = msgid[DEST::CONSOLE] = TextMsg;
	}

	class VEmptyClass {};
	datamap_t *CBaseEntity_GetDataDescMap(CBaseEntity *pEntity)
	{
		int offset;

		if (!g_pGameConf->GetOffset("GetDataDescMap", &offset) || !offset)
		{
			return NULL;
		}

		void **this_ptr = *reinterpret_cast<void ***>(&pEntity);
		void **vtable = *reinterpret_cast<void ***>(pEntity);
		void *vfunc = vtable[offset];

		union
		{
			datamap_t *(VEmptyClass::*mfpnew)();
	#ifndef PLATFORM_POSIX
			void *addr;
		} u;
		u.addr = vfunc;
	#else
			struct  
			{
				void *addr;
				intptr_t adjustor;
			} s;
		} u;
		u.s.addr = vfunc;
		u.s.adjustor = 0;
	#endif

		return (datamap_t *)(reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)();
	}
private:
	bool SetupVomitUpon();
	bool SetupTeleport();
	bool SetupStargget();
	bool SetupRespawnPlayer();
	bool SetupGetVelocity();
	bool IndexToAThings(cell_t num, CBaseEntity **pEntData, edict_t **pEdictData);
	edict_t *BaseEntityToEdict(CBaseEntity *pEntity)
	{
		IServerUnknown *pUnk = (IServerUnknown *)pEntity;
		IServerNetworkable *pNet = pUnk->GetNetworkable();

		if (!pNet)
		{
			return NULL;
		}

		return pNet->GetEdict();
	}

	int MatchFieldAsInteger(int field_type)
	{
		switch (field_type)
		{
		case FIELD_TICK:
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
		case FIELD_INTEGER:
		case FIELD_COLOR32:
			return 32;
		case FIELD_SHORT:
			return 16;
		case FIELD_CHARACTER:
			return 8;
		case FIELD_BOOLEAN:
			return 1;
		default:
			return 0;
		}

		return 0;
	}
};

extern HL2 g_HL2;

#endif