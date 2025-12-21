#ifndef _INCLUDE_BASE_ANIMATING_OVERLAY_H_
#define _INCLUDE_BASE_ANIMATING_OVERLAY_H_

#include "IBaseAnimating.h"

class IBaseAnimatingOverlay;

class IAnimationLayer
{
public:	
	DECLARE_CLASS_NOBASE( IAnimationLayer );
	
	IAnimationLayer( void );
	void	Init( IBaseAnimatingOverlay *pOverlay );

	// float	SetBlending( int iBlender, float flValue, CBaseAnimating *pOwner );
	void	StudioFrameAdvance( float flInterval, IBaseAnimating *pOwner );
	void	DispatchAnimEvents( IBaseAnimating *eventHandler, IBaseAnimating *pOwner );
	void	SetOrder( int nOrder );

	float 	GetFadeout( float flCurTime );

	// For CNetworkVars.
	void 	NetworkStateChanged();
	void 	NetworkStateChanged( void *pVar );

public:	

#define ANIM_LAYER_ACTIVE		0x0001
#define ANIM_LAYER_AUTOKILL		0x0002
#define ANIM_LAYER_KILLME		0x0004
#define ANIM_LAYER_DONTRESTORE	0x0008
#define ANIM_LAYER_CHECKACCESS	0x0010
#define ANIM_LAYER_DYING		0x0020

	int		m_fFlags;

	bool	m_bSequenceFinished;
	bool	m_bLooping;
	
	int 	m_nSequence;
	float 	m_flCycle;
	float 	m_flPrevCycle;
	float 	m_flWeight;
	
	float	m_flPlaybackRate;

	float	m_flBlendIn; // start and end blend frac (0.0 for now blend)
	float	m_flBlendOut; 

	float	m_flKillRate;
	float	m_flKillDelay;

	float	m_flLayerAnimtime;
	float	m_flLayerFadeOuttime;

	// For checking for duplicates
	Activity	m_nActivity;

	// order of layering on client
	int		m_nPriority;
	int		m_nOrder;

	bool	IsActive( void ) { return ((m_fFlags & ANIM_LAYER_ACTIVE) != 0); }
	bool	IsAutokill( void ) { return ((m_fFlags & ANIM_LAYER_AUTOKILL) != 0); }
	bool	IsKillMe( void ) { return ((m_fFlags & ANIM_LAYER_KILLME) != 0); }
	bool	IsAutoramp( void ) { return (m_flBlendIn != 0.0 || m_flBlendOut != 0.0); }
	void	KillMe( void ) { m_fFlags |= ANIM_LAYER_KILLME; }
	void	Dying( void ) { m_fFlags |= ANIM_LAYER_DYING; }
	bool	IsDying( void ) { return ((m_fFlags & ANIM_LAYER_DYING) != 0); }
	void	Dead( void ) { m_fFlags &= ~ANIM_LAYER_DYING; }

	bool	IsAbandoned( void );
	void	MarkActive( void );

	float	m_flLastEventCheck;

	float	m_flLastAccess;

	// Network state changes get forwarded here.
	IBaseAnimatingOverlay *m_pOwnerEntity;
};

inline float IAnimationLayer::GetFadeout( float flCurTime )
{
	float s;

	if (m_flLayerFadeOuttime <= 0.0f)
	{
		s = 0;
	}
	else
	{
		s = 1.0 - (flCurTime - m_flLayerAnimtime) / m_flLayerFadeOuttime;
		if (s > 0 && s <= 1.0)
		{
			s = 3 * s * s - 2 * s * s * s;
		}
		else if ( s > 1.0f )
		{
			s = 1.0f;
		}
	}
	return s;
}

class IBaseAnimatingOverlay : public IBaseAnimating
{
public:
	DECLARE_CLASS(IBaseAnimatingOverlay, IBaseAnimating);
	enum 
	{
		MAX_OVERLAYS = 15,
	};

	int		AddGestureSequence( int sequence, bool autokill = true );
	int		AddGestureSequence( int sequence, float flDuration, bool autokill = true );
	int		AddGesture( Activity activity, bool autokill = true );
	int		AddGesture( Activity activity, float flDuration, bool autokill = true );
	void	RemoveGesture( Activity activity );
	void	RemoveAllGestures( void );
	bool	IsPlayingGesture( Activity activity );
	int		AddLayeredSequence( int sequence, int iPriority );
	bool	IsValidLayer( int iLayer );

	int		FindGestureLayer( Activity activity );
	void 	VerifyOrder( void );
	void	SetLayerAutokill( int iLayer, bool bAutokill );
	void	SetLayerDuration( int iLayer, float flDuration );

	void	RemoveLayer( int iLayer, float flKillRate = 0.2, float flKillDelay = 0.0 );
private:
	int		AllocateLayer( int iPriority = 0 ); // lower priorities are processed first

public:
	virtual ~IBaseAnimatingOverlay() {}

	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual	datamap_t*		GetDataDescMap(void) = 0;
	virtual void 			SetModel( const char *szModelName ) = 0;
	virtual void 			OnRestore() = 0;
	virtual void			StudioFrameAdvance() = 0;
	virtual void			GetSkeleton( CStudioHdr *pStudioHdr, Vector pos[], Quaternion q[], int boneMask ) = 0;
	virtual	void			DispatchAnimEvents ( CBaseAnimating *eventHandler ) = 0;

private:
	CUtlVector< IAnimationLayer > m_AnimOverlay;		// 1168
};

inline void IAnimationLayer::SetOrder( int nOrder )
{
	m_nOrder = nOrder;
}

inline void IAnimationLayer::NetworkStateChanged()
{
	if ( m_pOwnerEntity )
		m_pOwnerEntity->NetworkStateChanged();
}

inline void IAnimationLayer::NetworkStateChanged( void *pVar )
{
	if ( m_pOwnerEntity )
		m_pOwnerEntity->NetworkStateChanged();
}


#endif