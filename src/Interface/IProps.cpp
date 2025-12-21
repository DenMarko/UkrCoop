#include "IProps.h"

Vector Pickup_DefaultPhysGunLaunchVelocity( const Vector &vecForward, float flMass )
{
	// Do the simple calculation
	return ( vecForward * flMass );
}

void IBreakableProp::DisableAutoFade()
{
	SetGlobalFadeScale(0.f);
	m_flDefaultFadeScale = 0;
}
