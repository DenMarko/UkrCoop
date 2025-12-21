#include "CUserMessage.h"
#include "Interface/ITeam.h"

#define IS_BIT_BUF_WALID(...)  \
    if(!g_pMsgBuffer) \
    { \
        Msg(__VA_ARGS__); \
        return; \
    }

CUserRecipientFilter::CUserRecipientFilter(const int* ptr, size_t count)
{
    m_IsReliable = false;
    m_IsInitMessage = false;

    memcpy(m_Player, ptr, count * sizeof(int));
    m_Size = count;
}

int CUserRecipientFilter::GetRecipientIndex(int slot) const
{
    if((slot < 0) || (slot >= GetRecipientCount()))
    {
        return -1;
    }
    return static_cast<int>(m_Player[slot]);
}

CUserMessage::CUserMessage(IRecipientFilter &filter, const char *messagename)
{
    msg_type = usermsgs->GetMessageIndex(messagename);
    if(msg_type != -1)
    {
        g_pMsgBuffer = engine->UserMessageBegin(static_cast<IRecipientFilter *>(&filter), msg_type, messagename);
    } else {
        g_pMsgBuffer = nullptr;
        Error("CUserMessage: Unregister message '%s'\n", messagename);
    }
}

CUserMessage::~CUserMessage()
{
    if(g_pMsgBuffer)
    {
        engine->MessageEnd();
    }
}

int CUserMessage::GetMsgType()
{
    return msg_type;
}

void CUserMessage::MsgWriteByte(int iVal)
{
    IS_BIT_BUF_WALID("WRITE_BYTE called with no active message\n")

    g_pMsgBuffer->WriteByte(iVal);
}

void CUserMessage::MsgWriteChar(int iVal)
{
    IS_BIT_BUF_WALID("WRITE_CHAR called with no active message\n")

    g_pMsgBuffer->WriteChar(iVal);
}

void CUserMessage::MsgWriteShort(int iVal)
{
    IS_BIT_BUF_WALID("WRITE_SHORT called with no active message\n")

    g_pMsgBuffer->WriteShort(iVal);
}

void CUserMessage::MsgWriteWord(int iVal)
{
    IS_BIT_BUF_WALID("WRITE_WORD called with no active message\n")

    g_pMsgBuffer->WriteWord(iVal);
}

void CUserMessage::MsgWriteLong(int iVal)
{
    IS_BIT_BUF_WALID("WriteLong called with no active message\n")

    g_pMsgBuffer->WriteLong(iVal);
}

void CUserMessage::MsgWriteEntity(int iValue)
{
    IS_BIT_BUF_WALID("WriteEntity called with no active message\n")

    g_pMsgBuffer->WriteShort(iValue);
}

void CUserMessage::MsgWriteFloat(float fVal)
{
    IS_BIT_BUF_WALID("WriteFloat called with no active message\n")

    g_pMsgBuffer->WriteFloat(fVal);
}

void CUserMessage::MsgWriteAngle(float fVal)
{
    IS_BIT_BUF_WALID("WriteAngle called with no active message\n")

    g_pMsgBuffer->WriteBitAngle(fVal, 8);
}

void CUserMessage::MsgWriteCoord(float fVal)
{
    IS_BIT_BUF_WALID("WriteCoord called with no active message\n")

    g_pMsgBuffer->WriteBitCoord(fVal);
}

void CUserMessage::MsgWriteVec3Coord(const Vector &rgflValue)
{
    IS_BIT_BUF_WALID("WriteVec3Coord called with no active message\n")

    g_pMsgBuffer->WriteBitVec3Coord(rgflValue);
}

void CUserMessage::MsgWriteVec3Normal(const Vector &rgflValue)
{
    IS_BIT_BUF_WALID("WriteVec3Normal called with no active message\n")

    g_pMsgBuffer->WriteBitVec3Normal(rgflValue);
}

void CUserMessage::MsgWriteAngles(const QAngle &rgflValue)
{
    IS_BIT_BUF_WALID("WriteVec3Normal called with no active message\n")

    g_pMsgBuffer->WriteBitAngles(rgflValue);
}

void CUserMessage::MsgWriteString(const char *sz)
{
    IS_BIT_BUF_WALID("WriteString called with no active message\n")

    g_pMsgBuffer->WriteString(sz);
}

void CUserMessage::MsgWriteEHandle(CBaseEntity *pEntity)
{
    IS_BIT_BUF_WALID("WriteEHandle called with no active message\n")

    long iEncodedEHandle;
    if(pEntity)
    {
        EHANDLE hEnt = pEntity;

        int iSerialNum = hEnt.GetSerialNumber() & ((1 << NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS) - 1);
        iEncodedEHandle = hEnt.GetEntryIndex() | (iSerialNum << MAX_EDICT_BITS);
    }
    else
    {
        iEncodedEHandle = INVALID_NETWORKED_EHANDLE_VALUE;
    }
    g_pMsgBuffer->WriteLong(iEncodedEHandle);
}

void CUserMessage::MsgWriteBool(bool bValue)
{
    IS_BIT_BUF_WALID("WriteBool called with no active message\n")

    g_pMsgBuffer->WriteOneBit(bValue ? 1 : 0);
}

void CUserMessage::MsgWriteUBitLong(unsigned int data, int numbits)
{
    IS_BIT_BUF_WALID("WriteUBitLong called with no active message\n")

    g_pMsgBuffer->WriteUBitLong(data, numbits);
}

void CUserMessage::MsgWriteSBitLong(int data, int numbits)
{
    IS_BIT_BUF_WALID("WriteSBitLong called with no active message\n")

    g_pMsgBuffer->WriteSBitLong(data, numbits);
}

void CUserMessage::MsgWriteBits(const void *pIn, int nBits)
{
    IS_BIT_BUF_WALID("WriteBits called with no active message\n")

    g_pMsgBuffer->WriteBits(pIn, nBits);
}

MyRecipientFilter::MyRecipientFilter()
{
    Reset();
}

MyRecipientFilter::~MyRecipientFilter()
{
}

void MyRecipientFilter::CopyFrom(const MyRecipientFilter &src)
{
	m_bReliable = src.IsReliable();
	m_bInitMessage = src.IsInitMessage();

	m_bUsingPredictionRules = src.IsUsingPredictionRules();
	m_bIgnorePredictionCull = src.IgnorePredictionCull();

	int c = src.GetRecipientCount();
	for ( int i = 0; i < c; ++i )
	{
		m_Recipients.AddToTail( src.GetRecipientIndex( i ) );
	}
}

void MyRecipientFilter::Reset( void )
{
	m_bReliable			= false;
	m_bInitMessage		= false;
	m_Recipients.RemoveAll();
	m_bUsingPredictionRules = false;
	m_bIgnorePredictionCull = false;
}

void MyRecipientFilter::MakeReliable( void )
{
	m_bReliable = true;
}

bool MyRecipientFilter::IsReliable( void ) const
{
	return m_bReliable;
}

int MyRecipientFilter::GetRecipientCount( void ) const
{
	return m_Recipients.Size();
}

int	MyRecipientFilter::GetRecipientIndex( int slot ) const
{
	if ( slot < 0 || slot >= GetRecipientCount() )
		return -1;

	return m_Recipients[ slot ];
}

void MyRecipientFilter::AddAllPlayers( void )
{
	m_Recipients.RemoveAll();

	int i;
	for ( i = 1; i <= g_pGlobals->maxClients; i++ )
	{
		IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>( i );
		if ( !pPlayer )
		{
			continue;
		}

		AddRecipient( pPlayer );
	}
}

void MyRecipientFilter::AddRecipient( const IBasePlayer *player )
{
	Assert( player );

	if ( !player )
		return;

	int index = const_cast<IBasePlayer*>(player)->entindex();

	// If we're predicting and this is not the first time we've predicted this sound
	//  then don't send it to the local player again.
	/*if ( m_bUsingPredictionRules )
	{
		// Only add local player if this is the first time doing prediction
		if ( g_RecipientFilterPredictionSystem.GetSuppressHost() == player )
		{
			return;
		}
	}*/

	// Already in list
	if ( m_Recipients.Find( index ) != m_Recipients.InvalidIndex() )
		return;

	m_Recipients.AddToTail( index );
}

void MyRecipientFilter::RemoveAllRecipients( void )
{
	m_Recipients.RemoveAll();
}

void MyRecipientFilter::RemoveRecipient( IBasePlayer *player )
{
	Assert( player );
	if ( player )
	{
		int index = player->entindex();

		// Remove it if it's in the list
		m_Recipients.FindAndRemove( index );
	}
}

void MyRecipientFilter::RemoveRecipientByPlayerIndex( int playerindex )
{
	Assert( playerindex >= 1 && playerindex <= ABSOLUTE_PLAYER_LIMIT );

	m_Recipients.FindAndRemove( playerindex );
}

void MyRecipientFilter::AddRecipientsByTeam( ITeam *team )
{
	Assert( team );

	int i;
	int c = team->GetNumPlayers();
	for ( i = 0 ; i < c ; i++ )
	{
		IBasePlayer *player = (IBasePlayer*)team->GetPlayer( i );
		if ( !player )
			continue;

		AddRecipient( player );
	}
}

void MyRecipientFilter::RemoveRecipientsByTeam( ITeam *team )
{
	Assert( team );

	int i;
	int c = team->GetNumPlayers();
	for ( i = 0 ; i < c ; i++ )
	{
		IBasePlayer *player = (IBasePlayer*)team->GetPlayer( i );
		if ( !player )
			continue;

		RemoveRecipient( player );
	}
}

void MyRecipientFilter::RemoveRecipientsNotOnTeam( ITeam *team )
{
	Assert( team );

	int i;
	for ( i = 1; i <= g_pGlobals->maxClients; i++ )
	{
		IBasePlayer *player = GetVirtualClass<IBasePlayer>( i );
		if ( !player )
			continue;

		if ( player->GetTeam() != team )
		{
			RemoveRecipient( player );
		}
	}
}

void MyRecipientFilter::AddPlayersFromBitMask( CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits )
{
	int index = playerbits.FindNextSetBit( 0 );

	while ( index > -1 )
	{
		IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>( index + 1 );
		if ( pPlayer )
		{
			AddRecipient( pPlayer );
		}

		index = playerbits.FindNextSetBit( index + 1 );
	}
}

void MyRecipientFilter::RemovePlayersFromBitMask( CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits )
{
	int index = playerbits.FindNextSetBit( 0 );

	while ( index > -1 )
	{
		IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>( index + 1 );
		if ( pPlayer )
		{
			RemoveRecipient( pPlayer );
		}

		index = playerbits.FindNextSetBit( index + 1 );
	}
}

void MyRecipientFilter::AddRecipientsByPVS( const Vector& origin )
{
	if ( g_pGlobals->maxClients == 1 )
	{
		AddAllPlayers();
	}
	else
	{
		CBitVec< ABSOLUTE_PLAYER_LIMIT > playerbits;
		engine->Message_DetermineMulticastRecipients( false, origin, playerbits );
		AddPlayersFromBitMask( playerbits );
	}
}

void MyRecipientFilter::RemoveRecipientsByPVS( const Vector& origin )
{
	if ( g_pGlobals->maxClients == 1 )
	{
		m_Recipients.RemoveAll();
	}
	else
	{
		CBitVec< ABSOLUTE_PLAYER_LIMIT > playerbits;
		engine->Message_DetermineMulticastRecipients( false, origin, playerbits );
		RemovePlayersFromBitMask( playerbits );
	}
}



void MyRecipientFilter::AddRecipientsByPAS( const Vector& origin )
{
	if ( g_pGlobals->maxClients == 1 )
	{
		AddAllPlayers();
	}
	else
	{
		CBitVec< ABSOLUTE_PLAYER_LIMIT > playerbits;
		engine->Message_DetermineMulticastRecipients( true, origin, playerbits );
		AddPlayersFromBitMask( playerbits );
	}
}

bool MyRecipientFilter::IsInitMessage( void ) const
{
	return m_bInitMessage;
}

void MyRecipientFilter::MakeInitMessage( void )
{
	m_bInitMessage = true;
}

/*void MyRecipientFilter::UsePredictionRules( void )
{
	if ( m_bUsingPredictionRules )
		return;

	m_bUsingPredictionRules = true;

	// Cull list now, if needed
	if ( GetRecipientCount() == 0 )
		return;

	IBasePlayer *pPlayer = ToBasePlayer( (CBaseEntity*)g_RecipientFilterPredictionSystem.GetSuppressHost() );

	if ( pPlayer)
	{
		RemoveRecipient( pPlayer );
	}
}*/

bool MyRecipientFilter::IsUsingPredictionRules( void ) const
{
	return m_bUsingPredictionRules;
}

bool MyRecipientFilter::	IgnorePredictionCull( void ) const
{
	return m_bIgnorePredictionCull;
}

void MyRecipientFilter::SetIgnorePredictionCull( bool ignore )
{
	m_bIgnorePredictionCull = ignore;
}

inline IBaseEntity* Instance(int index)
{
	edict_t *pEnt = g_Sample.PEntityOfEntIndex(index);
	if(!pEnt)
		pEnt = g_Sample.PEntityOfEntIndex(0);

	if(pEnt && pEnt->GetUnknown())
	{
		return (IBaseEntity*)pEnt->GetUnknown()->GetBaseEntity();
	}
	return nullptr;
};

inline IBasePlayer *ToBasePlayer(IBaseEntity* pEntity)
{
	if(!pEntity && !pEntity->IsPlayer())
		return nullptr;

	return static_cast<IBasePlayer*>(pEntity);
}

void CPASAttenuationFilter::Filter( const Vector& origin, float attenuation /*= ATTN_NORM*/ )
{
	// Don't crop for attenuation in single player
	if ( g_pGlobals->maxClients == 1 )
		return;

	// CPASFilter adds them by pure PVS in constructor
	if ( attenuation <= 0 )
		return;

	// Now remove recipients that are outside sound radius
	float distance, maxAudible;
	Vector vecRelative;

	int c = GetRecipientCount();
	
	for ( int i = c - 1; i >= 0; i-- )
	{
		int index = GetRecipientIndex( i );

		IBaseEntity *ent = Instance( index );
		if ( !ent || !ent->IsPlayer() )
		{
			Assert( 0 );
			continue;
		}

		IBasePlayer *player = ToBasePlayer( ent );
		if ( !player )
		{
			Assert( 0 );
			continue;
		}

		// never remove the HLTVs
		IPlayerInfo *pPlayerInfo = playerhelpers->GetGamePlayer(index)->GetPlayerInfo();
		if ( pPlayerInfo->IsHLTV())
			continue;

		VectorSubtract( player->EarPosition(), origin, vecRelative );
		distance = VectorLength( vecRelative );
		maxAudible = ( 2 * SOUND_NORMAL_CLIP_DIST ) / attenuation;
		if ( distance <= maxAudible )
			continue;

		RemoveRecipient( player );
	}
}