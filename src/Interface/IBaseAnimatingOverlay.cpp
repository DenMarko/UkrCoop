#include "IBaseAnimatingOverlay.h"
#include "extension.h"
#include "CStudio.h"

IAnimationLayer::IAnimationLayer(void)
{
    Init(nullptr);
}

void IAnimationLayer::Init(IBaseAnimatingOverlay *pOverlay)
{
	m_pOwnerEntity = pOverlay;
	m_fFlags = 0;
	m_flWeight = 0;
	m_flCycle = 0;
	m_flPrevCycle = 0;
	m_bSequenceFinished = false;
	m_nActivity = ACT_INVALID;
	m_nSequence = 0;
	m_nPriority = 0;
	m_nOrder = 15 ;

	m_flBlendIn = 0.0;
	m_flBlendOut = 0.0;

	m_flKillRate = 100.0;
	m_flKillDelay = 0.0;
	m_flPlaybackRate = 1.0;
	m_flLastEventCheck = 0.0;
	m_flLastAccess = g_pGlobals->curtime;
	m_flLayerAnimtime = 0;
	m_flLayerFadeOuttime = 0;
}

void IAnimationLayer::StudioFrameAdvance(float flInterval, IBaseAnimating *pOwner)
{
	float flCycleRate = pOwner->GetSequenceCycleRate(m_nSequence);

	m_flPrevCycle = m_flCycle;
	m_flCycle += flInterval * flCycleRate * m_flPlaybackRate;

	if (m_flCycle < 0.0)
	{
		if (m_bLooping)
		{
			m_flCycle -= (int)(m_flCycle);
		}
		else
		{
			m_flCycle = 0;
		}
	}
	else if (m_flCycle >= 1.0) 
	{
		m_bSequenceFinished = true;

		if (m_bLooping)
		{
			m_flCycle -= (int)(m_flCycle);
		}
		else
		{
			m_flCycle = 1.0;
		}
	}

	if (IsAutoramp())
	{
		m_flWeight = 1;
	
		// blend in?
		if ( m_flBlendIn != 0.0f )
		{
			if (m_flCycle < m_flBlendIn)
			{
				m_flWeight = m_flCycle / m_flBlendIn;
			}
		}
		
		// blend out?
		if ( m_flBlendOut != 0.0f )
		{
			if (m_flCycle > 1.0 - m_flBlendOut)
			{
				m_flWeight = (1.0 - m_flCycle) / m_flBlendOut;
			}
		}

		m_flWeight = 3.0 * m_flWeight * m_flWeight - 2.0 * m_flWeight * m_flWeight * m_flWeight;
		if (m_nSequence == 0)
			m_flWeight = 0;
	}
}

mstudioevent_t *GetEventIndexForSequence( mstudioseqdesc_t &seqdesc )
{
	if (!(seqdesc.flags & STUDIO_EVENT))
	{
		SetEventIndexForSequence( seqdesc );
	}

	return seqdesc.pEvent( 0 );
}

int GetAnimationEvent( CStudioHdr *pstudiohdr, int sequence, animevent_t *pNPCEvent, float flStart, float flEnd, int index )
{
	if ( !pstudiohdr || sequence >= GetNumSeq(pstudiohdr) || !pNPCEvent )
		return 0;

	mstudioseqdesc_t &seqdesc = pSeqdesc(pstudiohdr, sequence );
	if (seqdesc.numevents == 0 || index >= (int)seqdesc.numevents )
		return 0;

	mstudioevent_t *pevent = GetEventIndexForSequence( seqdesc );
	for (; index < (int)seqdesc.numevents; index++)
	{
		if ( pevent[index].type & (1<<10) )
		{
			if ( !(pevent[index].type & (1<<0)) )
				 continue;
		}
		else if ( pevent[index].event >= 5000 )
			continue;
	
		bool bOverlapEvent = false;

		if (pevent[index].cycle >= flStart && pevent[index].cycle < flEnd)
		{
			bOverlapEvent = true;
		}

		else if ((seqdesc.flags & STUDIO_LOOPING) && flEnd < flStart)
		{
			if (pevent[index].cycle >= flStart || pevent[index].cycle < flEnd)
			{
				bOverlapEvent = true;
			}
		}

		if (bOverlapEvent)
		{
			pNPCEvent->pSource = NULL;
			pNPCEvent->cycle = pevent[index].cycle;
			pNPCEvent->eventtime = g_pGlobals->curtime;
			pNPCEvent->event = pevent[index].event;
			pNPCEvent->options = pevent[index].pszOptions();
			pNPCEvent->type	= pevent[index].type;
			return index + 1;
		}
	}
	return 0;
}

void IAnimationLayer::DispatchAnimEvents(IBaseAnimating *eventHandler, IBaseAnimating *pOwner)
{
	animevent_t	event;
	CStudioHdr *pstudiohdr = pOwner->GetModelPtr( );

	if ( !pstudiohdr )
	{
		return;
	}

	if ( !SeqencesAvailable(pstudiohdr) )
	{
		return;
	}

	if ( m_nSequence >= GetNumSeq(pstudiohdr) )
		return;
	
	if ( pSeqdesc( pstudiohdr, m_nSequence ).numevents == 0 )
	{
		return;
	}

	float flCycleRate = pOwner->GetSequenceCycleRate( m_nSequence ) * m_flPlaybackRate;
	float flStart = m_flLastEventCheck;
	float flEnd = m_flCycle;

	if (!m_bLooping)
	{
		float flLastVisibleCycle = 1.0f - (pSeqdesc(pstudiohdr, m_nSequence ).fadeouttime) * flCycleRate;
		if (flEnd >= flLastVisibleCycle || flEnd < 0.0) 
		{
			m_bSequenceFinished = true;
			flEnd = 1.0f;
		}
	}
	m_flLastEventCheck = flEnd;

	int index = 0;
	while ( (index = GetAnimationEvent( pstudiohdr, m_nSequence, &event, flStart, flEnd, index ) ) != 0 )
	{
		event.pSource = pOwner;
		if (flCycleRate > 0.0)
		{
			float flCycle = event.cycle;
			if (flCycle > m_flCycle)
			{
				flCycle = flCycle - 1.0;
			}
			event.eventtime = access_member<float>(pOwner, 128) + (flCycle - m_flCycle) / flCycleRate + pOwner->GetAnimTimeInterval();
		}

		eventHandler->HandleAnimEvent( &event );
	}
}

bool IAnimationLayer::IsAbandoned(void)
{
	if (IsActive() && !IsAutokill() && !IsKillMe() && m_flLastAccess > 0.0 && (g_pGlobals->curtime - m_flLastAccess > 0.2)) 
		return true; 
	else 
		return false;
}

void IAnimationLayer::MarkActive(void)
{
	m_flLastAccess = g_pGlobals->curtime;
}

int IBaseAnimatingOverlay::AddGestureSequence(int sequence, bool autokill)
{
	int i = AddLayeredSequence( sequence, 0 );
	// No room?
	if ( IsValidLayer( i ) )
	{
		SetLayerAutokill( i, autokill );
	}

	return i;
}

int IBaseAnimatingOverlay::AddGestureSequence(int sequence, float flDuration, bool autokill)
{
	int iLayer = AddGestureSequence( sequence, autokill );
	Assert( iLayer != -1 );

	if (iLayer >= 0 && flDuration > 0)
	{
		m_AnimOverlay[iLayer].m_flPlaybackRate = SequenceDuration( sequence ) / flDuration;
	}
	return iLayer;
}

int IBaseAnimatingOverlay::AddGesture(Activity activity, bool autokill)
{
    if(IsPlayingGesture(activity))
    {
        return FindGestureLayer(activity);
    }

    int seq = SelectWeightedSequence(activity);
    if(seq <= 0)
    {
        return -1;
    }

    int i = AddGestureSequence(seq, autokill);
    if(i != -1)
    {
        m_AnimOverlay[i].m_nActivity = activity;
    }

    return i;
}

int IBaseAnimatingOverlay::AddGesture(Activity activity, float flDuration, bool autokill)
{
	int iLayer = AddGesture( activity, autokill );
	SetLayerDuration( iLayer, flDuration );

	return iLayer;
}

void IBaseAnimatingOverlay::RemoveGesture(Activity activity)
{
	int iLayer = FindGestureLayer(activity);
	if(iLayer == -1)
		return;

	RemoveLayer(iLayer);
}

bool IBaseAnimatingOverlay::IsPlayingGesture(Activity activity)
{
    return FindGestureLayer(activity) != -1 ? true : false;
}

void IBaseAnimatingOverlay::RemoveAllGestures(void)
{
	for(int i = 0; i < m_AnimOverlay.Count(); i++)
	{
		RemoveLayer(i);
	}
}

int IBaseAnimatingOverlay::AddLayeredSequence(int sequence, int iPriority)
{
	int i = AllocateLayer( iPriority );
	// No room?
	if ( IsValidLayer( i ) )
	{
		m_AnimOverlay[i].m_flCycle = 0;
		m_AnimOverlay[i].m_flPrevCycle = 0;
		m_AnimOverlay[i].m_flPlaybackRate = 1.0;
		m_AnimOverlay[i].m_nActivity = ACT_INVALID;
		m_AnimOverlay[i].m_nSequence = sequence;
		m_AnimOverlay[i].m_flWeight = 1.0f;
		m_AnimOverlay[i].m_flBlendIn = 0.0f;
		m_AnimOverlay[i].m_flBlendOut = 0.0f;
		m_AnimOverlay[i].m_bSequenceFinished = false;
		m_AnimOverlay[i].m_flLastEventCheck = 0;
		m_AnimOverlay[i].m_bLooping = ((GetSequenceFlags( GetModelPtr(), sequence ) & 0x0001) != 0);
	}

	return i;
}

bool IBaseAnimatingOverlay::IsValidLayer(int iLayer)
{
	return (iLayer >= 0 && iLayer < m_AnimOverlay.Count() && m_AnimOverlay[iLayer].IsActive());
}

int IBaseAnimatingOverlay::FindGestureLayer(Activity activity)
{
	for (int i = 0; i < m_AnimOverlay.Count(); i++)
	{
		if ( !(m_AnimOverlay[i].IsActive()) )
			continue;

		if ( m_AnimOverlay[i].IsKillMe() )
			continue;

		if ( m_AnimOverlay[i].m_nActivity == -1 )
			continue;

		if ( m_AnimOverlay[i].m_nActivity == activity )
			return i;
	}

	return -1;
}

void IBaseAnimatingOverlay::VerifyOrder(void)
{
}

void IBaseAnimatingOverlay::SetLayerAutokill(int iLayer, bool bAutokill)
{
	if (!IsValidLayer( iLayer ))
		return;

	if (bAutokill)
	{
		m_AnimOverlay[iLayer].m_fFlags |= ANIM_LAYER_AUTOKILL;
	}
	else
	{
		m_AnimOverlay[iLayer].m_fFlags &= ~ANIM_LAYER_AUTOKILL;
	}
}

void IBaseAnimatingOverlay::SetLayerDuration(int iLayer, float flDuration)
{
	if (IsValidLayer( iLayer ) && flDuration > 0)
	{
		m_AnimOverlay[iLayer].m_flPlaybackRate = SequenceDuration( m_AnimOverlay[iLayer].m_nSequence ) / flDuration;
	}
}

void IBaseAnimatingOverlay::RemoveLayer(int iLayer, float flKillRate, float flKillDelay)
{
	if(!IsValidLayer(iLayer))
		return;

	if(flKillRate > 0)
	{
		m_AnimOverlay[iLayer].m_flKillRate = m_AnimOverlay[iLayer].m_flWeight / flKillRate;
	}
	else
	{
		m_AnimOverlay[iLayer].m_flKillRate = 100;
	}

	m_AnimOverlay[iLayer].m_flKillDelay = flKillDelay;
	m_AnimOverlay[iLayer].KillMe();
}

int IBaseAnimatingOverlay::AllocateLayer(int iPriority)
{
	int i;

	// look for an open slot and for existing layers that are lower priority
	int iNewOrder = 0;
	int iOpenLayer = -1;
	int iNumOpen = 0;
	for (i = 0; i < m_AnimOverlay.Count(); i++)
	{
		if ( m_AnimOverlay[i].IsActive() )
		{
			if (m_AnimOverlay[i].m_nPriority <= iPriority)
			{
				iNewOrder = MAX( iNewOrder, m_AnimOverlay[i].m_nOrder + 1 );
			}
		}
		else if (m_AnimOverlay[ i ].IsDying())
		{
			// skip
		}
		else if (iOpenLayer == -1)
		{
			iOpenLayer = i;
		}
		else
		{
			iNumOpen++;
		}
	}

	if (iOpenLayer == -1)
	{
		if (m_AnimOverlay.Count() >= 15)
		{
			return -1;
		}

		iOpenLayer = m_AnimOverlay.AddToTail();
		m_AnimOverlay[iOpenLayer].Init( this );
	}

	// make sure there's always an empty unused layer so that history slots will be available on the client when it is used
	if (iNumOpen == 0)
	{
		if (m_AnimOverlay.Count() < 15)
		{
			i = m_AnimOverlay.AddToTail();
			m_AnimOverlay[i].Init( this );
		}
	}

	for (i = 0; i < m_AnimOverlay.Count(); i++)
	{
		if ( m_AnimOverlay[i].m_nOrder >= iNewOrder && m_AnimOverlay[i].m_nOrder < 15)
		{
			m_AnimOverlay[i].m_nOrder++;
		}
	}

	m_AnimOverlay[iOpenLayer].m_fFlags = ANIM_LAYER_ACTIVE;
	m_AnimOverlay[iOpenLayer].m_nOrder = iNewOrder;
	m_AnimOverlay[iOpenLayer].m_nPriority = iPriority;

	m_AnimOverlay[iOpenLayer].MarkActive();
	VerifyOrder();

	return iOpenLayer;
}
