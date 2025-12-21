#include "chat_log.h"

void chat_log::InitChatLog(void)
{
	time_t t = time(NULL);
	tm *curtime = localtime(&t);

    info.m_NrmCurDay = curtime->tm_mday;

	char patch[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_SM, patch, sizeof(patch), "/logs/CHAT_LOG_%04d%02d%02d.log", curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
	info.m_NrmFileName.assign(patch);
	info.m_DailPrinted = true;

	if(info.m_DailPrinted)
	{
		FILE *fp = fopen(info.m_NrmFileName.c_str(), "a+");
		char date[64];
		info.m_DailPrinted = false;
		strftime(date, sizeof(date), "%d.%m.%Y %H:%M:%S", curtime);
		fprintf(fp, "L [%s] ChatLog log file session started(file \"CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday, SMEXT_CONF_VERSION);
		fclose(fp);
	}
}

void chat_log::ChatLogToFile(FILE *fp, const char* msg, va_list ap)
{
	char buffer[3072];
	g_pSM->FormatArgs(buffer, sizeof(buffer), msg, ap);

	char date[64];
    g_Sample.GetCurTime(date, sizeof(date));

	fprintf(fp, "L [%s] %s\n", date, buffer);
}

bool chat_log::ChatLogMsg(const char *msg, ...)
{
	time_t t = time(NULL);
	tm *curtime = localtime(&t);

	if(info.m_NrmCurDay != curtime->tm_mday){
		char patch[PLATFORM_MAX_PATH];
		g_pSM->BuildPath(Path_SM, patch, sizeof(patch), "/logs/CHAT_LOG_%04d%02d%02d.log", curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
		info.m_NrmFileName.assign(patch);
		info.m_NrmCurDay = curtime->tm_mday;
		info.m_DailPrinted = true;
	}

	FILE *fp = fopen(info.m_NrmFileName.c_str(), "a+");
	if(fp)
	{
		if(info.m_DailPrinted)
		{
			char date[64];
			info.m_DailPrinted = false;
			strftime(date, sizeof(date), "%d.%m.%Y %H:%M:%S", curtime);
			fprintf(fp, "L [%s] ChatLog log file session started (file \"CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday, SMEXT_CONF_VERSION);
		}
		va_list ap;
		va_start(ap, msg);
		ChatLogToFile(fp, msg, ap);
		va_end(ap);
		fclose(fp);
		return true;
	}
	fclose(fp);
	return false;
}