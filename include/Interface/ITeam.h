#ifndef _HEADER_TEAM_INCLUDE_
#define _HEADER_TEAM_INCLUDE_
#include "IBasePlayer.h"

class CTeamSpawnPoint;

class ITeam : public IBaseEntity
{
public:
    virtual ~ITeam() {}
	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual void			Precache( void ) = 0;

	virtual void			Think( void ) = 0;
	virtual int				UpdateTransmitState() = 0;

	//-----------------------------------------------------------------------------
	// Initialization
	//-----------------------------------------------------------------------------
	virtual void		    Init( const char *pName, int iNumber ) = 0;

	//-----------------------------------------------------------------------------
	// Data Handling
	//-----------------------------------------------------------------------------
	virtual int			    GetTeamNumber( void ) const = 0;
	virtual const char      *GetName( void ) = 0;
	virtual void            UpdateClientData( CBasePlayer *pPlayer ) = 0;
	virtual int             ShouldTransmitToPlayer( CBasePlayer* pRecipient, CBaseEntity* pEntity ) = 0;

	//-----------------------------------------------------------------------------
	// Spawnpoints
	//-----------------------------------------------------------------------------
	virtual void            InitializeSpawnpoints( void ) = 0;
	virtual void            AddSpawnpoint( CTeamSpawnPoint *pSpawnpoint ) = 0;
	virtual void            RemoveSpawnpoint( CTeamSpawnPoint *pSpawnpoint ) = 0;
	virtual CBaseEntity     *SpawnPlayer( CBasePlayer *pPlayer ) = 0;

	//-----------------------------------------------------------------------------
	// Players
	//-----------------------------------------------------------------------------
	virtual void            InitializePlayers( void ) = 0;
	virtual void            AddPlayer( CBasePlayer *pPlayer ) = 0;
	virtual void            RemovePlayer( CBasePlayer *pPlayer ) = 0;
	virtual int             GetNumPlayers( void ) = 0;
	virtual CBasePlayer     *GetPlayer( int iIndex ) = 0;

	//-----------------------------------------------------------------------------
	// Scoring
	//-----------------------------------------------------------------------------
	virtual void            AddScore( int score ) = 0;
	virtual void            SetScore( int score ) = 0;
	virtual int             GetScore( void ) = 0;
	virtual void            ResetScores( void ) = 0;

	// Round scoring
    virtual int             GetRoundsWon( void ) = 0;
    virtual void            SetRoundsWon( int iRounds ) = 0;
    virtual void            IncrementRoundsWon( void ) = 0;
};

class ICSTeam : public ITeam
{
public:
	virtual ServerClass*	GetServerClass(void) = 0;
	virtual int				YouForgotToImplementOrDeclareServerClass(void) = 0;
	virtual void			Precache( void ) = 0;
	virtual void			Think( void ) = 0;
	virtual void		    Init( const char *pName, int iNumber ) = 0;
	virtual void            AddPlayer( CBasePlayer *pPlayer ) = 0;
	virtual void            RemovePlayer( CBasePlayer *pPlayer ) = 0;
};
#endif