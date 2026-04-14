#ifndef _INCLUDE_VISION_H_
#define _INCLUDE_VISION_H_
#include "INextBotComponent.h"
#include "IBaseEntity.h"

class IBaseEntity;
class IBody;

class INextBotEntityFilter
{
public:
	virtual bool IsAllowed( IBaseEntity *entity ) const = 0;
};

class IVision : public INextBotComponent
{
public:
    virtual ~IVision() {}

	virtual void        	Reset( void );
	virtual void        	Update( void );

    virtual CBaseEntity*	GetPrimaryRecognizedThreat(void) const;
	virtual float       	GetTimeSinceVisible( int team ) const;
    virtual CBaseEntity*	GetClosestRecognized(int) const;
    virtual int         	GetRecognizedCount(int, float) const;
    virtual CBaseEntity*	GetClosestRecognized(const INextBotEntityFilter&) const;
	virtual float       	GetMaxVisionRange( void ) const { return 0.0f; }
	virtual float       	GetMinRecognizeTime( void ) const { return 0.f; }

	enum FieldOfViewCheckType
    {
        USE_FOV,
        DISREGARD_FOV
    };

	virtual bool        	IsAbleToSee( CBaseEntity *subject, FieldOfViewCheckType checkFOV, Vector *visibleSpot = nullptr ) const;
	virtual bool        	IsAbleToSee( const Vector &pos, FieldOfViewCheckType checkFOV ) const;
	virtual bool        	IsNoticed( CBaseEntity *subject ) const { return true; }
	virtual bool        	IsIgnored( CBaseEntity *subject ) const { return false; }
	virtual bool        	IsInFieldOfView( const Vector &pos ) const;
	virtual bool        	IsInFieldOfView( CBaseEntity *subject ) const;
	virtual float       	GetDefaultFieldOfView( void ) const { return 90.f; }
	virtual float       	GetFieldOfView( void ) const { return m_FOV; }
	virtual void        	SetFieldOfView( float horizAngle ) { m_FOV = horizAngle; m_cosHalfFOV = cos( 0.5f * m_FOV * M_PI / 180.0f ); }
	virtual bool        	IsLineOfSightClear( const Vector &pos ) const;
	virtual bool        	IsLineOfSightClearToEntity( const CBaseEntity *subject, Vector *visibleSpot = nullptr ) const;
	virtual bool        	IsLookingAt( const Vector &pos, float cosTolerance = 0.95f ) const;
	virtual bool        	IsLookingAt( const CBaseCombatCharacter *actor, float cosTolerance = 0.95f ) const;

private:
	void UpdateRecognizeEntitys( void );
	void UpdateRecognizedSet( void );
	void CollectPotentiallyVisibleEntities( CUtlVector<IBaseEntity*> *potentialVisible);

	template<typename T> bool ForEachRecognized(T& func);

    struct RecognizeInfo
    {
        CHandle<CBaseEntity> hEntity;							// +0x00: Handle сутності (4 байти)
		Vector vecLastKnownPosition;							// +0x04: Остання відома позиція сутності (12 байт)
        float flTimeLastSeen;									// +0x10: Float поле (4 байти)
    };

	IBody *m_bodyInterface;										// 6
	CountdownTimers m_scanTimer;								// 7 8
	float m_FOV;												// 9
	float m_cosHalfFOV;											// 10
	CUtlVector< RecognizeInfo > m_knownEntityVector;			// 11 12 13 14 15
	mutable CHandle< CBaseEntity > m_hPrimaryThreat;			// 16
	float m_lastVisionUpdateTimestamp;							// 17
	IntervalTimers m_notVisibleTimer[ MAX_TEAMS ];				// 18 - 50
};


template<typename T> inline bool IVision::ForEachRecognized(T& func)
{
	if(m_knownEntityVector.Count() <= 0)
		return true;

	for ( int i = 0; (m_knownEntityVector).IsUtlVector && i < (m_knownEntityVector).Count(); i++ )
	{
		RecognizeInfo& recog = m_knownEntityVector[i];
		if(g_pGlobals->curtime >= recog.flTimeLastSeen)
		{
			CBaseEntity *pEntity = recog.hEntity.Get();
			if(!pEntity)
				continue;

			if(!func((IBaseEntity*)pEntity, recog.flTimeLastSeen))
				return false;
		}
	}

	return true;
}
#endif