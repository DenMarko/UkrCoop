#ifndef _HEADER_CBANSMANAGER_INCLUDE_
#define _HEADER_CBANSMANAGER_INCLUDE_
#include "extension.h"
#include "IBaseBans.h"

class CBaseBans : public IClientListener, public IBaseBans, public IBaseBansDB
{
public:
	CBaseBans();
	~CBaseBans();

public: // IClientListener
	void OnClientAuthorized(int client, const char *authstring);

public: // IBaseBans
	bool AddBan(int clientID, int adminID, int time, const char *reason) override;
	bool AddBan(const char *authId, int adminID, int time, const char *reason) override;
	bool UnBan(const char* authId, int adminId, const char* reason) override;

	bool GetClientBanData(const char *authID, SBanInfo *info) override;
	IQuery* GetActiveBans( void ) override;
	IResultSet* GetResultSet(IQuery* pQuery) override;
	bool GetBanInfo(IResultSet* query, SBanInfo* info) override;

public: // IBaseBansDB
	SourceMod::IDBDriver *GetDriver( void ) const override { return driver; }
	SourceMod::IDatabase *GetDataBase() const override { return m_db; }
	void Release( void ) override { m_db = nullptr; }
	
private:
	bool InitDB();
	bool CheckAdminList();

private:
	SourceMod::IDatabase *m_db;
	SourceMod::IDBDriver *driver;

	Handle_t m_hndl;
};

#endif