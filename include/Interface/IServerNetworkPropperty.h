#ifndef _HEADER_SERVER_NETWORK_PROPERTY_INCLUDE_
#define _HEADER_SERVER_NETWORK_PROPERTY_INCLUDE_

#include <utlpriorityqueue.h>
#include <HL2.h>

class ITimedEventMgr;
class IBaseEntity;
class CBaseEntity;
class edict_t;
class IHandleEntity;
class ServerClass;
class CCheckTransmitInfo;

typedef CHandle<IBaseEntity> IHANDLES;

class IEventRegisterCallback
{
public:
	virtual void FireEvent() = 0;
};

class IEventRegister
{
    friend bool TimedEventMgr_LessFunc( IEventRegister* const &a, IEventRegister* const &b );
    friend class ITimedEventMgr;

public:
	IEventRegister();
	~IEventRegister();

	void Init( ITimedEventMgr *pMgr, IEventRegisterCallback *pCallback );
	
	void SetUpdateInterval( float interval );
	void StopUpdates();

	inline bool IsRegistered() const { return m_bRegistered; }

private:

	void Reregister();
	void Term();


private:
	ITimedEventMgr *m_pEventMgr;
	float m_flNextEventTime;
	float m_flUpdateInterval;
	IEventRegisterCallback *m_pCallback;
	bool m_bRegistered;
};

class ITimedEventMgr
{
friend class IEventRegister;

public:
	ITimedEventMgr();

	void FireEvents();
private:

	void RegisterForNextEvent( IEventRegister *pEvent );
	void RemoveEvent( IEventRegister *pEvent );	
	
private:	
	CUtlPriorityQueue<IEventRegister*> m_Events;
};

class IServerNetworkProperty : public IServerNetworkable, public IEventRegisterCallback
{
public:
	virtual datamap_t 			*GetDataDescMap( void ) { return NULL; };
public:
	IServerNetworkProperty();
	virtual	~IServerNetworkProperty();

public:
	virtual IHandleEntity  		*GetEntityHandle( );
	virtual edict_t				*GetEdict() const;
	virtual CBaseNetworkable	*GetBaseNetworkable();
	virtual CBaseEntity			*GetBaseEntity();
	virtual ServerClass			*GetServerClass();
	virtual const char			*GetClassName() const;
	virtual void				Release();
	virtual int					AreaNum() const;
	virtual PVSInfo_t			*GetPVSInfo();

public:
	void						Init( CBaseEntity *pEntity );
	void						AttachEdict( edict_t *pRequiredEdict = NULL );	
	int							entindex() const;
	edict_t						*edict();
	const edict_t				*edict() const;
	void						SetEdict( edict_t *pEdict );
	void						NetworkStateForceUpdate();
	void						NetworkStateChanged();
	void						NetworkStateChanged( unsigned short offset );
	void						MarkPVSInformationDirty();
	void						MarkForDeletion();
	bool						IsMarkedForDeletion() const;
	void						SetNetworkParent( EHANDLE hParent );
	IServerNetworkProperty		*GetNetworkParent();
	void						SetUpdateInterval( float N );
	bool						IsInPVS( const CCheckTransmitInfo *pInfo );
	bool						IsInPVS( const edict_t *pRecipient, const void *pvs, int pvssize );
	virtual void				FireEvent();
	void						RecomputePVSInformation();

private:
	void						DetachEdict();
	IBaseEntity					*GetOuter();

private:
	IBaseEntity *m_pOuter;			// dword 2
	edict_t	*m_pPev;				// dword 3
	PVSInfo_t m_PVSInfo;			// dword 16
	ServerClass *m_pServerClass;	// dword 12
	IHANDLES m_hParent;				// dword 13
	IEventRegister	m_TimerEvent;	// dword 18
	bool m_bPendingStateChange : 1;	// dword 19
};

inline CBaseNetworkable* IServerNetworkProperty::GetBaseNetworkable()
{
	return NULL;
}

inline CBaseEntity* IServerNetworkProperty::GetBaseEntity()
{
	return (CBaseEntity *)m_pOuter;
}

inline IBaseEntity *IServerNetworkProperty::GetOuter()
{
	return m_pOuter;
}

inline PVSInfo_t *IServerNetworkProperty::GetPVSInfo()
{
	return &m_PVSInfo;
}

inline void IServerNetworkProperty::MarkPVSInformationDirty()
{
	if ( m_pPev )
	{
		m_pPev->m_fStateFlags |= FL_EDICT_DIRTY_PVS_INFORMATION;
	}
}

inline void IServerNetworkProperty::SetNetworkParent( EHANDLE hParent )
{
	m_hParent = hParent;
}

inline void IServerNetworkProperty::NetworkStateForceUpdate()
{ 
	if ( m_pPev )
		m_pPev->StateChanged();
}

inline void IServerNetworkProperty::NetworkStateChanged()
{ 
	if ( m_TimerEvent.IsRegistered() )
	{
		m_bPendingStateChange = true;
	}
	else
	{
		if ( m_pPev )
			m_pPev->StateChanged();
	}
}

inline void IServerNetworkProperty::NetworkStateChanged( unsigned short varOffset )
{ 
	if ( m_TimerEvent.IsRegistered() )
	{
		m_bPendingStateChange = true;
	}
	else
	{
		if ( m_pPev )
			m_pPev->StateChanged( varOffset );
	}
}

inline int IServerNetworkProperty::entindex() const
{
	return g_Sample.IndexOfEdict( m_pPev );
}

inline edict_t* IServerNetworkProperty::GetEdict() const
{
	return m_pPev;
}

inline edict_t *IServerNetworkProperty::edict()
{
	return m_pPev;
}

inline const edict_t *IServerNetworkProperty::edict() const
{
	return m_pPev;
}

inline void IServerNetworkProperty::SetEdict( edict_t *pEdict )
{
	m_pPev = pEdict;
}

inline int IServerNetworkProperty::AreaNum() const
{
	const_cast<IServerNetworkProperty*>(this)->RecomputePVSInformation();
	return m_PVSInfo.m_nAreaNum;
}

#endif