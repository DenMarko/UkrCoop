#ifndef _INCLUDE_BASE_TEMP_ENTITY_PROPER_H_
#define _INCLUDE_BASE_TEMP_ENTITY_PROPER_H_
#include <edict.h>

class IRecipientFilter;

class IBaseTempEntity
{
public:
	const char			*GetName(void);
	IBaseTempEntity		*GetNext(void);

public:
	virtual ServerClass *GetServerClass() = 0;
	virtual int			YouForgotToImplementOrDeclareServerClass() = 0;
	virtual ~IBaseTempEntity() {};

	virtual void		Test( const Vector& current_origin, const QAngle& current_angles ) = 0;
	virtual	void		Create( IRecipientFilter& filter, float delay = 0.0 ) = 0;
	virtual void		Precache( void ) = 0;
private:
	const char			*m_pszName;	// 1
	IBaseTempEntity		*m_pNext;	// 2
};


class CEffectData
{
public:
	Vector m_vOrigin;			// 0
	Vector m_vStart;			// 12
	Vector m_vNormal;			// 24
	QAngle m_vAngles;			// 36
	int		m_fFlags;			// 48
	int		m_nEntIndex;		// 52
	float	m_flScale;			// 56
	float	m_flMagnitude;		// 60
	float	m_flRadius;			// 64
	int		m_nAttachmentIndex;	// 68
	short	m_nSurfaceProp;		// 72

	int		m_nMaterial;		// 76
	int		m_nDamageType;		// 80
	int		m_nHitBox;			// 84
	
	unsigned char	m_nColor;	// 88

public:
	CEffectData()
	{
		m_vOrigin.Init();
		m_vStart.Init();
		m_vNormal.Init();
		m_vAngles.Init();

		m_fFlags = 0;
		m_nEntIndex = 0;
		m_flScale = 1.f;
		m_nAttachmentIndex = 0;
		m_nSurfaceProp = 0;

		m_flMagnitude = 0.0f;
		m_flRadius = 0.0f;

		m_nMaterial = 0;
		m_nDamageType = 0;
		m_nHitBox = 0;

		m_nColor = 0;
	}

	int GetEffectNameIndex() { return m_iEffectName; }
public:
	int m_iEffectName;			// 92
};

class ITEEffectDispatch : public IBaseTempEntity
{
public:
	virtual ServerClass *GetServerClass() = 0;
	virtual int			YouForgotToImplementOrDeclareServerClass() = 0;
    virtual ~ITEEffectDispatch() {}
public:
    CEffectData m_EffectData;
};

class ITEBaseBeam : public IBaseTempEntity
{
public:
	virtual ServerClass *GetServerClass() = 0;
	virtual int			YouForgotToImplementOrDeclareServerClass() = 0;
    virtual ~ITEBaseBeam() {}

	virtual void		Test( const Vector& current_origin, const QAngle& current_angles ) = 0;
public:
	int         m_nModelIndex;	// 3
	int         m_nHaloIndex;	// 4
	int         m_nStartFrame;	// 5
	int         m_nFrameRate;	// 6
	float       m_fLife;		// 7
	float       m_fWidth;		// 8
	float       m_fEndWidth;	// 9
	int         m_nFadeLength;	// 10
	float       m_fAmplitude;	// 11
	int         r;				// 12
	int         g;				// 13
	int         b;				// 14
	int         a;				// 15
	int         m_nSpeed;		// 16
	int         m_nFlags;		// 17
};

class ITEBeamPoints : public ITEBaseBeam
{
public:
	virtual ServerClass *GetServerClass() = 0;
	virtual int			YouForgotToImplementOrDeclareServerClass() = 0;
    virtual ~ITEBeamPoints() {}

	virtual void		Test( const Vector& current_origin, const QAngle& current_angles ) = 0;
public:
    Vector      m_vecStartPoint;	// 18
    Vector      m_vecEndPoint;		// 21
};

class ITESmoke : public IBaseTempEntity
{
	virtual ServerClass *GetServerClass() = 0;
	virtual int			YouForgotToImplementOrDeclareServerClass() = 0;
    virtual ~ITESmoke() {}

	virtual void		Test( const Vector& current_origin, const QAngle& current_angles ) = 0;
public:
	Vector      m_vecOrigin;		// 3
	int         m_nModelIndex;		// 6
	float       m_fScale;			// 7
	int         m_nFrameRate;		// 8
};

#endif