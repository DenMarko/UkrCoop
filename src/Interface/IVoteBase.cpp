#include "IVoteBase.h"
#include "ITeam.h"
#include "ConVar_l4d.h"
#include "CUserMessage.h"

bool CanTeamCastVote(int iTeam)
{
    void *pVoteController = g_HL2->GetVoteController();
    int m_iOnlyTeamToVote = access_member<int>(pVoteController, 224 * 4);

    return (m_iOnlyTeamToVote == -1) || (iTeam == m_iOnlyTeamToVote);
}

int GetVoterTeam(IBaseEntity *pEntity)
{
    if(!pEntity)
        return TEAM_UNASSIGNED;

    return (pEntity->GetTeamNumber() == 1) ? 2 : pEntity->GetTeamNumber();
}

bool IsFinaleEscapeInProgress(void *pThis)
{
    bool bRes = access_member<bool>(pThis, 358);
    if(bRes)
    {
        return access_member<int>(pThis, 102*4) == 2;
    }
    return bRes;
}

bool IsFinaleWow(void *pThis)
{
    bool bRes = access_member<bool>(pThis, 358);
    if(bRes) {
        return access_member<int>(pThis, 102*4) == 3;
    }
    return bRes;
}

class PotentialVotesCount
{
public:
    PotentialVotesCount() : iCount(0) {}
    virtual bool operator() (int nPlayer)
    {
        IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(nPlayer);
        if(!pPlayer || !pPlayer->IsConnected() || pPlayer->IsBot())
            return true;

        if(CanTeamCastVote(GetVoterTeam(pPlayer)))
        {
            iCount++;
        }
        return true;
    }

    int GetCount()
    {
        return iCount;
    }
private:
    int iCount;
};

NOINLINE void RegisterIssue(IBaseIssue *pszNewIssue)
{
    void *pVoteController = g_HL2->GetVoteController();
    if(pVoteController)
    {
        CUtlVector<IBaseIssue *> &m_potentialIssues = access_member<CUtlVector<IBaseIssue *>>(pVoteController, 287 * 4);
        m_potentialIssues.AddToTail(pszNewIssue);
        return;
    }
    return;
}

void IBaseIssue::ListStandardNoArgCommand(IBasePlayer *pPlayer, const char *issueString)
{
    char szBuffer[MAX_COMMAND_LENGTH];
    V_snprintf(szBuffer, MAX_COMMAND_LENGTH, "callvote %s\n", issueString);
    g_HL2->TextMsg(pPlayer->entindex(), DEST::CONSOLE, szBuffer);
}

IBaseIssue::IBaseIssue(const char *typeString)
{
    _strcpy_safe(m_szTypeString, typeString);

    m_iNumYesVotes = 0;
    m_iNumNoVotes = 0;
    m_iNumPotentialVotes = 0;

    RegisterIssue(this);
}

IBaseIssue::~IBaseIssue()
{
    for(int index = 0; index < m_FailedVotes.Count(); index++)
    {
        FailedVote* pFailedVote = m_FailedVotes[index];
        delete pFailedVote;
    }
}

const char *IBaseIssue::GetTypeString(void)
{
    return m_szTypeString;
}

const char *IBaseIssue::GetDetailsString()
{
    return m_szDetailsString;
}

void IBaseIssue::SetIssueDetails(const char *pszDetails)
{
    _strcpy_safe(m_szDetailsString, pszDetails);
}

void IBaseIssue::OnVotePassed(void)
{
}

void IBaseIssue::OnVoteFailed(void)
{
	static ConVarRef VoteFailureRef("sv_vote_failure_timer");
    for(int index = 0; index < m_FailedVotes.Count(); index++)
    {
        FailedVote *pFailedVote = m_FailedVotes[index];
        if(V_strncmp(pFailedVote->szFailedVoteParameter, GetDetailsString(), Q_ARRAYSIZE(pFailedVote->szFailedVoteParameter) - 1) == 0)
        {
            pFailedVote->flLockoutTime = g_pGlobals->curtime + VoteFailureRef.GetFloat();
            return;
        }
    }
    FailedVote *pNewFailedVote = new FailedVote;
    int iIndex = m_FailedVotes.AddToTail(pNewFailedVote);
    _strcpy_safe(m_FailedVotes[iIndex]->szFailedVoteParameter, GetDetailsString());
    m_FailedVotes[iIndex]->flLockoutTime = g_pGlobals->curtime + VoteFailureRef.GetFloat();
}

bool IBaseIssue::CanTeamCallVote(int iTeam) const
{
    return true;
}

bool IBaseIssue::CanCallVote(int iEntIndex, const char *pszCommand, const char *pszDetails)
{
    if(iEntIndex == -1)
    {
        return false;
    }
    
    IBaseEntity *pVoteCaller = GetVirtualClass<IBaseEntity>(iEntIndex);
    if(pVoteCaller)
    {
        int nVoteTeam = GetVoterTeam(pVoteCaller);
        if(!CanTeamCallVote(nVoteTeam))
        {
            ITeam *pTeam = pVoteCaller->GetTeam();
            if(pTeam)
            {
                V_snprintf((char *)pszDetails, sizeof(pszDetails), "The %s team cannot call this vote.", pTeam->GetName());
            } else {
                V_snprintf((char *)pszDetails, sizeof(pszDetails), "Your team cannot call this vote.");
            }
            return false;
        }
    }

    return true;
}

bool IBaseIssue::IsAllyRestrictedVote(void)
{
    return false;
}

const char *IBaseIssue::GetVotePassedString(void)
{
    return "Unknown vote passed.";
}

int IBaseIssue::CountPotentialVoters(void)
{
    PotentialVotesCount mPotentialVoter;
    g_Sample.ForEachPlayers(mPotentialVoter);

    return mPotentialVoter.GetCount();
}

bool IBaseTerrorIssue::CanTeamCallVote(int iTeam) const
{
    return true;
}

bool IBaseTerrorIssue::CanCallVote(int iEntIndex, const char *pszCommand, const char *pszDetails)
{
    if(IBaseIssue::CanCallVote(iEntIndex, pszCommand, pszDetails))
    {
        int nTeam2_1 = 0;
        int nTeam2_2 = 0;
        g_CallHelper->TransitionPlayerCount(&nTeam2_1, &nTeam2_2, 2);
        
        int nTeam3_1 = 0;
        int nTeam3_2 = 0;
        g_CallHelper->TransitionPlayerCount(&nTeam3_1, &nTeam3_2, 3);

        if((nTeam3_1 + nTeam2_1) <= 0)
        {
            void *pDirector = g_HL2->GetDirector();
            if(IsStartSpawnOnlyVote() && (access_member<bool>(pDirector, 356) || access_member<int>(pDirector, 1648*4)))
            {
                V_strncpy((char *)pszDetails, "You can only vote on that before people leave the starting spawn area.", 0x60);
            }
            else if(!IsFinaleEscapeInProgress(pDirector) && !IsFinaleWow(pDirector))
            {
                return true;
            }
        }
    }

    IBasePlayer *pPlayer = GetVirtualClass<IBasePlayer>(iEntIndex);
    if(pPlayer)
    {
        int pP[] = {iEntIndex};
        CUserRecipientFilter pFilter(pP, 1);
        pFilter.MakeReliable();

        CUserMessage *pMsg = nullptr;
        if((pMsg = new CUserMessage(pFilter, "CallVoteFailed")) != nullptr)
        {
            pMsg->MsgWriteByte(1u);
            delete pMsg;
        }
    }

    return false;
}

bool IBaseTerrorIssue::IsAllyRestrictedVote(void)
{
    return true;
}

int IBaseTerrorIssue::CountPotentialVoters(void)
{
    int iCount = IBaseIssue::CountPotentialVoters();
    if(CanTeamCastVote(2))
    {
        int v2 = 0;
        int v3 = 0;
        g_CallHelper->TransitionPlayerCount(&v2, &v3, 2);
        return v2 + iCount - v3;
    }

    if(CanTeamCastVote(3))
    {
        int v2 = 0;
        int v3 = 0;
        g_CallHelper->TransitionPlayerCount(&v2, &v3, 3);
        return v2 + iCount - v3;
    }

    return iCount;
}

bool IBaseTerrorIssue::IsStartSpawnOnlyVote(void)
{
    return false;
}
