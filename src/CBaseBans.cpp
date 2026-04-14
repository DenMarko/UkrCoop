#include "CBaseBans.h"
#include "sourcehook/sh_string.h"
#include "log_messege.h"
#include "Interface/IProps.h"

IBaseBans *g_pBaseBans = nullptr;
IBaseBansDB* g_pBanDB = nullptr;

ReasonList *g_ListReason = nullptr;

ReasonList::ReasonList() : m_next(nullptr)
{ 
	display[0] = '\0'; 
	info[0] = '\0';
}

void ServerCommand(const char *command, ...)
{
	va_list args;
	char buffer[128];
	va_start(args, command);
	size_t len = vsnprintf(buffer, sizeof(buffer), command, args);
	va_end(args);

	if (len >= sizeof(buffer))
	{
		buffer[sizeof(buffer) - 1] = '\0';
	}

	engine->ServerCommand(buffer);
	engine->ServerExecute();
}

class BanReasonLoadFromFile : public ITextListener_SMC
{
public:
    BanReasonLoadFromFile() : m_list(nullptr)
    {
		SMCStates states;
		SMCError error;

		g_ListReason = new ReasonList();
		g_pSM->BuildPath(Path_SM, m_file, sizeof(m_file), "configs/banreasons.txt");

		if ((error = textparsers->ParseFile_SMC(m_file, this, &states)) != SMCError_Okay)
		{
			const char* err_string = textparsers->GetSMCErrorString(error);
			if (!err_string)
			{
				err_string = "Unknown error";
			}
			ParseError(NULL, "Error %d (%s)", error, err_string);
		}
    }
    ~BanReasonLoadFromFile() { }

private:
    SMCResult ReadSMC_KeyValue(const SMCStates *states, const char *key, const char *value) override
	{
        if(!m_list)
        {
			m_list = g_ListReason;
            g_Sample.UTIL_Format(m_list->info, sizeof(m_list->info), "%s", key);
            g_Sample.UTIL_Format(m_list->display, sizeof(m_list->display), "%s", value);

            return SMCResult_Continue;
        }

        m_list->m_next = new ReasonList();
        m_list = m_list->m_next;

        g_Sample.UTIL_Format(m_list->info, sizeof(m_list->info), "%s", key);
        g_Sample.UTIL_Format(m_list->display, sizeof(m_list->display), "%s", value);

        return SMCResult_Continue;
    }

	void ParseError(const SMCStates *states, const char *message, ...)
	{
		va_list ap;
		char buffer[256];

		va_start(ap, message);
		g_pSM->FormatArgs(buffer, sizeof(buffer), message, ap);
		va_end(ap);

		m_sLog->LogToFileEx(false, "[BanReasonLoadFromFile] (Line %d): %s", states ? states->line : 0, buffer);
	}
private:
    char m_file[MAX_PATH];
    ReasonList *m_list;
};


class CBanLoader : public CAppSystem
{
public:
	CBanLoader() : CAppSystem(), m_pBans(nullptr) { }
	virtual ~CBanLoader() { OnUnload(); }

	void OnLoad() override
	{
		m_pBans = new CBaseBans();
		BanReasonLoadFromFile parse;
	}

	void OnAllLoaded() override
	{
		playerhelpers->AddClientListener(m_pBans);
	}

	void OnUnload() override
	{
		RELEASE(g_ListReason)
		RELEASE(m_pBans)
	}

	ResultType OnClientCommand(int client, const CCommand &args) override
	{
        IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
        if(!pPlayer ||
           !pPlayer->IsConnected() || 
           !pPlayer->IsInGame())
        {
            return Pl_Continue;
        }

		if(g_Sample.my_bStrcmp(args[0], "ukr_addban"))
		{
            if(IsAdminAccess(pPlayer->GetAdminId(), ADMFLAG_BAN))
            {
                if(args.ArgC() >= 4)
                {
                    g_pBaseBans->AddBan(atoi(args[1]), client, atoi(args[2]), args[3]);
                }
                else
                    g_HL2->TextMsg(client, CONSOLE, "[SM] To use the command type \"ukr_addban\" or \"ukr_addban <player_id> <time> <reason>\"");
            }
			return Pl_Handled;
		}
		else if(g_Sample.my_bStrcmp(args[0], "ukr_addbanid"))
		{
			if(IsAdminAccess(pPlayer->GetAdminId(), ADMFLAG_BAN))
			{
                if(args.ArgC() >= 4)
                {
                    g_pBaseBans->AddBan(args[1], client, atoi(args[2]), args[3]);
                }
                else
                    g_HL2->TextMsg(client, CONSOLE, "[SM] To use the command type \"ukr_addbanid\" or \"ukr_addbanid <authid> <time> <reason>\"");
			}
			return Pl_Handled;
		}
		else if(g_Sample.my_bStrcmp(args[0], "ukr_unban"))
		{
			if(IsAdminAccess(pPlayer->GetAdminId(), ADMFLAG_BAN))
			{
                if(args.ArgC() >= 4)
                {
                    g_pBaseBans->UnBan(args[1], client, args[2]);
                }
                else
                    g_HL2->TextMsg(client, CONSOLE, "[SM] To use the command type \"ukr_unban\" or \"ukr_unban <authid> <reason>\"");
			}
			return Pl_Handled;
		}
		else if(g_Sample.my_bStrcmp(args[0], "ukr_active_bans"))
		{
			if(IsAdminAccess(pPlayer->GetAdminId(), ADMFLAG_BAN))
			{
				auto pQuery = g_pBaseBans->GetActiveBans();
				if(!pQuery)
				{
					return Pl_Handled;
				}

				auto pQueryRes = g_pBaseBans->GetResultSet(pQuery);
				if(!pQueryRes)
				{
					pQuery->Destroy();
					g_HL2->TextMsg(client, CONSOLE, "No Active bans!");
					return Pl_Handled;
				}

				bool bPrint = true;
				while (bPrint)
				{
					SBanInfo *pInfo = new SBanInfo();
					if(g_pBaseBans->GetBanInfo(pQueryRes, pInfo))
					{
						char buffer[512];
						g_Sample.UTIL_Format(buffer, sizeof(buffer), "Name: %s\nAuthId: %s\nCreate: %d\nEnd: %d\nLenght: %d\nAdmin Name: %s\nReason: %s", 
							pInfo->szName, 
							pInfo->authId, 
							pInfo->iBanCreate, 
							pInfo->iBanEnds, 
							pInfo->ilength, 
							pInfo->szAdminName, 
							pInfo->szReason);

						g_HL2->TextMsg(client, CONSOLE, buffer);
					} else { bPrint = false; }
					delete pInfo;
				}
				pQuery->Destroy();
			}
			return Pl_Handled;
		}

		return Pl_Continue;
	}
private:
	CBaseBans *m_pBans;
};
CBanLoader *g_pBanLoader = new CBanLoader();

enum ParseState {
	state_none = 0,
	state_admins
};

struct AdminList
{
	AdminList() {
		name[0] = '\0';
		auth[0] = '\0';
		identity[0] = '\0';
		password[0] = '\0';
		group[0] = '\0';
		flags[0] = '\0';

		immunity = 0;
		m_next = nullptr;
	}

	~AdminList()
	{
		if(m_next)
			delete m_next;
	}

	char name[64];
	char auth[64];
	char identity[64];
	char password[64];
	char group[64];
	char flags[64];
	unsigned int immunity;

	AdminList* m_next;
};

class AdminCash : public ITextListener_SMC
{
public:
	AdminCash() : m_list(nullptr), m_cash(nullptr)
	{
		SMCStates states;
		SMCError error;

		m_ParseState = state_none;
		g_pSM->BuildPath(Path_SM, m_file, sizeof(m_file), "configs/admins.cfg");

		if ((error = textparsers->ParseFile_SMC(m_file, this, &states)) != SMCError_Okay)
		{
			const char* err_string = textparsers->GetSMCErrorString(error);
			if (!err_string)
			{
				err_string = "Unknown error";
			}
			ParseError(NULL, "Error %d (%s)", error, err_string);
		}
	}
	~AdminCash()
	{
		if(m_list)
		{
			delete m_list;
		}
	}

	AdminList *GetAdminList()
	{
		return m_list;
	}

private:
	void ReadSMC_ParseStart() override
	{
		m_list = new AdminList();
		g_Sample.UTIL_Format(m_list->name, sizeof(m_list->name), "SERVER");
		g_Sample.UTIL_Format(m_list->identity, sizeof(m_list->identity), "STEAM_ID_SERVER");
	}

	SMCResult ReadSMC_NewSection(const SMCStates *states, const char *name) override
	{
		switch (m_ParseState)
		{
		case state_none:
			{
				if (g_Sample.my_bStrcmp(name, "Admins"))
				{
					m_ParseState = state_admins;
				}
				break;
			}
		case state_admins:
			{
				if(m_cash == nullptr)
				{
					m_cash = m_list;
				}

				m_cash->m_next = new AdminList();
				m_cash = m_cash->m_next;
				g_Sample.UTIL_Format(m_cash->name, sizeof(m_cash->name), "%s", name);
				break;
			}
		}

		return SMCResult_Continue;
	}

	SMCResult ReadSMC_KeyValue(const SMCStates *states, const char *key, const char *value) override
	{
		if (g_Sample.my_bStrcmp(key, "auth")) {
			g_Sample.UTIL_Format(m_cash->auth, sizeof(m_cash->auth), "%s", value);
		} else if (g_Sample.my_bStrcmp(key, "identity")) {
			g_Sample.UTIL_Format(m_cash->identity, sizeof(m_cash->identity), "%s", value);
		} else if (g_Sample.my_bStrcmp(key, "password")) {
			g_Sample.UTIL_Format(m_cash->password, sizeof(m_cash->password), "%s", value);
		} else if (g_Sample.my_bStrcmp(key, "group")) {
			g_Sample.UTIL_Format(m_cash->group, sizeof(m_cash->group), "%s", value);
		} else if (g_Sample.my_bStrcmp(key, "flags")) {
			g_Sample.UTIL_Format(m_cash->flags, sizeof(m_cash->flags), "%s", value);
		} else if (g_Sample.my_bStrcmp(key, "immunity")) {
			m_cash->immunity = atoi(value);
		}

		return SMCResult_Continue;
	}

	void ParseError(const SMCStates *states, const char *message, ...)
	{
		va_list ap;
		char buffer[256];

		va_start(ap, message);
		g_pSM->FormatArgs(buffer, sizeof(buffer), message, ap);
		va_end(ap);

		m_sLog->LogToFileEx(false, "[AdminCash] (Line %d): %s", states ? states->line : 0, buffer);
	}

private:
	char m_file[MAX_PATH];
	ParseState m_ParseState;
	AdminList* m_list;
	AdminList* m_cash;
};

CBaseBans::CBaseBans() : m_db(nullptr), driver(nullptr)
{
    g_pBanDB = this;
	g_pBaseBans = this;

	char error[256];
	if (!dbi->Connect("basebans", &driver, &m_db, false, error, sizeof(error)))
	{
		m_sLog->LogToFileEx(false, "[CBaseBans::CBaseBans] Connect to database is failed (%s)", error);
		return;
	}

	dbi->AddDependency(myself, driver);
	m_hndl = dbi->CreateHandle(DBHandle_Database, m_db, myself->GetIdentity());
	if(!m_hndl)
	{
		m_db->Release();
		m_sLog->LogToFileEx(false, "[CBaseBans::CBaseBans] Create Handle is failed");
		return;
	}

	this->InitDB();
	this->CheckAdminList();
}

CBaseBans::~CBaseBans()
{
	g_pBaseBans = nullptr;
	g_pBanDB = nullptr;

	if(m_hndl)
	{
		HandleSecurity sec;
		sec.pIdentity = nullptr;
		sec.pOwner = myself->GetIdentity();
		handlesys->FreeHandle(m_hndl, &sec);
	}
}

class TQueryCheckConnect : public IDBThreadOperation
{
public:
	TQueryCheckConnect(Handle_t hndl, SourceHook::String &query, int iClient) : m_db(nullptr), m_client(iClient)
	{
		m_Query.append(query);
		if(dbi->ReadHandle(hndl, DBHandle_Database, (void**)&m_db) == HandleError_None)
		{
			m_db->IncReferenceCount();
		}
	}

	~TQueryCheckConnect()
	{
		if(m_pQuery)
			m_pQuery->Destroy();
	}

	IDBDriver *GetDriver()
	{
		if(m_db)
			return m_db->GetDriver();

		return (IDBDriver *)g_pBanDB->GetDriver();
	}
	IdentityToken_t *GetOwner() { return myself->GetIdentity(); }

	void RunThreadPart()
	{
		if(!m_db) return;

		m_db->LockForFullAtomicOperation();
		m_pQuery = m_db->DoQuery(m_Query.c_str());
		if(!m_pQuery)
		{
			g_Sample.UTIL_Format(error, sizeof(error), "%s", m_db->GetError());
			Msg("Error: %s\n", error);
		}
		m_db->UnlockFromFullAtomicOperation();
	}

	void RunThinkPart()
	{
		if(!m_db) return;

		if(m_pQuery)
		{
			IResultSet *pRes = m_pQuery->GetResultSet();
			if(pRes)
			{
				IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(m_client);
				if(pRes->GetRowCount() > 0)
				{
					if(pPlayer)
					{
						gamehelpers->AddDelayedKick(m_client, pPlayer->GetUserId(), "You banned this server\n");
					}
				}

				if(pPlayer->IsAuthorized())
				{
					char szName[64];
					size_t written;
					SourceHook::String update;

					m_db->QuoteString(pPlayer->GetName(), szName, sizeof(szName), &written);

					update += "UPDATE bb_bans SET user = '";
					update += (szName[0] == '\0' ? "Unknown" : szName);
					update += "' WHERE authid = '";
					update += pPlayer->GetSteam2Id();
					update += "' AND name = '';";

					m_db->DoSimpleQuery(update.c_str());
				}
			}
		}
	}

	void CancelThinkPart() { }
	void Destroy() { delete this; }
private:
	IDatabase* m_db;
	SourceHook::String m_Query;
	IQuery* m_pQuery;
	char error[255];
	int m_client;
};

void CBaseBans::OnClientAuthorized(int client, const char *authstring)
{
	if(!m_db) {
		return;
	}

	if (authstring[0] == 'B' || authstring[9] == 'L') {
		return;
	}

	IGamePlayer* pPlayer = nullptr;
	if ((pPlayer = playerhelpers->GetGamePlayer(client)) == nullptr) {
		return;
	}

	char ip[64], *ptr;
	strcpy(ip, pPlayer->GetIPAddress());
	if((ptr = strchr(ip, ':'))) {
		*ptr = '\0';
	}

	SourceHook::String query;
	query += "SELECT bid FROM bb_bans WHERE ((type = 0 AND authid = '";
	query += authstring;
	query += "') OR (type = 1 AND ip = '";
	query += ip;
	query += "')) AND (length = '0' OR ends > strftime('%s', 'now')) AND RemoveType IS NULL";

	TQueryCheckConnect *pQuery = new TQueryCheckConnect(m_hndl, query, client);
	dbi->AddToThreadQueue(pQuery, PrioQueue_High);
}

bool CBaseBans::InitDB()
{
	SourceHook::String query;
	query += "CREATE TABLE IF NOT EXISTS bb_bans (";
	query += "bid INTEGER PRIMARY KEY AUTOINCREMENT,";
	query += "ip TEXT,";
	query += "authid TEXT NOT NULL DEFAULT '',";
	query += "name TEXT NOT NULL DEFAULT 'unnamed',";
	query += "created INTEGER NOT NULL DEFAULT 0,";
	query += "ends INTEGER NOT NULL DEFAULT 0,";
	query += "length INTEGER NOT NULL DEFAULT 0,";
	query += "reason TEXT NOT NULL,";
	query += "aid INTEGER NOT NULL DEFAULT '0',";
	query += "RemovedBy INTEGER NOT NULL DEFAULT '0',";
	query += "RemoveType TEXT,";
	query += "RemovedOn INTEGER,";
	query += "type INTEGER NOT NULL DEFAULT 0,";
	query += "ureason TEXT);";

	if (!m_db->DoSimpleQuery(query.c_str()))
	{
		m_sLog->LogToFileEx(false, "Create tablet bb_bans is failed");
		return false;
	}

	query.clear();

	query += "CREATE TABLE IF NOT EXISTS bb_admins (";
	query += "aid INTEGER PRIMARY KEY AUTOINCREMENT,";
	query += "user TEXT NOT NULL,";
	query += "authid TEXT NOT NULL DEFAULT '',";
	query += "immunity INTEGER NOT NULL DEFAULT '0',";
	query += "flags TEXT DEFAULT NULL)";

	if (!m_db->DoSimpleQuery(query.c_str()))
	{
		m_sLog->LogToFileEx(false, "Create tablet bb_admins is failed");
		return false;
	}

	return true;
}

bool CBaseBans::CheckAdminList()
{
    if(!m_db) {
        return false;
	}

	AdminCash *parse = new AdminCash();
	auto begin = parse->GetAdminList();
	if (!begin)
	{
		delete parse;
		return false;
	}

	while (begin)
	{
		SourceHook::String query;
		query += "SELECT aid FROM bb_admins WHERE authid = '";
		query += begin->identity;
		query += "'";

		IQuery* pRes = nullptr;
		if((pRes = m_db->DoQuery(query.c_str())) != nullptr)
		{
			IResultSet* pResSet = nullptr;
			if ((pResSet = pRes->GetResultSet()) != nullptr)
			{
				if (!pResSet->GetRowCount())
				{
					SourceHook::String insert;
					insert += "INSERT INTO bb_admins (user, authid, immunity, flags) VALUES('";
					insert += begin->name;
					insert += "', '";
					insert += begin->identity;
					insert += "', ";
					insert += begin->immunity;
					insert += ", '";
					insert += begin->flags;
					insert += "');";

					m_db->DoSimpleQuery(insert.c_str());
				}
			}
			pRes->Destroy();
		}

		begin = begin->m_next;
	}

	delete parse;
	return true;
}

bool CBaseBans::AddBan(int clientID, int adminID, int time, const char* reason)
{
	m_sLog->LogToFileEx(false, "[CBaseBans::AddBan] param: <%d><%d><%d><%s>", clientID, adminID, time, reason);
	if(m_db == nullptr)
	{
		return false;
	}

	SourceHook::String query;
	auto pPlayer = playerhelpers->GetGamePlayer(clientID);
	if (!pPlayer) {
		m_sLog->LogToFileEx(false, "[CBaseBans::AddBan] pPlayer is null");
		return false;
	}

	const char *szAuthId = pPlayer->GetSteam2Id();
	if (szAuthId[0] == 'B' || szAuthId[9] == 'L') {
		return false;
	}

	auto pAdmin = playerhelpers->GetGamePlayer(adminID);

	query += "INSERT INTO bb_bans (ip, authid, name, created, ends, length, reason, aid) VALUES";

	char ip[64], *ptr;
	strcpy(ip, pPlayer->GetIPAddress());
	if((ptr = strchr(ip, ':')))
		*ptr = '\0';
	
	char szName[64], szReason[256];
	size_t written;
	m_db->QuoteString(pPlayer->GetName(), szName, sizeof(szName), &written);
	m_db->QuoteString(reason, szReason, sizeof(szReason), &written);

	query += "('";
	query += ip;
	query += "', '";
	query += szAuthId;
	query += "', '";
	query += (szName[0] == '\0' ? "Unknown" : szName);
	query += "', strftime('%s', 'now'), strftime('%s', 'now') + ";
	query += (60 * time);
	query += ", ";
	query += (60 * time);
	query += ", '";
	query += (szReason[0] == '\0' ? "Unknown" : szReason);
	query += "', (SELECT aid FROM bb_admins WHERE authid = '";
	query += (pAdmin ? pAdmin->GetSteam2Id() : "STEAM_ID_SERVER");
	query += "'));";

	if(!m_db->DoSimpleQuery(query.c_str()))
	{
		m_sLog->LogToFileEx(false, "[CBaseBans::AddBan] Last Error: %s", m_db->GetError());
		return false;
    }

	if (pPlayer->IsConnected() || !pPlayer->IsFakeClient())
	{
		ServerCommand("banid 5 %s\n", szAuthId);
		gamehelpers->AddDelayedKick(clientID, pPlayer->GetUserId(), reason);
	}

	return true;
}

bool CBaseBans::AddBan(const char* authId, int adminID, int time, const char* reason)
{
	m_sLog->LogToFileEx(false, "[CBaseBans::AddBan] param: <%s><%d><%d><%s>", authId, adminID, time, reason);

	if(m_db == nullptr)
	{
		return false;
	}

	if (authId[0] == 'B' || authId[9] == 'L') {
		return false;
	}

	SourceHook::String query;
	query += "SELECT bid FROM bb_bans WHERE type = 0 AND authid = '";
	query += authId;
	query += "' AND (length = 0 OR ends > strftime('%s', 'now')) AND RemoveType IS NULL";

	IQuery* pRes = nullptr;
	if ((pRes = m_db->DoQuery(query.c_str())) != nullptr)
	{
		IResultSet* pResSet = nullptr;
		if ((pResSet = pRes->GetResultSet()) != nullptr)
		{
			if (pResSet->GetRowCount())
			{
				m_sLog->LogToFileEx(false, "[CBaseBans::AddBan] Ban is active");
				pRes->Destroy();
				return true;
			}
		}
		pRes->Destroy();
	}

	auto pAdmin = playerhelpers->GetGamePlayer(adminID);
	query.clear();

	query += "INSERT INTO bb_bans (authid, name, created, ends, length, reason, aid) VALUES";
	query += "('";
	query += authId;
	query += "', '', strftime('%s', 'now'), strftime('%s', 'now') + ";
	query += (60 * time);
	query += ", ";
	query += (60 * time);
	query += ", '";

	char szReason[256];
	size_t written;
	m_db->QuoteString(reason, szReason, sizeof(szReason), &written);
	query += (szReason[0] == '\0' ? "Unknown" : szReason);

	query += "', (SELECT aid FROM bb_admins WHERE authid = '";
	query += (pAdmin ? pAdmin->GetSteam2Id() : "STEAM_ID_SERVER");
	query += "'));";

	if(!m_db->DoSimpleQuery(query.c_str()))
    {
		m_sLog->LogToFileEx(false, "[CBaseBans::AddBan] Last error: %s", m_db->GetError());
		return false;
    }

	return true;
}

bool CBaseBans::UnBan(const char* authId, int adminId, const char* reason)
{
	m_sLog->LogToFileEx(false, "[CBaseBans::UnBan] param: <%s><%d><%s>", authId, adminId, reason);
	if(m_db == nullptr)
	{
		return false;
	}
	
	if (authId[0] == 'B' || authId[9] == 'L') {
		return false;
	}

	SourceHook::String buffer;
	buffer += "SELECT bid FROM bb_bans WHERE (type = 0 AND authid = '";
	buffer += authId;
	buffer += "') AND (length = '0' OR ends > strftime('%s', 'now')) AND RemoveType IS NULL";

	auto pAdmin = playerhelpers->GetGamePlayer(adminId);

	IQuery *pRes = nullptr;
	if ((pRes = m_db->DoQuery(buffer.c_str())) == nullptr)
	{
		return false;
	}

	IResultSet* pResSet = nullptr;
	if ((pResSet = pRes->GetResultSet()) == nullptr)
	{
		pRes->Destroy();
		return false;
	}

	if (!pResSet->GetRowCount())
	{
		pRes->Destroy();
		return true;
	}

	IResultRow *pRow = nullptr;
	if ((pRow = pResSet->CurrentRow()) == nullptr)
	{
		pRes->Destroy();
		return false;
	}

	int bid;
	pRow->GetInt(0, &bid);
	pRes->Destroy();
	SourceHook::String buf;

	buf += "UPDATE bb_bans SET RemovedBy = (SELECT aid FROM bb_admins WHERE authid = '";
	buf += (pAdmin ? pAdmin->GetSteam2Id() : "STEAM_ID_SERVER");
	buf += "'), RemoveType = 'U', RemovedOn = strftime('%s', 'now'), ureason = '";

	char szReason[256];
	size_t written;
	m_db->QuoteString(reason, szReason, sizeof(szReason), &written);
	buf += (szReason[0] == '\0' ? "Unknown" : szReason);

	buf += "' WHERE bid = ";
	buf += bid;
	
	if (!m_db->DoSimpleQuery(buf.c_str())) {
		return false;
	}

	return true;
}

bool CBaseBans::GetClientBanData(const char *authID, SBanInfo *info)
{
	if(info == nullptr)
	{
    	return false;
	}

	SourceHook::String query;
	query += query = "SELECT * FROM bb_bans WHERE authid = '";
	query += authID;
	query += "';";
	
	IQuery* pRes = nullptr;
	if((pRes = m_db->DoQuery(query.c_str())) == nullptr)
	{
		return false;
	}

	IResultSet* pSet = nullptr;
	if((pSet = pRes->GetResultSet()) == nullptr)
	{
		pRes->Destroy();
		return false;
	}

	if(pSet->GetRowCount() == 0)
	{
		pRes->Destroy();
		return false;
	}

	IResultRow *pRow = pSet->CurrentRow();
	if(pRow)
	{
		size_t writen;
		pRow->GetString(2, &info->authId, &writen);
		pRow->GetString(3, &info->szName, &writen);
		pRow->GetInt(4, &info->iBanCreate);
		pRow->GetInt(5, &info->iBanEnds);
		pRow->GetInt(6, &info->ilength);
		pRow->GetString(7, &info->szReason, &writen);

		int aid = -1;
		pRow->GetInt(8, &aid);
		pRes->Destroy();

		query.clear();
		query += "SELECT user FROM bb_admins WHERE aid = ";
		query += aid;
		query += ';';

		IQuery *pQuery = m_db->DoQuery(query.c_str());
		if(pQuery)
		{
			IResultSet *pResSet = pQuery->GetResultSet();
			if(pResSet)
			{
				IResultRow *pResRow = pResSet->FetchRow();
				if(pResRow)
				{
					pResRow->GetString(0, &info->szAdminName, &writen);
				}
			}
			pQuery->Destroy();
		}
		return true;
	}

	pRes->Destroy();
	return false;
}

IQuery* CBaseBans::GetActiveBans()
{
	SourceHook::String query;
	query += "SELECT * FROM bb_bans WHERE (length = 0 OR ends > strftime('%s','now')) AND RemoveType IS NULL;";

	IQuery* pQuery = m_db->DoQuery(query.c_str());
	if(pQuery)
	{
		return pQuery;
	}

    return nullptr;
}

IResultSet *CBaseBans::GetResultSet(IQuery *pQuery)
{
	if(!pQuery)
		return nullptr;

	IResultSet* pSet = nullptr;
	if((pSet = pQuery->GetResultSet()) == nullptr)
	{
		pQuery->Destroy();
		return nullptr;
	}

	if(!pSet->GetRowCount())
	{
		pQuery->Destroy();
		return nullptr;
	}

    return pSet;
}

bool CBaseBans::GetBanInfo(IResultSet *pQueryRes, SBanInfo *info)
{
	if(!pQueryRes || !info)
	{
		return false;
	}

	IResultRow *pRow = pQueryRes->FetchRow();
	if(pRow)
	{
		size_t writen;
		pRow->GetString(2, &info->authId, &writen);
		pRow->GetString(3, &info->szName, &writen);
		pRow->GetInt(4, &info->iBanCreate);
		pRow->GetInt(5, &info->iBanEnds);
		pRow->GetInt(6, &info->ilength);
		pRow->GetString(7, &info->szReason, &writen);

		int aid = -1;
		pRow->GetInt(8, &aid);

		SourceHook::String query;
		query += "SELECT user FROM bb_admins WHERE aid = ";
		query += aid;
		query += ";";

		IQuery *pQuery = m_db->DoQuery(query.c_str());
		if(pQuery)
		{
			IResultSet *pResSet = pQuery->GetResultSet();
			if(pResSet)
			{
				IResultRow *pResRow = pResSet->FetchRow();
				if(pResRow)
				{
					pResRow->GetString(0, &info->szAdminName, &writen);
				}
			}
			pQuery->Destroy();
		}
		return true;
	}
	
    return false;
}
