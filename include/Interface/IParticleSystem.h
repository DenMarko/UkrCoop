#ifndef _INCLUDE_PARTICLE_SYSTEM_HEADER_
#define _INCLUDE_PARTICLE_SYSTEM_HEADER_
#include "IBaseEntity.h"

class IParticleSystem : public IBaseEntity
{
public:
    virtual ~IParticleSystem() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;

	virtual int  UpdateTransmitState(void) = 0;
	virtual void Spawn( void ) = 0;
	virtual void Precache( void ) = 0;
	virtual void Activate( void ) = 0;

    void        StopParticleSystem( void );
    void        StartParticleSystem( void );
	void		InputStart( inputdata_t &inputdata );
	void		InputStop( inputdata_t &inputdata );
	void		StartParticleSystemThink( void );
protected:
    void		ReadControlPointEnts( void );
};

class IBaseParticleEntity : public IBaseEntity
{
public:
	virtual ~IBaseParticleEntity() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual int  			UpdateTransmitState(void) = 0;
	virtual void 			Activate( void ) = 0;
	virtual void			Think( void ) = 0;

	void SetLifetime(float lifetime);
};

class IParticleSmokeGrenete: public IBaseParticleEntity
{
	DECLARE_CLASS( IParticleSmokeGrenete, IBaseParticleEntity );

public:
	virtual ~IParticleSmokeGrenete() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;

	virtual void			Spawn( void ) = 0;
	virtual int  			UpdateTransmitState(void) = 0;

public:
	void FillVolume( void );
	void SetFadeTime(float startTime, float entTime);
	void SetRelativeFadeTime(float startTime, float endTime);

	CNetworkVar( unsigned char, m_CurrentStage );

	CNetworkVar( float, m_flSpawnTime );

	// When to fade in and out.
	CNetworkVar( float, m_FadeStartTime );
	CNetworkVar( float, m_FadeEndTime );
};

namespace {
	inline bool SmokeGranate(Vector vecPos)
	{
		IParticleSmokeGrenete *pSmoke = (IParticleSmokeGrenete*)g_Sample.CreateEntityByName("env_particlesmokegrenade");
		if(pSmoke)
		{
			pSmoke->SetLocalOrigin(vecPos);
			pSmoke->SetFadeTime(25, 30);
			pSmoke->Activate();
			pSmoke->SetLifetime(30);
			pSmoke->FillVolume();

			return true;
		}

		return false;
	}
}

#endif // _INCLUDE_PARTICLE_SYSTEM_HEADER_