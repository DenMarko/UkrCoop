#ifndef _INCLUDE_INFECTED_H_
#define _INCLUDE_INFECTED_H_
#include "INextBotCombatCharacter.h"
#include "IWitchLocomotion.h"
#include "IZombieBotBody.h"
#include "CUserMessage.h"

class InfectedDying;
class ZombieBotVision;

class IInfectedAnimationLayer
{
public:
	DECLARE_CLASS_NOBASE( IInfectedAnimationLayer );

	IInfectedAnimationLayer()
	{
		Reset();
	}

	void Reset( void );

	void Update(IBaseAnimating* pAnim, IAnimationLayer* pLayer);

	virtual void NetworkStateChanged() {}
	virtual void NetworkStateChanged( void *pProp ) {}

private:
	CNetworkVar(int, m_nSequence);
	CNetworkVar(float, m_flStartTime);
	CNetworkVar(int, m_nOrder);
	CNetworkVar(bool, m_bLooping);
	float m_flCycle;
};

class IInfected : public INextBotCombatCharacter
{
	typedef IInfected ThisClass;
public:
	void Vocalize(const char* szVocalise, bool bValue);
	bool SetDamagedBodyGroupVariant(const char* szVal0, const char* szVal1);
	void AttackSurvivorTeam();
	const float GetRandomFactor() const;
public:
    virtual ~IInfected() { }

	virtual ServerClass*			GetServerClass(void) = 0;
	virtual int						YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	bool					ShouldCollide( int collisionGroup, int contentsMask ) const = 0;
	virtual void					Spawn( void ) = 0;
	virtual void					Precache( void ) = 0;
	virtual int						DrawDebugTextOverlays(void) = 0;
	virtual Class_T					Classify ( void ) = 0;
protected:
	virtual void					TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr ) = 0;
public:
	virtual void					Event_Killed( const CTakeDamageInfo &info ) = 0;
	virtual IInfected				*MyInfectedPointer(void) = 0;
	virtual Vector					EyePosition( void ) = 0;
	virtual unsigned int			PhysicsSolidMaskForEntity( void ) const = 0;
	virtual void					NetworkStateChanged_m_fFlags(void) = 0;
	virtual void					NetworkStateChanged_m_fFlags(void *) = 0;
	virtual int						OnSequenceSet(int) = 0;
	virtual const char*     		GetFootstepSound(const char *, bool, float, bool) const = 0;
	virtual CNavArea				*GetLastKnownArea(void) const = 0;
	virtual ZombieClassType         GetClass(void) const = 0;
    virtual CBaseCombatCharacter	*GetEntity(void) const = 0;
    virtual void	           		Update(void) = 0;
	virtual	int 					DoBloodEffect(float, const CTakeDamageInfo &info, const Vector &vec3, trace_t *tr) = 0;
	virtual IZombieBotLocomotion	*GetLocomotionInterface(void) const = 0;
	virtual IZombieBotBody			*GetBodyInterface(void) const = 0;
	virtual IIntention				*GetIntentionInterface(void) const = 0;
	virtual IZombieBotVision		*GetVisionInterface(void) const = 0;
    virtual void             		OnIgnite(void) = 0;
    virtual bool            		IsAbleToClimbOnto(CBaseEntity const*)const = 0;
    virtual bool            		IsAbleToBreak(CBaseEntity const*)const = 0;
    virtual bool            		IsAbleToBlockMovementOf(INextBot const*)const = 0;
    virtual bool             		ShouldTouch(CBaseEntity const*) const = 0;
    virtual bool            		IsSacrificeFor(ZombieClassType)const = 0;
    virtual bool            		TryToCull(void) = 0;
    virtual InfectedDying*  		CreateDeathAction(CTakeDamageInfo const&) = 0;
    virtual float		     		GetEyeOffsetUpdateInterval(void)const = 0;
    virtual void            		MakeLowViolence(void) = 0;
    virtual void			 		CreateComponents(void) = 0;

private:
	MyRecipientFilter 				m_recipFilter;
public:
	IZombieBotLocomotion*			m_pLocomotion;
	IZombieBotBody*					m_pBody;
	IIntention*						m_pIntention;
	IZombieBotVision*				m_pVision;
private:
	IntervalTimers					m_interval_871;
	IntervalTimers					m_interval_873;
	bool							m_bUnknown_875;
	CNetworkVar(bool,				m_mobRush);
public:
	IntervalTimers					m_burningTimer;
private:
	CountdownTimers					m_timer_878;
	CNetworkVar(float,				m_sequenceStartTime);
	CNetworkVar(int,				m_gibbedLimbs);
	CNetworkVector(					m_gibbedLimbForce);
	CNetworkVar(int,				m_originalBody);
	float							m_flRandomFactor;
	int								m_Unknown_888;
	CountdownTimers					m_vocaliseTimer;
	bool							m_bUnknown_3568;
	bool							m_bUnknown_3569;
	CNetworkHandle(IBaseEntity,		m_clientLookatTarget);
	CNetworkVar(float,				m_clientLeanYaw);
	CUtlVector<IInfectedAnimationLayer>	m_Unknown_895;
	CUtlVector<CHandle<IInfected>>		m_Unknown_900;
	CountdownTimers					m_timer_905;
};

inline const float IInfected::GetRandomFactor() const
{
    return m_flRandomFactor;
}

#endif