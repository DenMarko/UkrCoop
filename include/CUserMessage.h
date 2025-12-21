#ifndef _INCLUDE_USER_MESSAGE_PROPER_H_
#define _INCLUDE_USER_MESSAGE_PROPER_H_

#include "HL2.h"
#include "Interface/IBasePlayer.h"

class IBasePlayer;
class ITeam;

class MyRecipientFilter : public IRecipientFilter
{
public:
	MyRecipientFilter();
	virtual 		~MyRecipientFilter();

	virtual bool	IsReliable( void ) const;
	virtual bool	IsInitMessage( void ) const;

	virtual int		GetRecipientCount( void ) const;
	virtual int		GetRecipientIndex( int slot ) const;

public:

	void			CopyFrom( const MyRecipientFilter& src );

	void			Reset( void );

	void			MakeInitMessage( void );

	void			MakeReliable( void );
	
	void			AddAllPlayers( void );
	void			AddRecipientsByPVS( const Vector& origin );
	void			RemoveRecipientsByPVS( const Vector& origin );
	void			AddRecipientsByPAS( const Vector& origin );
	void			AddRecipient( const IBasePlayer *player );
	void			RemoveAllRecipients( void );
	void			RemoveRecipient( IBasePlayer *player );
	void			RemoveRecipientByPlayerIndex( int playerindex );
	void			AddRecipientsByTeam( ITeam *team );
	void			RemoveRecipientsByTeam( ITeam *team );
	void			RemoveRecipientsNotOnTeam( ITeam *team );

	//void			UsePredictionRules( void );
	bool			IsUsingPredictionRules( void ) const;

	bool			IgnorePredictionCull( void ) const;
	void			SetIgnorePredictionCull( bool ignore );

	void			AddPlayersFromBitMask( CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits );
	void			RemovePlayersFromBitMask( CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits );

private:

	bool				m_bReliable;
	bool				m_bInitMessage;
	CUtlVector< int >	m_Recipients;
	bool				m_bUsingPredictionRules;
	bool				m_bIgnorePredictionCull;
};

class IPASFilter : public MyRecipientFilter
{
public:
	IPASFilter( void )
	{
	}

	IPASFilter( const Vector& origin )
	{
		AddRecipientsByPAS( origin );
	}
};

class IPVSFilter : public MyRecipientFilter
{
public:
	IPVSFilter( const Vector& origin )
	{
		AddRecipientsByPVS( origin );
	}
};


class CPASAttenuationFilter : public IPASFilter
{
public:
	CPASAttenuationFilter( void )
	{
	}

	CPASAttenuationFilter( IBaseEntity *entity, soundlevel_t soundlevel ) :	IPASFilter( static_cast<const Vector&>(entity->GetSoundEmissionOrigin()) )
	{
		Filter( entity->GetSoundEmissionOrigin(), SNDLVL_TO_ATTN( soundlevel ) );
	}

	CPASAttenuationFilter( IBaseEntity *entity, float attenuation = ATTN_NORM ) : IPASFilter( static_cast<const Vector&>(entity->GetSoundEmissionOrigin()) )
	{
		Filter( entity->GetSoundEmissionOrigin(), attenuation );
	}

	CPASAttenuationFilter( const Vector& origin, soundlevel_t soundlevel ) : IPASFilter( origin )
	{
		Filter( origin, SNDLVL_TO_ATTN( soundlevel ) );
	}

	CPASAttenuationFilter( const Vector& origin, float attenuation = ATTN_NORM ) : IPASFilter( origin )
	{
		Filter( origin, attenuation );
	}

	CPASAttenuationFilter( IBaseEntity *entity, const char *lookupSound ) : IPASFilter( static_cast<const Vector&>(entity->GetSoundEmissionOrigin()) )
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevel( lookupSound );
		float attenuation = SNDLVL_TO_ATTN( level );
		Filter( entity->GetSoundEmissionOrigin(), attenuation );
	}

	CPASAttenuationFilter( const Vector& origin, const char *lookupSound ) : IPASFilter( origin )
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevel( lookupSound );
		float attenuation = SNDLVL_TO_ATTN( level );
		Filter( origin, attenuation );
	}

	CPASAttenuationFilter( IBaseEntity *entity, const char *lookupSound, HSOUNDSCRIPTHANDLE& handle ) : IPASFilter( static_cast<const Vector&>(entity->GetSoundEmissionOrigin()) )
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevelByHandle( lookupSound, handle );
		float attenuation = SNDLVL_TO_ATTN( level );
		Filter( entity->GetSoundEmissionOrigin(), attenuation );
	}

	CPASAttenuationFilter( const Vector& origin, const char *lookupSound, HSOUNDSCRIPTHANDLE& handle ) : IPASFilter( origin )
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevelByHandle( lookupSound, handle );
		float attenuation = SNDLVL_TO_ATTN( level );
		Filter( origin, attenuation );
	}

public:
	void Filter( const Vector& origin, float attenuation = ATTN_NORM );
};


class CUserRecipientFilter : public IRecipientFilter
{
private:
    int m_Player[255];
    bool m_IsReliable;
    bool m_IsInitMessage;
    int m_Size;
public:
    CUserRecipientFilter(const int *ptr, size_t count);
    ~CUserRecipientFilter() {}

    void MakeReliable()
    {
        m_IsReliable = true;
    }

    void MakeInitMessage()
    {
        m_IsInitMessage = true;
    }

public:
	bool IsReliable( void ) const
    {
        return m_IsReliable;
    }
	bool IsInitMessage( void ) const
    {
        return m_IsInitMessage;
    }
	int GetRecipientCount( void ) const
    {
        return m_Size;
    }
	int GetRecipientIndex( int slot ) const;
};

class CUserMessage
{
private:
    bf_write *g_pMsgBuffer;
    int msg_type;
public:
    CUserMessage(IRecipientFilter &filter, const char* messagename);
    ~CUserMessage();

    int GetMsgType();

    void MsgWriteByte(int iVal);
    void MsgWriteChar(int iVal);
    void MsgWriteShort(int iVal);
    void MsgWriteWord(int iVal);
    void MsgWriteLong(int iVal);
    void MsgWriteEntity( int iValue);

    void MsgWriteFloat(float fVal);
    void MsgWriteAngle(float fVal);
    void MsgWriteCoord(float fVal);

    void MsgWriteVec3Coord( const Vector& rgflValue);
    void MsgWriteVec3Normal( const Vector& rgflValue);

    void MsgWriteAngles( const QAngle& rgflValue);

    void MsgWriteString( const char *sz );
    void MsgWriteEHandle( CBaseEntity *pEntity );
    void MsgWriteBool( bool bValue );
    void MsgWriteUBitLong( unsigned int data, int numbits );
    void MsgWriteSBitLong( int data, int numbits );
    void MsgWriteBits( const void *pIn, int nBits );
};


#endif