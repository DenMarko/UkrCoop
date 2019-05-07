#include "HL2.h"

bool HL2::TextMsg(int client, DEST dest, const char *msg)
{
	bf_write *pBitBuf = nullptr;
	cell_t players[] = {client};

	if((msgid[dest]) == -1)
	{
		m_sLog.LogToFileEx("message ID is -1");
		return false;
	}

	if((pBitBuf = usermsgs->StartBitBufMessage(msgid[dest], players, 1, USERMSG_RELIABLE)) == NULL)
	{
		m_sLog.LogToFileEx("pBitBuf == NULL");
		return false;
	}

	if(dest != DEST::HINTTEXT){
		pBitBuf->WriteByte(dest);
	}
	pBitBuf->WriteString(msg);

	usermsgs->EndMessage();
	return true;
}

CallHelper s_Teleport;
CallHelper s_GetVelocity;
CallHelper s_RoundRespawn;
CallHelper s_Stargged;
CallHelper s_VomitUpon;

bool HL2::SetupStargget()
{
	if(s_Stargged.setup)
		return s_Stargged.supported;

	void *addr = nullptr;
	if(g_pGameConf->GetMemSig("CTerrorPlayer_OnStaggered", &addr) && addr != nullptr)
	{
		PassInfo info[2];
		info[0].flags = info[1].flags = PASSFLAG_BYVAL;
		info[0].size = info[1].size = ptr_size;
		info[0].type = info[1].type = PassType_Basic;

		s_Stargged.call = g_pBinTools->CreateCall(addr, CallConv_ThisCall, nullptr, info, 2);
		if(s_Stargged.call != nullptr)
			s_Stargged.supported = true;
	}
	else
		m_sLog.LogToFileEx("[UKR COOP]::SetupStargget()-> addr == nullptr");

	s_Stargged.setup = true;
	return s_Stargged.supported;
}

bool HL2::SetupTeleport()
{
	if (s_Teleport.setup)
		return s_Teleport.supported;

	int offset;
	if (g_pGameConf->GetOffset("Teleport", &offset)){
		PassInfo info[3];
		info[0].flags = info[1].flags = info[2].flags = PASSFLAG_BYVAL;
		info[0].size = info[1].size = info[2].size = ptr_size;
		info[0].type = info[1].type = info[2].type = PassType_Basic;

		s_Teleport.call = g_pBinTools->CreateVCall(offset, 0, 0, NULL, info, 3);

		if (s_Teleport.call != NULL)
			s_Teleport.supported = true;
	}
	else
		m_sLog.LogToFileEx("[UKR COOP]::SetupTeleport()-> GetOffset return is false");

	s_Teleport.setup = true;

	return s_Teleport.supported;
}

bool HL2::SetupVomitUpon()
{
	if(s_VomitUpon.setup)
		return s_VomitUpon.supported;

	_ptr addr = nullptr;
	if(g_pGameConf->GetMemSig("CTerrorPlayer_OnVomitedUpon", &addr) && addr != NULL)
	{
		PassInfo info[2];
		info[0].flags = info[1].flags = PASSFLAG_BYVAL;
		info[0].size = info[1].size = int_size;
		info[0].type = info[1].type = PassType_Basic;

		s_VomitUpon.call = g_pBinTools->CreateCall(addr, CallConv_ThisCall, nullptr, info, 2);

		if(s_VomitUpon.call != NULL)
			s_VomitUpon.supported = true;
	}
	else
		m_sLog.LogToFileEx("[UKR COOP]::SetVomitUpon()-> addr == NULL");

	s_VomitUpon.setup = true;
	return s_VomitUpon.supported;
}

void HL2::PlayerVomitUpon(CBaseEntity *pEntity, CBaseEntity *aEntity, cell_t params)
{
	if(SetupVomitUpon())
	{
		unsigned char param[(ptr_size * 2) + int_size];
		unsigned char *vptr = param;

		*(CBaseEntity **)vptr = pEntity;
		vptr += sizeof(CBaseEntity *);
		*(CBaseEntity **)vptr = aEntity;
		vptr += sizeof(CBaseEntity *);
		*(cell_t *)vptr = params;

		s_VomitUpon.call->Execute(param, nullptr);
	}
}

void HL2::Teleport(CBaseEntity *pEntity, Vector *origin, QAngle *ang, Vector *velocity)
{
	if(SetupTeleport())
	{
		unsigned char params[ptr_size * 4];
		unsigned char *vptr = params;
		*(CBaseEntity **)vptr = pEntity;
		vptr += sizeof(CBaseEntity *);
		*(Vector **)vptr = origin;
		vptr += sizeof(Vector *);
		*(QAngle **)vptr = ang;
		vptr += sizeof(QAngle *);
		*(Vector **)vptr = velocity;
	
		s_Teleport.call->Execute(params, NULL);
	}
}

bool HL2::SetupRespawnPlayer()
{
	if(s_RoundRespawn.setup)
		return s_RoundRespawn.supported;

	void *addr = NULL;
	if(g_pGameConf->GetMemSig("RoundRespawn", &addr) && addr != NULL)
	{
		s_RoundRespawn.call = g_pBinTools->CreateCall(addr, CallConv_ThisCall, NULL, NULL, NULL);

		if(s_RoundRespawn.call != NULL)
			s_RoundRespawn.supported = true;
	}
	else
		m_sLog.LogToFileEx("[UKR COOP]::SetupRespawnPlayer()-> addr == NULL");

	s_RoundRespawn.setup = true;
	return s_RoundRespawn.supported;
}

void HL2::PlayerStartget(CBaseEntity *sEntity, CBaseEntity *tEntity, Vector *sVector)
{
	if(this->SetupStargget())
	{
		unsigned char param[ptr_size * 3];
		unsigned char *ptr = param;

		*(CBaseEntity **)ptr = sEntity;
		ptr += sizeof(CBaseEntity *);
		*(CBaseEntity **)ptr = tEntity;
		ptr += sizeof(CBaseEntity *);
		*(Vector **)ptr = sVector;	

		s_Stargged.call->Execute(param, NULL);
	}
}

void HL2::PlayerRespawn(CBaseEntity *pEntity)
{
	if(SetupRespawnPlayer())
	{
		unsigned char param[sizeof(CBaseEntity *)];
		unsigned char *ptr = param;

		*(CBaseEntity **)ptr = pEntity;
		ptr += sizeof(CBaseEntity *);

		s_RoundRespawn.call->Execute(param, NULL);
	}
}

bool HL2::SetupGetVelocity()
{
	if (s_GetVelocity.setup)
	{
		return s_GetVelocity.supported;
	}

	int offset;
	if (g_pGameConf->GetOffset("GetVelocity", &offset))
	{
		PassInfo info[2];
		info[0].flags = info[1].flags = PASSFLAG_BYVAL;
		info[0].size = info[1].size = sizeof(void *);
		info[0].type = info[1].type = PassType_Basic;

		s_GetVelocity.call = g_pBinTools->CreateVCall(offset, 0, 0, NULL, info, 2);

		if (s_GetVelocity.call != NULL)
		{
			s_GetVelocity.supported = true;
		}
	}
	else
		m_sLog.LogToFileEx("[UKR COOP]::SetupGetVelocity()-> GetOffset return is false");

	s_GetVelocity.setup = true;

	return s_GetVelocity.supported;
}

void HL2::GetVelocity(CBaseEntity *pEntity, Vector *velocity, AngularImpulse *angvelocity)
{
	if(SetupGetVelocity())
	{
		unsigned char params[ptr_size * 3];
		unsigned char *vptr = params;
		*(CBaseEntity **)vptr = pEntity;
		vptr += sizeof(CBaseEntity *);
		*(Vector **)vptr = velocity;
		vptr += sizeof(Vector *);
		*(AngularImpulse **)vptr = angvelocity;

		s_GetVelocity.call->Execute(params, NULL);
	}
}

bool HL2::Translate(char *buffer, size_t maxlength, const char *format, unsigned int numparams, size_t *pOutLength, ...)
{
	va_list ap;
	unsigned int i;
	const char *fail_phrase;
	void *params[MAX_TRANSLATE_PARAMS];

	if (numparams > MAX_TRANSLATE_PARAMS)
	{
		assert(false);
		return false;
	}

	va_start(ap, pOutLength);
	for (i = 0; i < numparams; i++)
	{
		params[i] = va_arg(ap, void *);
	}
	va_end(ap);

	if (!ipharases->FormatString(buffer, maxlength, format, params, numparams, pOutLength, &fail_phrase))
	{
		if(fail_phrase != NULL)
		{
			g_pSM->LogError(myself, "[UkrCoop] Could not find core phrase: %s", fail_phrase);
		} else {
			g_pSM->LogError(myself, "[UkrCoop] Unknown fatal error while translating a core phrase.");
		}
		return false;
	}
	return true;
}

void HL2::PrintToConsole(int client, const char* msg, ...)
{
	char buffer[512];
	va_list ap;

	va_start(ap, msg);
	size_t len = vsnprintf(buffer, sizeof(buffer), msg, ap);
	va_end(ap);

	if(len >= sizeof(buffer) - 1)
	{
		buffer[sizeof(buffer) - 2] = '\n';
		buffer[sizeof(buffer) - 1] = '\0';
	} else {
		buffer[len++] = '\n';
		buffer[len] = '\0';
	}
	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
	if(pPlayer)
	{
		if(!pPlayer->IsConnected() && !pPlayer->IsInGame())
		{
			return;
		}
	}
	engine->ClientPrintf(pPlayer->GetEdict(), buffer);
}

void HL2::PrintToConsole(edict_t *m_pEndict, const char* msg, ...)
{
	char buffer[512];
	va_list ap;

	va_start(ap, msg);
	size_t len = vsnprintf(buffer, sizeof(buffer), msg, ap);
	va_end(ap);

	if(len >= sizeof(buffer) - 1)
	{
		buffer[sizeof(buffer) - 2] = '\n';
		buffer[sizeof(buffer) - 1] = '\0';
	} else {
		buffer[len++] = '\n';
		buffer[len] = '\0';
	}
	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(m_pEndict);
	if(pPlayer){
		if(!pPlayer->IsConnected() && !pPlayer->IsInGame())
		{
			return;
		}
	}
	engine->ClientPrintf(m_pEndict, buffer);
}

/*Макроси початок*/

#define FIND_PROP_DATA(td) \
	datamap_t *pMap; \
	if ((pMap = CBaseEntity_GetDataDescMap(pEntity)) == NULL) \
	{ \
		m_sLog.LogToFileEx("[UkrCoop] Could not retrieve datamap"); \
		return 0; \
	} \
	sm_datatable_info_t info; \
	if (!gamehelpers->FindDataMapInfo(pMap, prop, &info)) \
	{ \
		const char *class_name = gamehelpers->GetEntityClassname(pEntity); \
		m_sLog.LogToFileEx("[UkrCoop] Property \"%s\" not found (entity %d/%s)", prop, entity, ((class_name) ? class_name : "")); \
		return 0; \
	} \
	td = info.prop;


#define CHECK_SET_PROP_DATA_OFFSET() \
	if (element < 0 || element >= td->fieldSize) \
	{ \
		m_sLog.LogToFileEx("[UkrCoop] Element %d is out of bounds (Prop %s has %d elements).", element, prop, td->fieldSize); \
		return 0; \
	} \
	\
	offset = info.actual_offset + (element * (td->fieldSizeInBytes / td->fieldSize));


#define FIND_PROP_SEND(type, type_name) \
	sm_sendprop_info_t info;\
	SendProp *pProp; \
	IServerUnknown *pUnk = (IServerUnknown *)pEntity; \
	IServerNetworkable *pNet = pUnk->GetNetworkable(); \
	if (!pNet) \
	{ \
		m_sLog.LogToFileEx("[UkrCoop] Edict %d is not networkable", entity); \
		return 0; \
	} \
	if (!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), prop, &info)) \
	{ \
		const char *class_name = gamehelpers->GetEntityClassname(pEntity); \
		m_sLog.LogToFileEx("[UkrCoop] Property \"%s\" not found (entity %d/%s)", prop, entity, ((class_name) ? class_name : "")); \
		return 0; \
	} \
	\
	offset = info.actual_offset; \
	pProp = info.prop; \
	bit_count = pProp->m_nBits; \
	\
	switch (pProp->GetType()) \
	{ \
	case type: \
		{ \
			if (element != 0) \
			{ \
				m_sLog.LogToFileEx("[UkrCoop] SendProp %s is not an array. Element %d is invalid.", prop, element); \
				return 0; \
			} \
			break; \
		} \
	case DPT_DataTable: \
		{ \
			FIND_PROP_SEND_IN_SENDTABLE(info, pProp, element, type, type_name); \
			\
			offset += pProp->GetOffset(); \
			bit_count = pProp->m_nBits; \
			break; \
		} \
	default: \
		{ \
			m_sLog.LogToFileEx("[UkrCoop] SendProp %s type is not %s (%d != %d)", prop, type_name, pProp->GetType(), type); \
			return 0; \
		} \
	} \


#define FIND_PROP_SEND_IN_SENDTABLE(info, pProp, element, type, type_name) \
	SendTable *pTable = pProp->GetDataTable(); \
	if (!pTable) \
	{ \
		m_sLog.LogToFileEx("[UkrCoop] Error looking up DataTable for prop %s", prop); \
		return 0; \
	} \
	\
	int elementCount = pTable->GetNumProps(); \
	if (element < 0 || element >= elementCount) \
	{ \
		m_sLog.LogToFileEx("[UkrCoop] Element %d is out of bounds (Prop %s has %d elements).", element, prop, elementCount); \
		return 0; \
	} \
	\
	pProp = pTable->GetProp(element); \
	if (pProp->GetType() != type) \
	{ \
		m_sLog.LogToFileEx("[UkrCoop] SendProp %s type is not %s ([%d,%d] != %d)", prop, type_name, pProp->GetType(), pProp->m_nBits, type); \
		return 0; \
	}

/*мароси кінець*/


int HL2::GetEntProp(cell_t entity, PropTypes type, const char *prop, int size, int element)
{
	CBaseEntity *pEntity;
	int offset;
	edict_t *pEdict;
	int bit_count;
	bool is_unsigned = false;

	if (!IndexToAThings(entity, &pEntity, &pEdict))
	{
		m_sLog.LogToFileEx("[UkrCoop] Entity index (%d) is invalid.", entity);
		return 0;
	}

	switch (type)
	{
	case Prop_Datas:
		{
			typedescription_t *td;

			FIND_PROP_DATA(td);

			if ((bit_count = MatchFieldAsInteger(td->fieldType)) == 0)
			{
				m_sLog.LogToFileEx("[UkrCoop] Data field %s is not an integer (%d)", prop, td->fieldType);
				return 0;
			}

			CHECK_SET_PROP_DATA_OFFSET();

			break;
		}
	case Prop_Sends:
		{
			FIND_PROP_SEND(DPT_Int, "integer");

			is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);
			break;
		}
	default:
		{
			m_sLog.LogToFileEx("[UkrCoop] Invalid Property type %d", type);
			return 0;
		}
	}

	if (bit_count < 1)
	{
		bit_count = size * 8;
	}

	if (bit_count >= 17)
	{
		return *(int32_t *)((uint8_t *)pEntity + offset);
	}
	else if (bit_count >= 9)
	{
		if (is_unsigned)
		{
			return *(uint16_t *)((uint8_t *)pEntity + offset);
		}
		else
		{
			return *(int16_t *)((uint8_t *)pEntity + offset);
		}
	}
	else if (bit_count >= 2)
	{
		if (is_unsigned)
		{
			return *(uint8_t *)((uint8_t *)pEntity + offset);
		}
		else
		{
			return *(int8_t *)((uint8_t *)pEntity + offset);
		}
	}
	else
	{
		return *(bool *)((uint8_t *)pEntity + offset) ? 1 : 0;
	}

	return 0;
}

int HL2::SetEntProp(cell_t entity, PropTypes type, const char *prop, int value, int size, int element)
{
	CBaseEntity *pEntity;
	int offset;
	edict_t *pEdict;
	int bit_count;

	if (!IndexToAThings(entity, &pEntity, &pEdict))
	{
		m_sLog.LogToFileEx("[UkrCoop] Entity index (%d) is invalid", entity);
		return -1;
	}

	switch (type)
	{
	case Prop_Datas:
		{
			typedescription_t *td;

			FIND_PROP_DATA(td);

			if ((bit_count = MatchFieldAsInteger(td->fieldType)) == 0)
			{
				m_sLog.LogToFileEx("[UkrCoop] Data field %s is not an integer (%d)", prop, td->fieldType);
				return 0;
			}

			CHECK_SET_PROP_DATA_OFFSET();

			break;
		}
	case Prop_Sends:
		{
			FIND_PROP_SEND(DPT_Int, "integer");
			break;
		}
	default:
		{
			m_sLog.LogToFileEx("[UkrCoop] Invalid Property type %d", type);
			return -1;
		}
	}

	if (bit_count < 1)
	{
		bit_count = size * 8;
	}

	if (bit_count >= 17)
	{
		*(int32_t *)((uint8_t *)pEntity + offset) = value;
	}
	else if (bit_count >= 9)
	{
		*(int16_t *)((uint8_t *)pEntity + offset) = (int16_t)value;
	}
	else if (bit_count >= 2)
	{
		*(int8_t *)((uint8_t *)pEntity + offset) = (int8_t)value;
	}
	else
	{
		*(bool *)((uint8_t *)pEntity + offset) = value ? true : false;
	}
	
	if (type == Prop_Sends && (pEdict != NULL))
	{
		gamehelpers->SetEdictStateChanged(pEdict, offset);
	}

	return 1;
}

enum PropEntType
{
	PropEnt_Handle,
	PropEnt_Entity,
	PropEnt_Edict,
};

int HL2::GetEntPropEnt(cell_t entity, PropTypes type, const char * prop, int element)
{
	CBaseEntity *pEntity = nullptr;
	int offset;
	int bit_count;
	edict_t *pEdict = nullptr;
	PropEntType EntType;

	if (!IndexToAThings(entity, &pEntity, &pEdict))
	{
		m_sLog.LogToFileEx("[UkrCoop] Entity index (%d) is invalid.", entity);
		return -1;
	}

	switch (type)
	{
		case Prop_Datas:
		{
			typedescription_t *td = nullptr;

			FIND_PROP_DATA(td);

			switch (td->fieldType)
			{
				case FIELD_EHANDLE:
					EntType = PropEnt_Handle;
					break;
				case FIELD_CLASSPTR:
					EntType = PropEnt_Entity;
					break;
				case FIELD_EDICT:
					EntType = PropEnt_Edict;
				default:
					m_sLog.LogToFileEx("Data field %s is not an entity nor edict (%d)", prop, td->fieldType);
					return -1;
			}

			CHECK_SET_PROP_DATA_OFFSET();

			break;
		}
		case Prop_Sends:
		{
			EntType = PropEnt_Handle;

			FIND_PROP_SEND(DPT_Int, "integer");

			break;
		}
		default:
			m_sLog.LogToFileEx("Invalid Property type %d", type);
			return -1;
	}

	switch (EntType)
	{
		case PropEnt_Handle:
		{
			CBaseHandle &hndl = *(CBaseHandle*)((uint8_t *)pEntity + offset);
			CBaseEntity *pHandleEntity = gamehelpers->ReferenceToEntity(hndl.GetEntryIndex());

			if (!pHandleEntity || hndl != reinterpret_cast<IHandleEntity *>(pHandleEntity)->GetRefEHandle())
				return -1;

			return gamehelpers->EntityToBCompatRef(pHandleEntity);
		}
		case PropEnt_Entity:
		{
			CBaseEntity *pPropEntity = *(CBaseEntity**)((uint8_t *)pEntity + offset);
			return gamehelpers->EntityToBCompatRef(pPropEntity);
		}
		case PropEnt_Edict:
		{
			edict_t *pEdict2 = *(edict_t **)((uint8_t*)pEntity + offset);
			if (!pEdict2 || pEdict2->IsFree())
				return -1;

			return gamehelpers->IndexOfEdict(pEdict2);
		}
	}

	return -1;
}

int HL2::SetEntPropEnt(cell_t entity, PropTypes type, const char * prop, cell_t other, int element)
{
	CBaseEntity *pEntity = nullptr;
	int offset;
	int bit_count;
	edict_t *pEdict = nullptr;
	PropEntType EntType;

	if (!IndexToAThings(entity, &pEntity, &pEdict))
	{
		m_sLog.LogToFileEx("[UkrCoop] Entity index (%d) is invalid.", entity);
		return -1;
	}

	switch (type)
	{
	case Prop_Sends: {
		EntType = PropEnt_Handle;
		FIND_PROP_SEND(DPT_Int, "integer");
		break;
	}
	case Prop_Datas:{
		typedescription_t *td = nullptr;

		FIND_PROP_DATA(td);

		switch (td->fieldType)
		{
			case FIELD_EHANDLE:
				EntType = PropEnt_Handle;
				break;
			case FIELD_CLASSPTR:
				EntType = PropEnt_Entity;
				break;
			case FIELD_EDICT:
				EntType = PropEnt_Edict;
				if (!pEdict) {
					m_sLog.LogToFileEx("Edict %d is invalid", entity);
					return -1;
				}
				break;
			default:
			{
				m_sLog.LogToFileEx("Data field %s is not an entity nor edict (%d)", prop, td->fieldType);
				return -1;
			}
		}
		CHECK_SET_PROP_DATA_OFFSET();
		break;
	}
	default:
		m_sLog.LogToFileEx("Invalid Property type %d", type);
		return -1;
	}
	
	CBaseEntity *pOther = nullptr;
	if (!g_HL2.IndexToAThings(other, &pOther, NULL))
	{
		m_sLog.LogToFileEx("Entity %d is invalid.(1)", other);
		return -1;
	}

	if (!pOther && other != -1)
	{
		m_sLog.LogToFileEx("Entity %d is invalid.(2)", other);
		return -1;
	}

	switch (EntType)
	{
		case PropEnt_Handle:
		{
			CBaseHandle &hndl = *(CBaseHandle *)((uint8_t*)pEntity + offset);
			hndl.Set((IHandleEntity*)pOther);

			if (type == Prop_Sends && (pEdict != nullptr))
				gamehelpers->SetEdictStateChanged(pEdict, offset);

			break;
		}
		case PropEnt_Entity:
		{
			*(CBaseEntity **)((uint8_t*)pEntity + offset) = pOther;
			break;
		}
		case PropEnt_Edict:
		{
			edict_t *pOtherEdict = nullptr;
			if (pOther)
			{
				IServerNetworkable *pNetworkable = ((IServerUnknown*)pOther)->GetNetworkable();
				if (!pNetworkable)
				{
					m_sLog.LogToFileEx("Entity %d does not have a valid edict", other);
					return -1;
				}

				pOtherEdict = pNetworkable->GetEdict();
				if (!pOtherEdict || pOtherEdict->IsFree())
				{
					m_sLog.LogToFileEx("Entity %d does not have a valid edict", other);
					return -1;
				}

				*(edict_t **)((uint8_t*)pEntity + offset) = pOtherEdict;
				break;
			}
		}
	}

	return 1;
}

bool HL2::IndexToAThings(cell_t num, CBaseEntity **pEntData, edict_t **pEdictData)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(num);

	if (!pEntity)
	{
		return false;
	}

	int index = gamehelpers->ReferenceToIndex(num);
	if (index > 0 && index <= playerhelpers->GetMaxClients())
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(index);
		if (!pPlayer || !pPlayer->IsConnected())
		{
			return false;
		}
	}

	if (pEntData)
	{
		*pEntData = pEntity;
	}

	if (pEdictData)
	{
		edict_t *pEdict = BaseEntityToEdict(pEntity);
		if (!pEdict || pEdict->IsFree())
		{
			pEdict = NULL;
		}

		*pEdictData = pEdict;
	}

	return true;
}
