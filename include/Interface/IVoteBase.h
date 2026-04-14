#ifndef _INCLUDE_VOTE_BASE_HEADER_
#define _INCLUDE_VOTE_BASE_HEADER_
#include "IBasePlayer.h"

#define MAX_COMMAND_LENGTH 64


class IBaseIssue
{
public:
    IBaseIssue(const char *typeString);
    virtual ~IBaseIssue();

	const char			*GetTypeString( void );
	virtual const char	*GetDetailsString();
	virtual void		SetIssueDetails( const char *pszDetails );
    virtual void        OnVotePassed(void);
	virtual void		OnVoteFailed( void );
	virtual void		OnVoteStarted( void ) {}
	virtual bool		CanTeamCallVote( int iTeam ) const;
	virtual bool		CanCallVote( int iEntIndex, const char *pszCommand, const char *pszDetails);
	virtual bool		IsAllyRestrictedVote( void );
	virtual const char *GetDisplayString( void ) = 0;
	virtual void		ExecuteCommand( void ) = 0;
	virtual void		ListIssueDetails( IBasePlayer *pForWhom ) = 0;
	virtual const char *GetVotePassedString( void );
	virtual int			CountPotentialVoters( void );
protected:
	static void ListStandardNoArgCommand(IBasePlayer *pPlayer, const char *issueString);

	struct FailedVote
	{
		char	szFailedVoteParameter[MAX_COMMAND_LENGTH];
		float	flLockoutTime;					
	};

	CUtlVector<FailedVote *> m_FailedVotes;

	char				m_szTypeString[MAX_COMMAND_LENGTH];
	char				m_szDetailsString[MAX_COMMAND_LENGTH];

	int m_iNumYesVotes;
	int m_iNumNoVotes;
	int m_iNumPotentialVotes;
};

class IBaseTerrorIssue : public IBaseIssue
{
public:
	IBaseTerrorIssue(const char *pszTypeString) : IBaseIssue(pszTypeString)
	{
	}

	virtual bool		CanTeamCallVote( int iTeam ) const;
	virtual bool		CanCallVote( int iEntIndex, const char *pszCommand, const char *pszDetails);
	virtual bool		IsAllyRestrictedVote( void );
	virtual int			CountPotentialVoters( void );
	virtual bool		IsStartSpawnOnlyVote(void);
};

#endif