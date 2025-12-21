#include "IVision.h"
#include "INextBot.h"
#include "INavArea.h"
#include "INextBotMeneger.h"
#include "../HL2.h"
#include "../ConVar_l4d.h"


bool IVision::IsAbleToSee(CBaseEntity *subject, FieldOfViewCheckType checkFOV, Vector *visibleSpot) const
{
    if(!m_bodyInterface)
    {
        return false;
    }

    if(GetBot()->IsRangeGreaterThan(subject, GetMaxVisionRange()))
    {
        return false;
    }

    if ( reinterpret_cast<IBaseCombatCharacter*>(GetBot()->GetEntity())->IsHiddenByFog( subject ) )
	{
		return false;
	}

	if ( checkFOV == USE_FOV && !IsInFieldOfView( subject ) )
	{
		return false;
	}

    if (!IsNoticed(subject))
    {
        return false;
    }
    
    return IsLineOfSightClearToEntity(subject);
}

bool IVision::IsAbleToSee(const Vector &pos, FieldOfViewCheckType checkFOV) const
{
    if(!m_bodyInterface)
    {
        return false;
    }

    if(GetBot()->IsRangeGreaterThan(pos, GetMaxVisionRange()))
    {
        return false;
    }

    if ( reinterpret_cast<IBaseCombatCharacter*>(GetBot()->GetEntity())->IsHiddenByFog( pos ) )
	{
		return false;
	}

	if ( checkFOV == USE_FOV && !IsInFieldOfView( pos ) )
	{
		return false;
	}

    return IsLineOfSightClear(pos);
}

bool IVision::IsInFieldOfView(const Vector &pos) const
{
    return PointWithinViewAngle( GetBot()->GetBodyInterface()->GetEyePosition(), pos, GetBot()->GetBodyInterface()->GetViewVector(), m_cosHalfFOV );
}

bool IVision::IsInFieldOfView(CBaseEntity *subject) const
{
	if ( IsInFieldOfView( reinterpret_cast<IBaseEntity*>(subject)->WorldSpaceCenter() ) )
	{
		return true;
	}

	return IsInFieldOfView( reinterpret_cast<IBaseEntity*>(subject)->EyePosition() );
}

bool IVision::IsLineOfSightClear(const Vector &pos) const
{
    trace_t result;
	NextBotVisionTraceFilter filter( reinterpret_cast<IBaseEntity*>(GetBot()->GetEntity()), COLLISION_GROUP_NONE );
	
	util_TraceLine( GetBot()->GetBodyInterface()->GetEyePosition(), pos, MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, &filter, &result );
	
	return ( result.fraction >= 1.0f && !result.startsolid );
}

bool IVision::IsLineOfSightClearToEntity(const CBaseEntity *subject, Vector *visibleSpot) const
{
    auto pEnt = const_cast<CBaseEntity*>(subject);

	bool bClear = g_CallHelper->IsLineOfSightBetweenTwoEntitiesClear( reinterpret_cast<IBaseEntity*>(GetBot()->GetBodyInterface()->GetEntity()), EOFFSET_MODE_EYEPOSITION,
														reinterpret_cast<IBaseEntity*>(pEnt), EOFFSET_MODE_WORLDSPACE_CENTER,
														reinterpret_cast<IBaseEntity*>(pEnt), COLLISION_GROUP_NONE,
														MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, VisionTraceFilterFunction, 1.0 );
    return bClear;
}

bool IVision::IsLookingAt(const Vector &pos, float cosTolerance) const
{
	Vector to = pos - GetBot()->GetBodyInterface()->GetEyePosition();
	to.NormalizeInPlace();

	Vector forward;
	AngleVectors( reinterpret_cast<IBaseCombatCharacter*>(GetBot()->GetEntity())->EyeAngles(), &forward );

	return DotProduct( to, forward ) > cosTolerance;
}

bool IVision::IsLookingAt(const CBaseCombatCharacter *actor, float cosTolerance) const
{
    return IsLookingAt( ((IBaseCombatCharacter*)const_cast<CBaseCombatCharacter*>(actor))->EyePosition(), cosTolerance );
}

class ConsiderRecognizedSet 
{
public:
	ConsiderRecognizedSet(INextBot* bot, float LastUpdate) : pBot(bot), lastUpdateTime(LastUpdate), bestThreat(nullptr) {}

	bool operator() (IBaseEntity* pEnt, float &flTimeLastSeen)
	{
		IBaseCombatCharacter* pEntity = (IBaseCombatCharacter*)pEnt->MyCombatCharacterPointer();
		if(pEntity)
		{
			if(pEntity->IsAlive())
			{
				if(pBot->IsEnemy(reinterpret_cast<CBaseCombatCharacter*>(pEntity)))
				{
					if(bestThreat)
					{
						IIntention *pIntention = pBot->GetIntentionInterface();
						bestThreat = (IBaseEntity*)pIntention->SelectMoreDangerousThreat(pBot, pBot->GetEntity(), (CBaseCombatCharacter*)bestThreat, (CBaseCombatCharacter* )pEntity);
					}
					else
					{
						bestThreat = pEntity;
					}
				}
			}
		}

		if( flTimeLastSeen <= lastUpdateTime )
			return true;

		pBot->OnSight((CBaseEntity*)pEnt);

		return true;
	}

	IBaseEntity *GetBestThreat( void ) { return bestThreat; }
private:
	INextBot* pBot;
	float lastUpdateTime;
	IBaseEntity* bestThreat;
};

void IVision::UpdateRecognizeEntitys(void)
{
	UpdateRecognizedSet();
	INextBot *pBot = GetBot();

	ConsiderRecognizedSet recognize(pBot, m_lastVisionUpdateTimestamp);
	ForEachRecognized(recognize);

	IBaseEntity *BestThreat = nullptr;
	if(recognize.GetBestThreat())
	{
		BestThreat = recognize.GetBestThreat();
	}

	IBaseEntity *PrimaryThreat = nullptr;
	if(m_hPrimaryThreat.Get())
	{
		PrimaryThreat = (IBaseEntity*)(m_hPrimaryThreat.Get());
	}

	if(BestThreat != PrimaryThreat)
	{
		if(BestThreat)
			m_hPrimaryThreat = BestThreat->GetRefEHandle();
		else
			m_hPrimaryThreat.Term();

		CBaseEntity* ThreatChange = nullptr;
		if(m_hPrimaryThreat.Get())
		{
			ThreatChange = m_hPrimaryThreat.Get();
		}
		pBot->OnThreatChanged(ThreatChange);
	}
}

class CollectVisible
{
public:
	CollectVisible( IVision *vision )
	{
		m_vision = vision;
	}
	
	bool operator() ( IBaseEntity *entity )
	{
		if ( entity &&
			 !m_vision->IsIgnored( (CBaseEntity*)entity ) &&
			 entity->IsAlive() &&
			 entity != reinterpret_cast<IBaseEntity*>(m_vision->GetBot()->GetEntity()) &&
			 m_vision->IsAbleToSee( (CBaseEntity*)entity, IVision::USE_FOV ) )
		{
			m_recognized.AddToTail( entity );	
		}
			
		return true;
	}
	
	bool Contains( IBaseEntity *entity ) const
	{
		for( int i=0; i < m_recognized.Count(); ++i )
		{
			if ( entity->entindex() == m_recognized[ i ]->entindex() )
			{
				return true;
			}
		}
		return false;
	}
	
	IVision *m_vision;
	CUtlVector< IBaseEntity * > m_recognized;
};

void IVision::UpdateRecognizedSet(void)
{
	CUtlVector<IBaseEntity*> visibleEntities;
	CollectPotentiallyVisibleEntities(&visibleEntities);
	
	CollectVisible visibleNow(this);
	_FOR_EACH_VEC_(visibleEntities, i)
	{
		if(!visibleNow(visibleEntities[i]))
		{
			break;
		}
	}

	_FOR_EACH_VEC_(m_knownEntityVector, i)
	{
		RecognizeInfo* info = &m_knownEntityVector[i];
		if(!info->hEntity.IsValid())
		{
			m_knownEntityVector.Remove(i);
			continue;
		}

		IBaseEntity *pEntity = (IBaseEntity*)(info->hEntity.Get());
		if(!pEntity)
		{
			m_knownEntityVector.Remove(i);
			continue;
		}

		if(visibleNow.Contains(pEntity))
		{
			info->vecLastKnownPosition = pEntity->GetAbsOrigin();
			info->flTimeLastSeen = g_pGlobals->curtime;

			int team = pEntity->GetTeamNumber();
			if(team >= 0 && team < MAX_TEAMS)
			{
				m_notVisibleTimer[team].Start();
			}
		}
		else if( g_pGlobals->curtime - info->flTimeLastSeen > 0.5f)
		{
			GetBot()->OnLostSight((CBaseEntity*)pEntity);
			m_knownEntityVector.Remove(i);
		}
	}

	bool alreadyKnown = false;
	_FOR_EACH_VEC_(visibleNow.m_recognized, i)
	{
		alreadyKnown = false;
		_FOR_EACH_VEC_(m_knownEntityVector, j)
		{
			if(visibleNow.m_recognized[i] == (IBaseEntity*)(m_knownEntityVector[j].hEntity.Get()))
			{
				alreadyKnown = true;
				break;
			}
		}

		IBaseEntity* pEntity = visibleNow.m_recognized[i];
		if(!alreadyKnown)
		{
			RecognizeInfo info;
			info.hEntity = pEntity->GetRefEHandle();
			info.vecLastKnownPosition = pEntity->GetAbsOrigin();
			info.flTimeLastSeen = GetMinRecognizeTime() + g_pGlobals->curtime;

			m_knownEntityVector.AddToTail(info);

			int team = pEntity->GetTeamNumber();
			if(team >= 0 && team < MAX_TEAMS)
			{
				m_notVisibleTimer[team].Start();
			}
		}
	}
}

class PopulateVisibleVector
{
public:
	PopulateVisibleVector( CUtlVector< IBaseEntity* > *potentiallyVisible )
	{
		m_potentiallyVisible = potentiallyVisible;
	}

	bool operator() ( IBaseEntity *actor )
	{
		m_potentiallyVisible->AddToTail( actor );
		return true;
	}

	CUtlVector< IBaseEntity * > *m_potentiallyVisible;
};

void IVision::CollectPotentiallyVisibleEntities( CUtlVector< IBaseEntity* > *potentialVisible)
{
	potentialVisible->RemoveAll();

	PopulateVisibleVector potentialVisibles( potentialVisible );
	ForEachActor(potentialVisibles);
}

void IVision::Reset( void )
{
	INextBotComponent::Reset();

	m_knownEntityVector.RemoveAll();
	m_lastVisionUpdateTimestamp = 0.0f;
	m_hPrimaryThreat = NULL;

	m_FOV = GetDefaultFieldOfView();
	m_cosHalfFOV = cos( 0.5f * m_FOV * M_PI / 180.0f );
	
	for( int i=0; i<MAX_TEAMS; ++i )
	{
		m_notVisibleTimer[i].Invalidate();
	}
}

void IVision::Update()
{
	if(!m_scanTimer.IsElapsed())
		return;

	float newTime = 0.5f * GetMinRecognizeTime();
	m_scanTimer.Start(newTime + newTime);

	if(g_pConVar->GetConVarInt("nb_blind") == 1)
	{
		m_knownEntityVector.RemoveAll();
		m_hPrimaryThreat.Term();
		return;
	}

	UpdateRecognizeEntitys();

	m_lastVisionUpdateTimestamp = g_pGlobals->curtime;
}

CBaseEntity *IVision::GetPrimaryRecognizedThreat(void) const
{
    if (!m_hPrimaryThreat.IsValid())
        return nullptr;
    
    CBaseEntity *pEntity = m_hPrimaryThreat.Get();
    if (!pEntity)
        return nullptr;
    
    if (this->IsIgnored(pEntity))
        return nullptr;

    return pEntity;
}

float IVision::GetTimeSinceVisible(int team) const
{
    if (team == -1) 
    {
		float flTime = 9999999999.9f;
        for (int i = 0; i < MAX_TEAMS; ++i) 
        {
            if (m_notVisibleTimer[i].HasStarted())
            {
				if(flTime > m_notVisibleTimer[i].GetElapsedTime())
				{
					team = m_notVisibleTimer[i].GetElapsedTime();
				}
            }
        }
        return flTime;
    }
    
    if (team >= 0 || team < MAX_TEAMS) 
    {
		return m_notVisibleTimer[team].GetElapsedTime();
    }
    
	return 0.0;
}

CBaseEntity *IVision::GetClosestRecognized(int teamFilter) const
{
	INextBot* pBot = GetBot();
	if(!pBot)
    	return nullptr;

	Vector myPos = pBot->GetPosition();

	CBaseEntity *pClosest = nullptr;
	float closeRange = 999999999.9f;

	_FOR_EACH_VEC_(m_knownEntityVector, i)
	{
		const RecognizeInfo& mInfo = m_knownEntityVector[i];

		if(!mInfo.hEntity.IsValid())
			continue;

		CBaseEntity* pEnts = mInfo.hEntity.Get();
		if(!pEnts)
			continue;
		
		if(teamFilter != -1 && reinterpret_cast<IBaseEntity*>(pEnts)->GetTeamNumber() != teamFilter)
			continue;

		Vector to = reinterpret_cast<IBaseEntity*>(pEnts)->GetAbsOrigin() - myPos;
		float rangeSq = to.LengthSqr();

		if(rangeSq < closeRange)
		{
			pClosest = pEnts;
			closeRange = rangeSq;
		}
	}

	return pClosest;
}

int IVision::GetRecognizedCount(int teamFilter, float maxDistance) const
{
    int count = 0;
	_FOR_EACH_VEC_(m_knownEntityVector, i)
	{
		const RecognizeInfo& mInfo = m_knownEntityVector[i];
		if(!mInfo.hEntity.IsValid())
			continue;

		CBaseEntity* pEnt = mInfo.hEntity.Get();
		if(!pEnt)
			continue;

		if(teamFilter != -1 && reinterpret_cast<IBaseEntity*>(pEnt)->GetTeamNumber() != teamFilter)
			continue;

		if(maxDistance >= 0.f)
		{
			INextBot *pBot = GetBot();
			if(!pBot || !pBot->IsRangeLessThan(pEnt, maxDistance))
				continue;
		}

		count++;
	}

	return count;
}

CBaseEntity *IVision::GetClosestRecognized(const INextBotEntityFilter &filter) const
{
	INextBot* pBot = GetBot();
	if(!pBot)
    	return nullptr;

	Vector myPos = pBot->GetPosition();

	CBaseEntity *pClosest = nullptr;
	float closeRange = 999999999.9f;

	_FOR_EACH_VEC_(m_knownEntityVector, i)
	{
		const RecognizeInfo& mInfo = m_knownEntityVector[i];

		if(!mInfo.hEntity.IsValid())
			continue;

		CBaseEntity* pEnts = mInfo.hEntity.Get();
		if(!pEnts)
			continue;
		
		if(!filter.IsAllowed(reinterpret_cast<IBaseEntity*>(pEnts)))
			continue;

		Vector to = reinterpret_cast<IBaseEntity*>(pEnts)->GetAbsOrigin() - myPos;
		float rangeSq = to.LengthSqr();

		if(rangeSq < closeRange)
		{
			pClosest = pEnts;
			closeRange = rangeSq;
		}
	}

	return pClosest;
}
