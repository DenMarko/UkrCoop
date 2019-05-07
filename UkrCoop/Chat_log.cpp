#include "chat_log.h"

void chat_log::InitChatLog(void)
{
	char date[32];
	tm curtime = g_pUkrCoop.GetCurDate(date, sizeof(date));
	g_pLogChatValues.m_NrmCurDay = curtime.tm_mday;

	char patch[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_SM, patch, sizeof(patch), "logs/CHAT_LOG_%04d%02d%02d.log", curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);
	g_pLogChatValues.m_NrmFileName.assign(patch);
	g_pLogChatValues.m_DailPrinted = true;

	if(g_pLogChatValues.m_DailPrinted)
	{
		FILE *fp = NULL;
		fopen_s(&fp, g_pLogChatValues.m_NrmFileName.c_str(), "a+");
		g_pLogChatValues.m_DailPrinted = false;
		fprintf_s(fp, "L [%s] ChatLog log file session started(file \"CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);
		fclose(fp);
	}
}

void chat_log::ChatLogToFile(FILE *fp, const char* msg, va_list ap)
{
	char buffer[3072];
	g_pSM->FormatArgs(buffer, sizeof(buffer), msg, ap);

	char date[15];
	g_pUkrCoop.GetCurDate(date, sizeof(date));

	fprintf_s(fp, "L [%s] %s\n", date, buffer);
}

void chat_log::ChatLogMsg(const char *msg, ...)
{
	char date[32];
	FILE *fp;
	tm curtime = g_pUkrCoop.GetCurDate(date, sizeof(date));
	
	if(g_pLogChatValues.m_NrmCurDay != curtime.tm_mday)
	{
		char patch[PLATFORM_MAX_PATH];
		g_pSM->BuildPath(Path_SM, patch, sizeof(patch), "/logs/CHAT_LOG_%04d%02d%02d.log", curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);
		g_pLogChatValues.m_NrmFileName.assign(patch);
		g_pLogChatValues.m_NrmCurDay = curtime.tm_mday;
		g_pLogChatValues.m_DailPrinted = true;
	}

	fopen_s(&fp, g_pLogChatValues.m_NrmFileName.c_str(), "a+");
	if(fp)
	{
		if(g_pLogChatValues.m_DailPrinted)
		{
			g_pLogChatValues.m_DailPrinted = false;
			fprintf_s(fp, "L [%s] LogMessege log file session started (file \"CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);
		}
		va_list ap;
		va_start(ap, msg);
		ChatLogToFile(fp, msg, ap);
		va_end(ap);
		fclose(fp);
	}
}
