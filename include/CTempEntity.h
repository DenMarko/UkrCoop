#ifndef _INCLUDE_CTEMPENTITY_PROPER_H_
#define _INCLUDE_CTEMPENTITY_PROPER_H_
#include "extension.h"
#include "Interface/IBaseTempEntity.h"

class TempEntityInfo
{
public:
	TempEntityInfo(IBaseTempEntity *me);
public:
	const char *GetName();
	ServerClass *GetServerClass();
	bool IsValidProp(const char *name);
	bool TE_SetEntData(const char *name, int value);
	bool TE_SetEntDataFloat(const char *name, float value);
	bool TE_SetEntDataVector(const char *name, Vector vector);
	bool TE_SetEntDataFloatArray(const char *name, cell_t *array, int size);
	bool TE_GetEntData(const char *name, int *value);
	bool TE_GetEntDataFloat(const char *name, float *value);
	bool TE_GetEntDataVector(const char *name, Vector *vector);
	void Send(IRecipientFilter &filter, float delay);
	template<typename T>
	T* Get();
private:
	int _FindOffset(const char *name, int *size = NULL);
private:
	IBaseTempEntity *m_Me;
};

class TempEntityManager
{
public:
	TempEntityManager() : m_Loaded(false) {}
public:
	void Initialize();
	bool IsAvailable();
	void Shutdown();
public:
	TempEntityInfo *GetTempEntityInfo(const char *name);
	const char *GetNameFromThisPtr(IBaseTempEntity *me);
private:
	SourceHook::List<TempEntityInfo *> m_TEList;
	IBasicTrie *m_TempEntInfo;
	IBaseTempEntity *m_ListHead;
	bool m_Loaded;
};

extern TempEntityManager g_TEManager;

void InitPrecache();

struct iColor4
{
    iColor4(int iR, int iG, int iB, int iA) : r(iR), g(iG), b(iB), a(iA) {}
    int r;
    int g;
    int b;
    int a;
};

enum ParticleAttachment_t
{
	PATTACH_ABSORIGIN = 0,
	PATTACH_ABSORIGIN_FOLLOW,
	PATTACH_CUSTOMORIGIN,
	PATTACH_POINT,
	PATTACH_POINT_FOLLOW,
	PATTACH_WORLDORIGIN,
	PATTACH_ROOTBONE_FOLLOW,
	MAX_PATTACH_TYPES,
};

#define PARTICLE_DISPATCH_FROM_ENTITY		(1<<0)
#define PARTICLE_DISPATCH_RESET_PARTICLES	(1<<1)

void DispatchParticleEffect(int iEffectIndex, Vector vecOrigin, Vector vecStart, QAngle vecAngle, edict_t* iEntIndex, ParticleAttachment_t bFollow);
void DispatchParticleEffect(const char* szParticleName, Vector vecOrigin, Vector vecStart, QAngle vecAngle, edict_t *pEdict);
void DispatchParticleEffect(const char* szParticleName, Vector vecOrigin, QAngle vecAngle, edict_t *pEdict);

void TempEntitySmoke(const Vector &vecPos, float flScale, int iFrameRate);

namespace DebugDraw
{
    void DrawLine(const Vector &vecStart, const Vector &vecEnd, float flLife, const iColor4 &color, float flWidth = 1.f, float flEndWidth = 1.f, float flAmplitude = 0.f, int iHaloIndex = 0, int iStartFrame = 0, int iFrameRate = 0, int iSpeed = 0, int FadeLength = 1);
	void DrawCross3D(const Vector &position, float size, float flLife, const iColor4 &color);
	void DrawCross3D(const Vector &position, const Vector &mins, const Vector &maxs, float flLife, const iColor4 &color);
	void DrawBox(const Vector &position, Vector mins, Vector maxs, float flLife, const iColor4 &color);
};

#endif

template <typename T>
inline T* TempEntityInfo::Get()
{
	return (T*)m_Me;
}
