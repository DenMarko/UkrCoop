#include "CLuaPropProxy.h"
#include "HL2.h"
#include "CTrace.h"
#include "Interface/ITerrorPlayer.h"
#include "CTempEntity.h"

#define PROP_DATA_STATIC_BEGIN(PropName) \
	static unsigned int offset = 0; \
	if(offset == 0) \
	{ \
		datamap_t *pMap; \
		if((pMap = GetVirtualClass<ITerrorPlayer>((CBaseEntity *)pThisPtr)->GetDataDescMap()) != nullptr) \
		{ \
			sm_datatable_info_t info; \
			if(gamehelpers->FindDataMapInfo(pMap, PropName, &info)) \
			{ \
				offset = info.actual_offset; \
			} \
		} \
	}


#define PROP_DATA_BEGIN(PropName) \
	datamap_t *pMap; \
	if((pMap = GetVirtualClass<ITerrorPlayer>((CBaseEntity *)pThisPtr)->GetDataDescMap()) != nullptr) \
	{ \
		sm_datatable_info_t info; \
		if(gamehelpers->FindDataMapInfo(pMap, PropName, &info)) \
		{ \
			unsigned int offset = 0; \
			offset = info.actual_offset;

#define PROP_DATA_END() \
		} \
	}


int GetClientAimTarget(CBaseEntity *pThisPtr, bool only_player)
{
	int id = gamehelpers->EntityToBCompatRef(pThisPtr);
	if(id == 0)
	{
		return -1;
	}

	ITerrorPlayer *pPlayer = GetVirtualClass<ITerrorPlayer>(id);

	Vector eye_pos = pPlayer->EarPosition();
	QAngle eye_ang = pPlayer->EyeAngles();

	Vector aim_dir;
	AngleVectors(eye_ang, &aim_dir);
	VectorNormalize(aim_dir);

	Ray_t ray;
	trace_t tr;

	ray.Init(eye_pos, eye_pos + aim_dir * 8000);
	util_TraceRay(ray, MASK_L4D_VISIBLE, pPlayer->GetNetworkable()->GetEntityHandle(), COLLISION_GROUP_NONE, &tr);

	if(tr.fraction == 1.f || tr.m_pEnt == NULL)
	{
		return -1;
	}

	int ent_ref = gamehelpers->EntityToBCompatRef(tr.m_pEnt);

	int ent_index = gamehelpers->ReferenceToIndex(ent_ref);
	IGamePlayer *pTargetPlayer = playerhelpers->GetGamePlayer(ent_index);
	if (pTargetPlayer != NULL && !pTargetPlayer->IsInGame())
	{
		return -1;
	}
	else if (only_player && pTargetPlayer == NULL)
	{
		return -1;
	}

	return ent_ref;
}

const char *GetNameClass(CBaseEntity* pThisPtr)
{
	PROP_DATA_BEGIN("m_iClassname")

	if(offset)
	{
		return access_member<const char*>(pThisPtr, offset);
	}

	PROP_DATA_END()
	return "NULL";
}

Vector *Prop_get_vecViewOffset(CBaseEntity *pThisPtr)
{
	PROP_DATA_BEGIN("m_vecViewOffset")
	
	if(offset)
	{
		return (Vector *)(((unsigned char*)pThisPtr) + offset);
	}

	PROP_DATA_END()
	return nullptr;
}

string_t Prop_get_target(CBaseEntity* pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_target")

	if(offset)
	{
		return access_member<string_t>(pThisPtr, offset);
	}
	return castable_string_t();
}

float GetPoseParameter(CBaseEntity* pThisPtr, int element)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flPoseParameter");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			switch(pProp->GetType())
			{
				case DPT_Float:
				{
					if(element != 0)
						return 0.0f;
					break;
				}
				case DPT_DataTable:
				{
					SendTable *pTable = pProp->GetDataTable();
					if(!pTable)
						return 0.0f;

					int elementCount = pTable->GetNumProps();
					if(element < 0 || element >= elementCount)
						return 0.0f;
					
					pProp = pTable->GetProp(element);
					if(pProp->GetType() != DPT_Float)
						return 0.0f;

					offset += pProp->GetOffset();
					break;
				}
				default:
				{
					return 0.0f;
				}
			}
			return access_member<float>(pThisPtr, offset);
		}
	}
	return 0.0f;
}

void SetPoseParameter(CBaseEntity* pThisPtr, float val, int element)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flPoseParameter");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			switch(pProp->GetType())
			{
				case DPT_Float:
				{
					if(element != 0)
						return;

					break;
				}
				case DPT_DataTable:
				{
					SendTable *pTable = pProp->GetDataTable();
					if(!pTable)
						return;

					int elementCount = pTable->GetNumProps();
					if(element < 0 || element >= elementCount)
						return;

					pProp = pTable->GetProp(element);
					if(pProp->GetType() != DPT_Float)
						return;

					offset += pProp->GetOffset();
					break;
				}
				default:
				{
					return;
				}
			}
			access_member<float>(pThisPtr, offset) = val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

int Prop_get_spawnflags(const CBaseEntity *pThisPtr)
{
	return reinterpret_cast<const IBaseEntity*>(pThisPtr)->GetSpawnFlags();
}

bool Prop_get_bSequenceLoops(const CBaseEntity* pThisPtr)
{
	PROP_DATA_BEGIN("m_bSequenceLoops")

	if(offset)
	{
		return access_member<bool>((CBaseEntity *)pThisPtr, offset);
	}

	PROP_DATA_END()
	return false;
}

void Prop_set_MaxHealth(CBaseEntity *pThisPtr, int val)
{
	PROP_DATA_BEGIN("m_iMaxHealth")

	if(offset) {
		access_member<int>((CBaseEntity*)pThisPtr, offset) = val;
	}

	PROP_DATA_END()
}

int Prop_get_MaxHealth(const CBaseEntity *pThisPtr)
{
	PROP_DATA_BEGIN("m_iMaxHealth")

	if(offset)
	{
		return access_member<int>((CBaseEntity*)pThisPtr, offset);
	}

	PROP_DATA_END()
    return 0;
}

int Prop_get_Health(const CBaseEntity *pThisPtr)
{
	PROP_DATA_BEGIN("m_iHealth")
	if(offset)
	{
		return access_member<int>((CBaseEntity*)pThisPtr, offset);
	}
	PROP_DATA_END()
	return 0;
}

void Prop_set_Health(CBaseEntity *pThisPtr, int val)
{
	PROP_DATA_BEGIN("m_iHealth")
	if(offset)
	{
		access_member<int>((CBaseEntity*)pThisPtr, offset) = val;
	}
	PROP_DATA_END()
}

CBaseEntity *Prop_get_hMoveParent(const CBaseEntity *pThisPtr)
{
	IBaseEntity *IBase = (IBaseEntity *)pThisPtr;
	SendProp *pProp = gamehelpers->FindInSendTable(IBase->GetServerClass()->GetName(), "m_hMoveParent");
	if(pProp)
	{
		uint32_t offset = pProp->GetOffset();
		if(offset)
		{
			CBaseHandle &hndl = access_member<CBaseHandle>((CBaseEntity*)pThisPtr, offset);
			IBaseEntity *pHandleEntity = GetVirtualClass<IBaseEntity>(hndl.GetEntryIndex());

			if (!pHandleEntity || hndl != pHandleEntity->GetRefEHandle())
			{
				return nullptr;
			}
			return pHandleEntity->GetBaseEntity();
		}
	}
	return nullptr;
}

int Prop_get_fFlags(const CBaseEntity* pThisPtr)
{
	static unsigned int offset = 0;
	if(offset == 0)
	{
		IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
		auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_fFlags");
		if(pProp)
		{
			offset = pProp->GetOffset();
		}
	}
	if(offset)
	{
		return access_member<int32>((CBaseEntity*)pThisPtr, offset);
	}
	return -1;
}

char Prop_get_takedamage(const CBaseEntity *pThisPtr)
{
	PROP_DATA_BEGIN("m_takedamage")
	if(offset)
	{
		return access_member<char>((CBaseEntity*)pThisPtr, offset);
	}
	PROP_DATA_END()
	return 0;
}

unsigned char Prop_get_nWaterLevel(const CBaseEntity *pThisPtr)
{
	PROP_DATA_BEGIN("m_nWaterLevel")
	if(offset)
	{
		return access_member<unsigned char>((CBaseEntity*)pThisPtr, offset);
	}
	PROP_DATA_END()
	return 0;
}

int Prop_get_zombieClass(const CBaseEntity *pThisPtr)
{
	IBaseEntity *pEnt = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pEnt->GetServerClass()->GetName(), "m_zombieClass");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<int32>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return -1;
}

int Prop_get_bLocked(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_bLocked");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<bool>((CBaseEntity*)pThisPtr, offset) ? 1 : 0;
		}
	}
	return -1;
}

void Prop_set_bLocked(CBaseEntity *pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_bLocked");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<bool>(pThisPtr, offset) = val >= 1 ? true : false;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
			{
				gamehelpers->SetEdictStateChanged(gEdict, offset);
			}
		}
	}
}

int Prop_get_eDoorState(const CBaseEntity *pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_eDoorState");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<uint8_t>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return -1;
}

void Prop_set_eDoorState(CBaseEntity* pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_eDoorState");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<uint8_t>(pThisPtr, offset) = val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
			{
				gamehelpers->SetEdictStateChanged(gEdict, offset);
			}
		}
	}
}

CBaseEntity *Prop_get_customAbility(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	SendProp* pProp = nullptr;
	if((pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_customAbility")) != nullptr)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			CBaseHandle &hndl = access_member<CBaseHandle>((CBaseEntity*)pThisPtr, offset);;
			IBaseEntity *pHandleEntity = GetVirtualClass<IBaseEntity>(hndl.GetEntryIndex());

			if (!pHandleEntity || hndl != pHandleEntity->GetRefEHandle())
			{
				return nullptr;
			}
			return (CBaseEntity *)pHandleEntity;
		}
	}
	return nullptr;
}

int Prop_get_isIncapacitated(const CBaseEntity *pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_isIncapacitated");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<bool>((CBaseEntity*)pThisPtr, offset) ? 1 : 0;
		}
	}
	return -1;
}

void Prop_set_isIncapacitated(CBaseEntity *pThisPtr, int Val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_isIncapacitated");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<bool>(pThisPtr, offset) = Val >= 1 ? true : false;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

Vector *Prop_get_vecAbsOrigin(CBaseEntity *pThisPtr)
{
	Vector *vecAbs = nullptr;

	PROP_DATA_BEGIN("m_vecAbsOrigin")

	if(offset)
	{
		vecAbs = (Vector*)((uint8_t *)pThisPtr + offset);
	}

	PROP_DATA_END()
	return vecAbs;
}

QAngle *Prop_get_angAbsRotation(CBaseEntity *pThisPtr)
{
	QAngle *vecAbsAng = nullptr;
	PROP_DATA_BEGIN("m_angAbsRotation")

	if(offset != 0)
	{
		vecAbsAng = (QAngle*)((uint8_t *)pThisPtr + offset);
	}

	PROP_DATA_END()
	return vecAbsAng;
}

Vector Prop_get_vecOrigin(const CBaseEntity* pThisPtr)
{
	Vector vec;
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_vecOrigin");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
			vec = access_member<Vector>((CBaseEntity*)pThisPtr, offset);
	}
	return vec;
}

QAngle* Prop_get_angRotation(const CBaseEntity* pThisPtr)
{
	QAngle *vec = nullptr;
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_angRotation");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			vec = (QAngle*)((uint8_t *)pThisPtr + offset);
		}
	}
	return vec;
}

float Prop_get_flCycle(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flCycle");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<float>((CBaseEntity*)pThisPtr, offset);
		}
	}

	return 0.0f;
}

int Prop_get_nSequence(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_nSequence");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return (int)access_member<int16_t>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return 0;
}

int Prop_get_nRenderFX(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_nRenderFX");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return (int)access_member<int8_t>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return 0;
}

int Prop_get_nRenderMode(const CBaseEntity* pThisPtr)
{
	static unsigned int offset = 0;
	if(offset == 0)
	{
		IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
		auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_nRenderMode");
		if(pProp)
		{
			offset = pProp->GetOffset();
		}
	}

	if(offset)
	{
		return access_member<uint8_t>((CBaseEntity*)pThisPtr, offset);
	}
	return -1;
}

void Prop_set_nRenderMode(CBaseEntity* pThisPtr, int nRenderMode)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_nRenderMode");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<uint8_t>(pThisPtr, offset) = nRenderMode;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);

		}
	}
}

float Prop_get_flAnimTime(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flAnimTime");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return (float)access_member<uint8_t>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return -1.f;
}

float Prop_get_flSimulationTime(const CBaseEntity *pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_flSimulationTime")
	if(offset)
	{
		return access_member<float>((CBaseEntity*)pThisPtr, offset);
	}
	return -1.f;
}

void Prop_set_flSimulationTime(CBaseEntity *pThisPtr, float val)
{
	PROP_DATA_STATIC_BEGIN("m_flSimulationTime")
	if(offset)
	{
		access_member<float>(pThisPtr, offset) = val;
	}
}

bool Prop_get_mobRush(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_mobRush");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<bool>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return false;
}

CBaseEntity* Prop_get_hGroundEntity(const CBaseEntity* pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_hGroundEntity");
	if(offset)
	{
		CBaseHandle &hndl = access_member<CBaseHandle>((CBaseEntity*)pThisPtr, offset);
		IBaseEntity *pHandleEntity = GetVirtualClass<IBaseEntity>(hndl.GetEntryIndex());

		if (!pHandleEntity || hndl != pHandleEntity->GetRefEHandle())
		{
			return nullptr;
		}
		return pHandleEntity->GetBaseEntity();
	}
	return nullptr;
}

CBaseEntity *Prop_get_hActiveWeapon(CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	SendProp* pProp = nullptr;
	if((pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_hActiveWeapon")) != nullptr)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			CBaseHandle &hndl = access_member<CBaseHandle>((CBaseEntity*)pThisPtr, offset);
			IBaseEntity *pHandleEntity = GetVirtualClass<IBaseEntity>(hndl.GetEntryIndex());

			if (!pHandleEntity || hndl != pHandleEntity->GetRefEHandle())
			{
				return nullptr;
			}
			return pHandleEntity->GetBaseEntity();
		}
	}
	return nullptr;
}

CBaseEntity *Prop_get_hEffectEntity(const CBaseEntity *pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_hEffectEntity")
	if(offset)
	{
		CBaseHandle &hndl = access_member<CBaseHandle>((CBaseEntity*)pThisPtr, offset);
		IBaseEntity *pHandleEntity = GetVirtualClass<IBaseEntity>(hndl.GetEntryIndex());

		if (!pHandleEntity || hndl != pHandleEntity->GetRefEHandle())
		{
			return nullptr;
		}
		return pHandleEntity->GetBaseEntity();
	}
	return nullptr;
}

float Prop_get_fGravity(const CBaseEntity* pThisPtr)
{
	PROP_DATA_BEGIN("m_flGravity")
	if(offset)
	{
		return access_member<float>((CBaseEntity*)pThisPtr, offset);
	}
	PROP_DATA_END()
	return -1.f;
}

IPhysicsObject *Prop_get_PhysicsObject(const CBaseEntity *pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_pPhysicsObject")

	if(offset)
	{
		return access_member<IPhysicsObject *>((CBaseEntity*)pThisPtr, offset);
	}
	return nullptr;
}

int Prop_get_fEffects(const CBaseEntity *pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_fEffects")
	if(offset)
	{
		return access_member<int>((CBaseEntity*)pThisPtr, offset);
	}
	return -1;
}

void Prop_set_fEffects(CBaseEntity *pThisPtr, int val)
{
	PROP_DATA_STATIC_BEGIN("m_fEffects")
	if(offset)
	{
		access_member<int>(pThisPtr, offset) = val;
	}
}

int Prop_get_EFlags(const CBaseEntity* pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_iEFlags")

	if(offset)
	{
		return access_member<int>((CBaseEntity*)pThisPtr, offset);
	}
	return -1;
}

void Prop_set_EFlags(CBaseEntity* pThisPtr, int nEFlags)
{
	PROP_DATA_STATIC_BEGIN("m_iEFlags")

	if(offset)
	{
		access_member<int>(pThisPtr, offset) = nEFlags;
	}
}

Vector *Prop_get_vecVelocity(const CBaseEntity *pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_vecVelocity")
	if(offset)
	{
		return (Vector *)((uint8_t *)pThisPtr + offset);
	}
	return nullptr;
}

void Prop_set_vecVelocity(CBaseEntity *pThisPtr, Vector* vel)
{
	PROP_DATA_STATIC_BEGIN("m_vecVelocity")
	if(offset)
	{
		Vector &v = access_member<Vector>(pThisPtr, offset);
		v = *vel;
	}
}

Vector *Prop_get_vecBaseVelocity(const CBaseEntity* pThisPtr)
{
	PROP_DATA_BEGIN("m_vecBaseVelocity")
	if(offset)
	{
		return (Vector *)((uint8_t *)pThisPtr + offset);
	}
	PROP_DATA_END()
	return nullptr;
}

void Prop_set_fGravity(CBaseEntity* pThisPtr, float g)
{
	PROP_DATA_BEGIN("m_flGravity")
	if(offset)
	{
		access_member<float>(pThisPtr, offset) = g;
	}
	PROP_DATA_END()
}

CBaseEntity *Prop_get_hOwnerEntity(const CBaseEntity* pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_hOwnerEntity")
	if(offset)
	{
		CBaseHandle &hndl = access_member<CBaseHandle>((CBaseEntity*)pThisPtr, offset);
		IBaseEntity *pHandleEntity = GetVirtualClass<IBaseEntity>(hndl.GetEntryIndex());

		if (!pHandleEntity || hndl != pHandleEntity->GetRefEHandle())
		{
			return nullptr;
		}
		return pHandleEntity->GetBaseEntity();
	}
	return nullptr;
}

float Prop_get_rage(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_rage");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<float>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return 0.0f;
}

float Prop_get_flPlaybackRate(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flPlaybackRate");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<float>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return 0.0f;
}

void Prop_set_spawnflags(CBaseEntity *pThisPtr, int flags)
{
	PROP_DATA_BEGIN("m_spawnflags")

	if(offset)
	{
		int &iflags = access_member<int>(pThisPtr, offset);
		if((iflags & flags) != flags)
		{
			iflags = flags;
		}
	}
	PROP_DATA_END()
}

void Prop_set_fFlags(CBaseEntity* pThisPtr, int flag)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_fFlags");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<int32>(pThisPtr, offset) = flag;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_vecBaseVelocity(CBaseEntity* pThisPtr, Vector &vecBaseVelocity)
{
	PROP_DATA_BEGIN("m_vecBaseVelocity")

	if(offset)
	{
		Vector &v = access_member<Vector>(pThisPtr, offset);
		v = vecBaseVelocity;
	}

	PROP_DATA_END()
}

void Prop_set_vecOrigin(CBaseEntity* pThisPtr, Vector* vec)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_vecOrigin");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			Vector &v = access_member<Vector>(pThisPtr, offset);
			v = *vec;
			
			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void PropData_set_vecOrigin(CBaseEntity* pThisPtr, Vector* vec)
{
	PROP_DATA_STATIC_BEGIN("m_vecOrigin")

	if(offset)
	{
		Vector &v = access_member<Vector>(pThisPtr, offset);
		v = *vec;
	}
}

void PropData_set_angRotation(CBaseEntity* pThisPtr, QAngle* ang)
{
	PROP_DATA_STATIC_BEGIN("m_angRotation")
	if(offset)
	{
		QAngle &vec = access_member<QAngle>(pThisPtr, offset);
		vec = *ang;
	}
}

void Prop_set_angRotation(CBaseEntity* pThisPtr, QAngle* ang)
{
	static unsigned int offset = 0;
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	if(offset == 0)
	{	
		auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_angRotation");
		if(pProp)
		{
			offset = pProp->GetOffset();
		}
	}

	if(offset)
	{
		QAngle &vecAngel = access_member<QAngle>(pThisPtr, offset);
		vecAngel = *ang;

		auto gEdict = pUnk->GetNetworkable()->GetEdict();
		if(gEdict != nullptr)
			gamehelpers->SetEdictStateChanged(gEdict, offset);
	}
}

void Prop_set_flCycle(CBaseEntity* pThisPtr, float val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flCycle");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<float>(pThisPtr, offset) = val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_nSequence(CBaseEntity* pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_nSequence");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<int16_t>(pThisPtr, offset) = (int16_t)val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_flAnimTime(CBaseEntity* pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flAnimTime");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<int8_t>(pThisPtr, offset) = (int8_t)val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_nRenderFX(CBaseEntity* pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_nRenderFX");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<int8_t>(pThisPtr, offset) = (int8_t)val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_mobRush(CBaseEntity* pThisPtr, bool val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_mobRush");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<bool>(pThisPtr, offset) = val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_hGroundEntity(CBaseEntity* pThisPtr, CBaseEntity* val)
{
	PROP_DATA_BEGIN("m_hGroundEntity")

	if(offset)
	{
		CBaseHandle &hndl = access_member<CBaseHandle>(pThisPtr, offset);
		hndl.Set((IHandleEntity*)val);
	}

	PROP_DATA_END()
}

int Prop_get_iPrimaryAmmoType(CBaseEntity* pThisPtr)
{
	PROP_DATA_BEGIN("m_iPrimaryAmmoType")
	if(offset)
	{
		return access_member<int>(pThisPtr, offset);
	}
	PROP_DATA_END()
	return -1;
}

void Prop_set_iPrimaryAmmoType(CBaseEntity* pThisPtr, int val)
{
	PROP_DATA_BEGIN("m_iPrimaryAmmoType")
	if(offset)
	{
		access_member<int>(pThisPtr, offset) = val;
	}
	PROP_DATA_END()
}

int Prop_get_iSecondaryAmmoType(CBaseEntity* pThisPtr)
{
	PROP_DATA_BEGIN("m_iSecondaryAmmoType")

	if(offset)
	{
		return access_member<int>(pThisPtr, offset);
	}

	PROP_DATA_END()
	return -1;
}

void Prop_set_iSecondaryAmmoType(CBaseEntity* pThisPtr, int val)
{
	PROP_DATA_BEGIN("m_iSecondaryAmmoType")
	if(offset)
	{
		access_member<int>(pThisPtr, offset) = val;
	}
	PROP_DATA_END()
}

void Prop_set_hEffectEntity(CBaseEntity* pThisPtr, CBaseEntity* ent)
{
	PROP_DATA_STATIC_BEGIN("m_hEffectEntity")
	if(offset)
	{
		CBaseHandle &hndl = access_member<CBaseHandle>(pThisPtr, offset);
		hndl.Set((IHandleEntity*)ent);
	}
}

void Prop_set_hActiveWeapon(CBaseEntity* pThisPtr, CBaseEntity* val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_hActiveWeapon");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			CBaseHandle &hndl = access_member<CBaseHandle>(pThisPtr, offset);
			hndl.Set((IHandleEntity*)val);
		}
	}
}

void Prop_set_hOwnerEntity(CBaseEntity* pThisPtr, CBaseEntity *val)
{
	PROP_DATA_STATIC_BEGIN("m_hOwnerEntity")

	if(offset)
	{
		CBaseHandle &hndl = access_member<CBaseHandle>(pThisPtr, offset);
		hndl.Set((IHandleEntity*)val);
	}
}

void Prop_set_rage(CBaseEntity* pThisPtr, float val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_rage");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<float>(pThisPtr, offset) = val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_flPlaybackRate(CBaseEntity* pThisPtr, float val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_flPlaybackRate");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<float>(pThisPtr, offset) = val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

void Prop_set_CollisionGroup(CBaseEntity* pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_CollisionGroup");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			access_member<uint8_t>(pThisPtr, offset) = (uint8_t)val;

			auto gEdict = pUnk->GetNetworkable()->GetEdict();
			if(gEdict != nullptr)
				gamehelpers->SetEdictStateChanged(gEdict, offset);
		}
	}
}

int Prop_get_CollisionGroup(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_CollisionGroup");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<uint8_t>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return 0;
}

int Prop_get_SolidFlags(const CBaseEntity* pThisPtr)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_usSolidFlags");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			return access_member<uint16_t>((CBaseEntity*)pThisPtr, offset);
		}
	}
	return -1;
}

void Prop_set_SolidFlags(CBaseEntity* pThisPtr, int val)
{
	IBaseEntity *pUnk = (IBaseEntity *)pThisPtr;
	auto pProp = gamehelpers->FindInSendTable(pUnk->GetServerClass()->GetName(), "m_usSolidFlags");
	if(pProp)
	{
		unsigned int offset = pProp->GetOffset();
		if(offset)
		{
			uint16_t &oldFlags = access_member<uint16_t>(pThisPtr, offset);
			if(oldFlags != (uint16_t)val)
			{
				oldFlags = (uint16_t)val;

				auto gEdict = pUnk->GetNetworkable()->GetEdict();
				if(gEdict != nullptr)
					gamehelpers->SetEdictStateChanged(gEdict, offset);
			}
		}
	}
}

int Prop_get_MoveType(const CBaseEntity* pThisPtr)
{
	PROP_DATA_STATIC_BEGIN("m_MoveType")
	if(offset)
	{
		return access_member<uint8_t>((CBaseEntity*)pThisPtr, offset);
	}
	return -1;
}

void Prop_set_MoveType(CBaseEntity* pThisPtr, int val)
{
	PROP_DATA_STATIC_BEGIN("m_MoveType")
	if(offset)
	{
		access_member<uint8_t>(pThisPtr, offset) = (uint8_t)val;
	}
}

bool IsIdentical(const Vector* pThis, const Vector* src)
{
	return (src->x == pThis->x) && (src->y == pThis->y) && (src->z == pThis->z);
}

bool IsNotIdentical(const Vector* pThis, const Vector* src)
{
	return (src->x != pThis->x) || (src->y != pThis->y) || (src->z != pThis->z);
}

bool IsIdentical(const QAngle* pThis, const QAngle* src)
{
	return (src->x == pThis->x) && (src->y == pThis->y) && (src->z == pThis->z);
}

bool IsNotIdentical(const QAngle* pThis, const QAngle* src)
{
	return (src->x != pThis->x) || (src->y != pThis->y) || (src->z != pThis->z);
}
