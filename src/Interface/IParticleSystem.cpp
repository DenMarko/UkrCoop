#include "IParticleSystem.h"

void IParticleSystem::StopParticleSystem(void)
{
    bool &m_bActive = access_member<bool>(this, 896);
    m_bActive = false;
    this->NetworkStateChanged(896);
}

void IParticleSystem::StartParticleSystem(void)
{
    bool &m_bActive = access_member<bool>(this, 896);
    float &m_flStartTime = access_member<float>(this, 904);

    if(m_bActive == false)
    {
        m_flStartTime = g_pGlobals->curtime;
        this->NetworkStateChanged(904);
        m_bActive = true;
        this->NetworkStateChanged(896);

        ReadControlPointEnts();
    }
}

void IParticleSystem::InputStart(inputdata_t &inputdata)
{
    StartParticleSystem();
}

void IParticleSystem::InputStop(inputdata_t &inputdata)
{
    StopParticleSystem();
}

void IParticleSystem::StartParticleSystemThink(void)
{
    StopParticleSystem();
}

#include "HL2.h"
void IParticleSystem::ReadControlPointEnts(void)
{
    string_t* m_iszControlPointNames = access_array_member<string_t>(this, 908);
    constexpr int kMAXCONTROLPOINTS = 63;

    for(int i = 0; i < kMAXCONTROLPOINTS; ++i)
    {
        if(m_iszControlPointNames[i] == NULL_STRING)
            continue;

        IBaseEntity *pPointENT = g_CallHelper->FindEntityGeneric(nullptr, m_iszControlPointNames[i].ToCStr(), this);
        if(pPointENT == nullptr)
        {
            Warning("Particle system %s could not find control point entity (%s)\n", GetDebugName(), m_iszControlPointNames[i].ToCStr() );
            continue;
        }

        access_array_member<EHANDLE>(this, 1160, i) = pPointENT->GetRefEHandle();
        this->NetworkStateChanged(1160);
    }
}

void IBaseParticleEntity::SetLifetime(float lifetime)
{
	if(lifetime == -1)
		SetNextThink( TICK_NEVER_THINK );
	else
		SetNextThink( g_pGlobals->curtime + lifetime );
}

void IParticleSmokeGrenete::FillVolume(void)
{
    m_CurrentStage = 1;            // m_CurrentStage
    CollisionProp()->SetCollisionBounds(Vector(-50, -50, -50), Vector(50, 50, 50));
}

void IParticleSmokeGrenete::SetFadeTime(float startTime, float endTime)
{
    m_FadeStartTime = startTime; // m_FadeStartTime
    m_FadeEndTime = endTime; // m_FadeEndTime
}

void IParticleSmokeGrenete::SetRelativeFadeTime(float startTime, float endTime)
{
	float flCurrentTime = g_pGlobals->curtime - m_flSpawnTime;

	m_FadeStartTime = flCurrentTime + startTime;
	m_FadeEndTime = flCurrentTime + endTime;
}