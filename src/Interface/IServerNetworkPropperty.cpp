#include "IServerNetworkPropperty.h"
#include "IBaseEntity.h"

ITimedEventMgr g_NetworkPropertyEventMgr;

IServerNetworkProperty::IServerNetworkProperty()
{
	Init(NULL);
}

IServerNetworkProperty::~IServerNetworkProperty()
{
    engine->CleanUpEntityClusterList(&m_PVSInfo);
    DetachEdict();
}

IHandleEntity *IServerNetworkProperty::GetEntityHandle()
{
    return (IHandleEntity *)m_pOuter;
}

ServerClass *IServerNetworkProperty::GetServerClass()
{
    if ( !m_pServerClass )
        m_pServerClass = m_pOuter->GetServerClass();
    return m_pServerClass;
}

const char *IServerNetworkProperty::GetClassName() const
{
    return m_pOuter->GetClassname().ToCStr();
}

void IServerNetworkProperty::Release()
{
    delete m_pOuter;
}

void IServerNetworkProperty::Init(CBaseEntity *pEntity)
{
    m_pPev = NULL;
    m_pOuter = (IBaseEntity *)pEntity;
    m_pServerClass = NULL;
//	m_pTransmitProxy = NULL;
    m_bPendingStateChange = false;
    m_PVSInfo.m_nClusterCount = 0;
    m_TimerEvent.Init( &g_NetworkPropertyEventMgr, this );
}

void IServerNetworkProperty::AttachEdict(edict_t *pRequiredEdict)
{
    if ( !pRequiredEdict )
    {
        pRequiredEdict = engine->CreateEdict();
    }

    m_pPev = pRequiredEdict;
    m_pPev->SetEdict( (IServerUnknown *)GetBaseEntity(), true );
}

void IServerNetworkProperty::MarkForDeletion()
{
    m_pOuter->AddEFlags( EFL_KILLME );
}

bool IServerNetworkProperty::IsMarkedForDeletion() const
{
    return ( m_pOuter->GetEFlags() & EFL_KILLME ) != 0;
}

IServerNetworkProperty *IServerNetworkProperty::GetNetworkParent()
{
    IBaseEntity *pParent = m_hParent.Get();
    return pParent ? (IServerNetworkProperty *)pParent->GetNetworkable() : NULL;
}

void IServerNetworkProperty::SetUpdateInterval(float N)
{
	if ( N == 0 )
		m_TimerEvent.StopUpdates();
	else
		m_TimerEvent.SetUpdateInterval( N );
}

bool IServerNetworkProperty::IsInPVS(const CCheckTransmitInfo *pInfo)
{	
	int i;

	if ( !m_PVSInfo.m_nAreaNum2 )
	{
		for ( i=0; i< pInfo->m_AreasNetworked; i++ )
		{
			int clientArea = pInfo->m_Areas[i];
			if ( clientArea == m_PVSInfo.m_nAreaNum || engine->CheckAreasConnected( clientArea, m_PVSInfo.m_nAreaNum ) )
				break;
		}
	}
	else
	{
		for ( i=0; i< pInfo->m_AreasNetworked; i++ )
		{
			int clientArea = pInfo->m_Areas[i];
			if ( clientArea == m_PVSInfo.m_nAreaNum || clientArea == m_PVSInfo.m_nAreaNum2 )
				break;

			if ( engine->CheckAreasConnected( clientArea, m_PVSInfo.m_nAreaNum ) )
				break;

			if ( engine->CheckAreasConnected( clientArea, m_PVSInfo.m_nAreaNum2 ) )
				break;
		}
	}

	if ( i == pInfo->m_AreasNetworked )
	{
		return false;
	}

	unsigned char *pPVS = ( unsigned char * )pInfo->m_PVS;
	
	if ( m_PVSInfo.m_nClusterCount < 0 )
	{
		return (engine->CheckHeadnodeVisible( m_PVSInfo.m_nHeadNode, pPVS, pInfo->m_nPVSSize ) != 0);
	}
	
	for ( i = m_PVSInfo.m_nClusterCount; --i >= 0; )
	{
		int nCluster = m_PVSInfo.m_pClusters[i];
		if ( ((int)(pPVS[nCluster >> 3])) & BitVec_BitInByte( nCluster ) )
			return true;
	}

	return false;
}

bool IServerNetworkProperty::IsInPVS(const edict_t *pRecipient, const void *pvs, int pvssize)
{
	RecomputePVSInformation();
	unsigned char *pPVS = ( unsigned char * )pvs;
	
	if ( m_PVSInfo.m_nClusterCount < 0 )
	{
		return ( engine->CheckHeadnodeVisible( m_PVSInfo.m_nHeadNode, pPVS, pvssize ) != 0);
	}
	
	for ( int i = m_PVSInfo.m_nClusterCount; --i >= 0; )
	{
		if (pPVS[m_PVSInfo.m_pClusters[i] >> 3] & (1 << (m_PVSInfo.m_pClusters[i] & 7) ))
			return true;
	}

	return false;
}

void IServerNetworkProperty::FireEvent()
{
	if ( m_bPendingStateChange )
	{
		m_pPev->StateChanged();
		m_bPendingStateChange = false;
	}
}

void IServerNetworkProperty::RecomputePVSInformation()
{
    if ( m_pPev && ( ( m_pPev->m_fStateFlags & FL_EDICT_DIRTY_PVS_INFORMATION ) != 0 ) )
    {
        m_pPev->m_fStateFlags &= ~FL_EDICT_DIRTY_PVS_INFORMATION;
        engine->BuildEntityClusterList( edict(), &m_PVSInfo );
    }
}

void IServerNetworkProperty::DetachEdict()
{
    if ( m_pPev )
    {
        m_pPev->SetEdict( NULL, false );
        engine->RemoveEdict( m_pPev );
        m_pPev = NULL;
    }
}

IEventRegister::IEventRegister()
{
	m_bRegistered = false;
	m_pEventMgr = NULL;
}

IEventRegister::~IEventRegister()
{
	Term();
}

void IEventRegister::Init(ITimedEventMgr *pMgr, IEventRegisterCallback *pCallback)
{
	Term();
	m_pEventMgr = pMgr;
	m_pCallback = pCallback;
}

void IEventRegister::SetUpdateInterval(float interval)
{
	if ( m_pEventMgr )
	{
		m_flUpdateInterval = interval;
		m_flNextEventTime = g_pGlobals->curtime + m_flUpdateInterval;

		m_pEventMgr->RegisterForNextEvent( this );
	}
}

void IEventRegister::StopUpdates()
{
	if ( m_pEventMgr )
	{
		m_pEventMgr->RemoveEvent( this );
	}
}

void IEventRegister::Reregister()
{
	if ( m_flUpdateInterval > 1e-6 && m_pEventMgr )
	{
		while ( m_flNextEventTime <= g_pGlobals->curtime )
		{
			m_flNextEventTime += m_flUpdateInterval;
		}

		m_pEventMgr->RegisterForNextEvent( this );
	}
}

void IEventRegister::Term()
{
	if ( m_pEventMgr && m_bRegistered )
	{
		m_pEventMgr->RemoveEvent( this );
	}
}

bool TimedEventMgr_LessFunc( IEventRegister* const &a, IEventRegister* const &b )
{
	return a->m_flNextEventTime > b->m_flNextEventTime;
}

ITimedEventMgr::ITimedEventMgr()
{
	m_Events.SetLessFunc( TimedEventMgr_LessFunc );
}

void ITimedEventMgr::FireEvents()
{
	while ( m_Events.Count() )
	{
		IEventRegister *pEvent = m_Events.ElementAtHead();
		if ( g_pGlobals->curtime >= pEvent->m_flNextEventTime )
		{
			m_Events.RemoveAtHead();
			pEvent->m_bRegistered = false;
			pEvent->Reregister();

			pEvent->m_pCallback->FireEvent();
		}
		else
		{
			break;
		}
	}
}

void ITimedEventMgr::RegisterForNextEvent(IEventRegister *pEvent)
{
	RemoveEvent( pEvent );
	m_Events.Insert( pEvent );
	pEvent->m_bRegistered = true;
}

void ITimedEventMgr::RemoveEvent(IEventRegister *pEvent)
{
	if ( pEvent->m_bRegistered )
	{
		int cnt = m_Events.Count();
		for ( int i=0; i < cnt; i++ )
		{
			if ( m_Events.Element( i ) == pEvent )
			{
				m_Events.RemoveAt( i );
				break;
			}
		}
	}
}
