#include "CTempEntity.h"
#include "HL2.h"
#include "CUserMessage.h"

TempEntityManager g_TEManager;

short g_ModelIndexLaserBeam = -1;
short g_ModelIndexSmoke = -1;

int PrecacheModels(const char *szName, bool isPreload = true)
{
	if(!szName, !*szName)
	{
		return -1;
	}
	return engine->PrecacheModel(szName, isPreload);
}

void InitPrecache()
{
    g_ModelIndexLaserBeam = PrecacheModels("materials/sprites/laserbeam.vmt");
	g_ModelIndexSmoke = PrecacheModels("sprites/steam1.vmt");
	PrecacheModels("sprites/blueglow1.vmt");
	PrecacheModels("particle/particle_smoker.vmt");
}

/*************************
*                        *
* Temp Entities Wrappers *
*                        *
**************************/

TempEntityInfo::TempEntityInfo(IBaseTempEntity *me) : m_Me(me) { }

const char *TempEntityInfo::GetName()
{
	return m_Me->GetName();
}

ServerClass *TempEntityInfo::GetServerClass()
{
	return m_Me->GetServerClass();
}

bool TempEntityInfo::IsValidProp(const char *name)
{
	return (gamehelpers->FindInSendTable(GetServerClass()->GetName(), name)) ? true : false;
}

int TempEntityInfo::_FindOffset(const char *name, int *size)
{
	int offset;

	sm_sendprop_info_t info;
	if (!gamehelpers->FindSendPropInfo(GetServerClass()->GetName(), name, &info))
	{
		return -1;
	}

	offset = info.actual_offset;
	if (size)
	{
		*size = info.prop->m_nBits;
	}

	return offset;
}

bool TempEntityInfo::TE_SetEntData(const char *name, int value)
{
	int size;
	int offset = _FindOffset(name, &size);

	if (offset < 0)
	{
		return false;
	}

	if (size <= 8) {
		*((uint8_t *)m_Me + offset) = value;
	} else if (size <= 16) {
		*(short *)((uint8_t *)m_Me + offset) = value;
	} else if (size <= 32) {
		*(int *)((uint8_t *)m_Me + offset) = value;
	} else {
		return false;
	}

	return true;
}

bool TempEntityInfo::TE_GetEntData(const char *name, int *value)
{
	int size;
	int offset = _FindOffset(name, &size);

	if (offset < 0)
	{
		return false;
	}

	if (size <= 8) {
		*value = *((uint8_t *)m_Me + offset);
	} else if (size <= 16) {
		*value = *(short *)((uint8_t *)m_Me + offset);
	} else if (size <= 32) {
		*value = *(int *)((uint8_t *)m_Me + offset);
	} else {
		return false;
	}

	return true;
}

bool TempEntityInfo::TE_SetEntDataFloat(const char *name, float value)
{
	int offset = _FindOffset(name);

	if (offset < 0)
	{
		return false;
	}

	*(float *)((uint8_t *)m_Me + offset) = value;

	return true;
}

bool TempEntityInfo::TE_GetEntDataFloat(const char *name, float *value)
{
	int offset = _FindOffset(name);

	if (offset < 0)
	{
		return false;
	}

	*value = *(float *)((uint8_t *)m_Me + offset);

	return true;
}

bool TempEntityInfo::TE_SetEntDataVector(const char *name, Vector vector)
{
	int offset = _FindOffset(name);

	if (offset < 0)
	{
		return false;
	}

	Vector *v = (Vector *)((uint8_t *)m_Me + offset);
	v->x = vector.x;
	v->y = vector.y;
	v->z = vector.z;

	return true;
}

bool TempEntityInfo::TE_GetEntDataVector(const char *name, Vector *vector)
{
	int offset = _FindOffset(name);

	if (offset < 0)
	{
		return false;
	}

	Vector *v = (Vector *)((uint8_t *)m_Me + offset);
	vector->x = v->x;
	vector->y = v->y;
	vector->z = v->z;

	return true;
}

bool TempEntityInfo::TE_SetEntDataFloatArray(const char *name, cell_t *array, int size)
{
	int offset = _FindOffset(name);

	if (offset < 0)
	{
		return false;
	}

	float *base = (float *)((uint8_t *)m_Me + offset);
	for (int i=0; i<size; i++)
	{
		base[i] = sp_ctof(array[i]);
	}

	return true;
}

void TempEntityInfo::Send(IRecipientFilter &filter, float delay)
{
	engine->PlaybackTempEntity(filter, delay, (void *)m_Me, GetServerClass()->m_pTable, GetServerClass()->m_ClassID);
}

/**********************
*                     *
* Temp Entity Manager *
*                     *
***********************/

void TempEntityManager::Initialize()
{
	void *addr;
	int offset;
	m_Loaded = false;

    if (g_pGameConf->GetMemSig("TempEntities", &addr) && addr)
    {
        m_ListHead = *(IBaseTempEntity **) addr;
        m_TempEntInfo = adtfactory->CreateBasicTrie();
        m_Loaded = true;
    }
}

bool TempEntityManager::IsAvailable()
{
	return m_Loaded;
}

void TempEntityManager::Shutdown()
{
	if (!IsAvailable())
	{
		return;
	}

	SourceHook::List<TempEntityInfo *>::iterator iter;
	for (iter = m_TEList.begin(); iter != m_TEList.end(); iter++)
	{
		delete (*iter);
	}
	m_TEList.clear();

	m_TempEntInfo->Destroy();
	m_ListHead = nullptr;
	m_Loaded = false;
}

TempEntityInfo *TempEntityManager::GetTempEntityInfo(const char *name)
{
	if (!IsAvailable())
	{
		return NULL;
	}

	TempEntityInfo *te = NULL;
	if (!m_TempEntInfo->Retrieve(name, reinterpret_cast<void **>(&te)))
	{
		IBaseTempEntity *iter = m_ListHead;
		while (iter)
		{
			const char *realname = iter->GetName();
			if (!realname)
			{
				continue;
			}
			if (strcmp(name, realname) == 0)
			{
				te = new TempEntityInfo(iter);
				m_TempEntInfo->Insert(name, (void *)te);
				m_TEList.push_back(te);
				return te;
			}
			iter = iter->GetNext();
		}
		return NULL;
	}

	return te;
}

const char *TempEntityManager::GetNameFromThisPtr(IBaseTempEntity *me)
{
	return me->GetName();
}

void TE_Send(TempEntityInfo *g_TECurrent, int *iClient, int iTotal, float delay = 0.f)
{
	CUserRecipientFilter g_TERecFilter(iClient, iTotal);
    g_TECurrent->Send(g_TERecFilter, delay);
}

void TE_SendAll(TempEntityInfo *g_TECurrent, float delay = 0.f)
{
    const int iMaxClient = playerhelpers->GetMaxClients();

    int *client = new int[iMaxClient];
    int total = 0;
    for(int i(1); i <= iMaxClient; i++)
    {
        auto pPlayer = playerhelpers->GetGamePlayer(i);
        if(pPlayer)
        {
            if(pPlayer->IsConnected() && pPlayer->IsInGame() && !pPlayer->IsFakeClient())
            {
                client[total++] = i;
            }
        }
    }
    TE_Send(g_TECurrent, client, total, delay);
    delete[] client;
}

int FindStringIndex(const char *tableName, const char *str)
{
	auto pNetStringTable = g_pNetStringTableContainer->FindTable(tableName);
	if(pNetStringTable)
	{
		int nuxString = pNetStringTable->GetNumStrings();

		for(int i(0); i < nuxString; i++)
		{
			const char* szName = pNetStringTable->GetString(i);
			if(!V_stricmp(str, szName))
				return i;
		}
	}
	return INVALID_STRING_TABLE;
}

int PrecacheParticle(const char *sEffectName)
{
	auto pNetStringTable = g_pNetStringTableContainer->FindTable("ParticleEffectNames");
	if(pNetStringTable)
	{
		int index = pNetStringTable->FindStringIndex(sEffectName);
		if(index == INVALID_STRING_TABLE)
		{
			bool bSave = engine->LockNetworkStringTables(false);
			pNetStringTable->AddString(true, sEffectName, -1, "");
			engine->LockNetworkStringTables(bSave);
			index = pNetStringTable->FindStringIndex(sEffectName);
		}
		return index;
	}
	return INVALID_STRING_TABLE;
}

int GetParticleIndex(const char* szParticleName)
{
	int iParticleIndex = FindStringIndex("ParticleEffectNames", szParticleName);
	if(iParticleIndex == INVALID_STRING_TABLE)
	{
		iParticleIndex = PrecacheParticle(szParticleName);
	}
	return iParticleIndex;
}

void TempEntitySmoke(const Vector &vecPos, float flScale, int iFrameRate)
{
	if(g_TEManager.IsAvailable())
	{
		TempEntityInfo *pTEInfoSmoke = g_TEManager.GetTempEntityInfo("Smoke");
		if(pTEInfoSmoke)
		{
			ITESmoke *pSmoke = pTEInfoSmoke->Get<ITESmoke>();

			pSmoke->m_vecOrigin = vecPos;
			pSmoke->m_nModelIndex = g_ModelIndexSmoke;
			pSmoke->m_fScale = flScale;
			pSmoke->m_nFrameRate = iFrameRate;

			TE_SendAll(pTEInfoSmoke);
		}
	}
}

void DispatchParticleEffect(const char* szParticleName, Vector vecOrigin, Vector vecStart, QAngle vecAngle, edict_t *pEdict)
{
	int iEffect = GetParticleIndex(szParticleName);
	DispatchParticleEffect(iEffect, vecOrigin, vecStart, vecAngle, pEdict, PATTACH_POINT_FOLLOW);
}

void DispatchParticleEffect(const char* szParticleName, Vector vecOrigin, QAngle vecAngle, edict_t *pEdict)
{
	int iEffect = GetParticleIndex(szParticleName);
	DispatchParticleEffect(iEffect, vecOrigin, vecOrigin, vecAngle, pEdict, PATTACH_POINT_FOLLOW);
}

void DispatchParticleEffect(int iEffectIndex, Vector vecOrigin, Vector vecStart, QAngle vecAngle, edict_t* iEntIndex, ParticleAttachment_t bFollow)
{
	if(g_TEManager.IsAvailable())
	{
		TempEntityInfo *pDispatchEffect = g_TEManager.GetTempEntityInfo("EffectDispatch");
		if(pDispatchEffect)
		{
			ITEEffectDispatch* pEffect = pDispatchEffect->Get<ITEEffectDispatch>();

			pEffect->m_EffectData.m_vOrigin = vecOrigin;
			pEffect->m_EffectData.m_vStart = vecStart;
			pEffect->m_EffectData.m_iEffectName = FindStringIndex("EffectDispatch", "ParticleEffect");
			pEffect->m_EffectData.m_nHitBox = iEffectIndex;
			pEffect->m_EffectData.m_nEntIndex = g_Sample.IndexOfEdict(iEntIndex);
			pEffect->m_EffectData.m_nAttachmentIndex = 1;
			pEffect->m_EffectData.m_fFlags |= PARTICLE_DISPATCH_FROM_ENTITY;
			pEffect->m_EffectData.m_vAngles = vecAngle;
			pEffect->m_EffectData.m_flMagnitude = 0.f;
			pEffect->m_EffectData.m_flScale = 1.0;
			pEffect->m_EffectData.m_flRadius = 0.0;
			pEffect->m_EffectData.m_nDamageType = bFollow;

			TE_SendAll(pDispatchEffect);
		}
	}
}

void DebugDraw::DrawLine(const Vector &vecStart, const Vector &vecEnd, float flLife, const iColor4 &color, float flWidth, float flEndWidth, float flAmplitude, int iHaloIndex, int iStartFrame, int iFrameRate, int iSpeed, int FadeLength)
{
    if(g_TEManager.IsAvailable())
    {
        TempEntityInfo *g_TECurrent = g_TEManager.GetTempEntityInfo("BeamPoints");
		if(g_TECurrent)
		{
			ITEBeamPoints *pBPoint = g_TECurrent->Get<ITEBeamPoints>();

			pBPoint->m_vecStartPoint = vecStart;
			pBPoint->m_vecEndPoint = vecEnd;
			pBPoint->m_nModelIndex = g_ModelIndexLaserBeam;
			pBPoint->m_nHaloIndex = iHaloIndex;
			pBPoint->m_nStartFrame = iStartFrame;
			pBPoint->m_nFrameRate = iFrameRate;
			pBPoint->m_fLife = flLife;
			pBPoint->m_fWidth = flWidth;
			pBPoint->m_fEndWidth = flEndWidth;
			pBPoint->m_fAmplitude = flAmplitude;
			pBPoint->r = color.r;
			pBPoint->g = color.g;
			pBPoint->b = color.b;
			pBPoint->a = color.a;
			pBPoint->m_nSpeed = 1;
			pBPoint->m_nFadeLength = 1;

			TE_SendAll(g_TECurrent);
		}
    }
}

void DebugDraw::DrawCross3D(const Vector &position, float size, float flLife, const iColor4 &color)
{
	DrawLine(position + Vector(size, 0, 0), position - Vector(size, 0, 0), flLife, color, 2.f, 2.f);
	DrawLine(position + Vector(0, size, 0), position - Vector(0, size, 0), flLife, color, 3.f, 2.f);
	DrawLine(position + Vector(0, 0, size), position - Vector(0, 0, size), flLife, color, 2.f, 2.f);
}

void DebugDraw::DrawCross3D(const Vector &position, const Vector &mins, const Vector &maxs, float flLife, const iColor4 &color)
{
	Vector start = mins + position;
	Vector end   = maxs + position;
	DrawLine(start, end, flLife, color, 2.f, 2.f);

	start.x += (maxs.x - mins.x);
	end.x	-= (maxs.x - mins.x);
	DrawLine(start, end, flLife, color, 2.f, 2.f);

	start.y += (maxs.y - mins.y);
	end.y	-= (maxs.y - mins.y);
	DrawLine(start, end, flLife, color, 2.f, 2.f);

	start.x -= (maxs.x - mins.x);
	end.x	+= (maxs.x - mins.x);
	DrawLine(start, end, flLife, color, 2.f, 2.f);
}

void DebugDraw::DrawBox(const Vector &position, Vector mins, Vector maxs, float flLife, const iColor4 &color)
{
	if(mins == maxs)
	{
		mins = Vector(-13.f, -13.f, 0);
		maxs = Vector(13.f, 13.f, 72.f);
	}
	
	mins = position + mins;
	maxs = position + maxs;
	
	Vector vPos1(mins.x, maxs.y, maxs.z);
	Vector vPos2(maxs.x, mins.y, maxs.z);
	Vector vPos3(maxs.x, maxs.y, mins.z);
	Vector vPos4(maxs.x, mins.y, mins.z);
	Vector vPos5(mins.x, maxs.y, mins.z);
	Vector vPos6(mins.x, mins.y, maxs.z);

	DrawLine(maxs, vPos1, flLife, color);
	DrawLine(maxs, vPos2, flLife, color);
	DrawLine(maxs, vPos3, flLife, color);

	DrawLine(vPos6, vPos1, flLife, color);
	DrawLine(vPos6, vPos2, flLife, color);
	DrawLine(vPos6, mins, flLife, color);
	
	DrawLine(vPos4, mins, flLife, color);
	DrawLine(vPos5, mins, flLife, color);
	DrawLine(vPos5, vPos1, flLife, color);

	DrawLine(vPos5, vPos3, flLife, color);
	DrawLine(vPos4, vPos3, flLife, color);
	DrawLine(vPos4, vPos2, flLife, color);
}
