#ifndef _INCLUDE_ENTITY_DISSOLVE_H_
#define _INCLUDE_ENTITY_DISSOLVE_H_
#include "IBaseEntity.h"

class IEntityDissolve : public IBaseEntity
{
public:
	typedef IEntityDissolve ThisClass;

    virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;

    void    SetStartTime( float flStartTime );
	void	AttachToEntity( IBaseEntity *pTarget );
	void	SetDissolverOrigin( Vector vOrigin ) { m_vDissolverOrigin = vOrigin; }
	void	SetMagnitude( int iMagnitude ){ m_nMagnitude = iMagnitude; }
	void	SetDissolveType( int iType ) { m_nDissolveType = iType;	}

	Vector	GetDissolverOrigin( void ) 
	{
		Vector vReturn = m_vDissolverOrigin; 
		return vReturn;	
	}
	int		GetMagnitude( void ) { return m_nMagnitude;	}
	int		GetDissolveType( void ) { return m_nDissolveType;	}

    CNetworkVar(float, m_flStartTime);
	CNetworkVar(float, m_flFadeInStart);
	CNetworkVar(float, m_flFadeInLength);
	CNetworkVar(float, m_flFadeOutModelStart);
	CNetworkVar(float, m_flFadeOutModelLength);
	CNetworkVar(float, m_flFadeOutStart);
	CNetworkVar(float, m_flFadeOutLength);
	CNetworkVar(int, m_nDissolveType);
private:
	CNetworkVector(m_vDissolverOrigin);
	CNetworkVar(int, m_nMagnitude);

};

namespace EntDissolve
{
    enum
    {
        ENTITY_DISSOLVE_NORMAL = 0,
        ENTITY_DISSOLVE_ELECTRICAL,
        ENTITY_DISSOLVE_ELECTRICAL_LIGHT,
        ENTITY_DISSOLVE_CORE,

        ENTITY_DISSOLVE_BITS = 3
    };

    IEntityDissolve	*Create( IBaseEntity *pTarget, const char *pMaterialName, float flStartTime, int nDissolveType = 0, bool *pRagdollCreated = NULL );
    IEntityDissolve	*Create( IBaseEntity *pTarget, IBaseEntity *pSource );
}

#endif //_INCLUDE_ENTITY_DISSOLVE_H_
