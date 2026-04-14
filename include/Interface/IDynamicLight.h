#ifndef _INCLUDE_DINAMIC_LIGHT_H_
#define _INCLUDE_DINAMIC_LIGHT_H_
#include "IBaseEntity.h"

class IDynamicLight : public IBaseEntity
{
public:
	typedef IDynamicLight ThisClass;

	inline void SetExponent(int val)
	{
		m_Exponent = val;
	}
	inline void SetSpotRadius(float val)
	{
		m_SpotRadius = val;
	}
	inline void SetRadius(float val)
	{
		m_Radius = val;
	}
	inline void SetLightStyle(unsigned char val)
	{
		m_LightStyle = val;
	}
	inline void Set_Flags(unsigned char val)
	{
		m_Flags = val;
	}
	inline void SetOn(bool val)
	{
		m_On = val;
	}
	inline unsigned char GetActualFlags()
	{
		return m_ActualFlags;
	}
public:
	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;

public:
	unsigned char m_ActualFlags;
	CNetworkVar(unsigned char, m_Flags);
	CNetworkVar(unsigned char, m_LightStyle);
	bool	m_On;
	CNetworkVar(float, m_Radius);
	CNetworkVar(int, m_Exponent);
	CNetworkVar(float, m_InnerAngle);
	CNetworkVar(float, m_OuterAngle);
	CNetworkVar(float, m_SpotRadius);
};

#endif