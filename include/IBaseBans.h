#ifndef _HEADER_IBASEBANS_INCLUDE_
#define _HEADER_IBASEBANS_INCLUDE_

struct ReasonList
{
    ReasonList();

    ~ReasonList()
    {
        if(m_next)  delete m_next;
    }

    char display[128];
    char info[128];

    ReasonList* m_next;
};

extern ReasonList *g_ListReason;

struct SBanInfo
{
    SBanInfo()
    {
        iBanCreate = -1;
        iBanEnds = -1;
        ilength = -1;
    }

    const char *szName;
    const char *authId;
    int iBanCreate;
    int iBanEnds;
    int ilength;
    const char *szAdminName;
    const char *szReason;
};

class IBaseBans
{
public:
	virtual ~IBaseBans() {}

	virtual bool AddBan(int clientID, int adminID, int time, const char *reason) = 0;
	virtual bool AddBan(const char *authId, int adminID, int time, const char *reason) = 0;
	virtual bool UnBan(const char* authId, int adminId, const char* reason) = 0;

    virtual bool GetClientBanData(const char *authID, SBanInfo *info) = 0;
    virtual IQuery* GetActiveBans( void ) = 0;
    virtual IResultSet* GetResultSet(IQuery* pQuery) = 0;
    virtual bool GetBanInfo(IResultSet* query, SBanInfo* info) = 0;
};

class IBaseBansDB
{
public:
    virtual ~IBaseBansDB() {}

	virtual SourceMod::IDBDriver *GetDriver( void ) const = 0;
    virtual SourceMod::IDatabase *GetDataBase( void ) const = 0;
	virtual void Release( void ) = 0;
};

extern IBaseBansDB* g_pBanDB;
extern IBaseBans *g_pBaseBans;

#endif // _HEADER_IBASEBANS_INCLUDE_