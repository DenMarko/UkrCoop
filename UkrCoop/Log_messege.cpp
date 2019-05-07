#include "log_messege.h"

void LM::InitLogMesseg(void){
	char date[32];
	tm curtime = g_pUkrCoop.GetCurDate(date, sizeof(date));
	g_pLogValues.m_NrmCurDay = curtime.tm_mday;

	char Path[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_SM, Path, sizeof(Path), "logs/UKRCOOP_%04d%02d%02d.log", curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);
	g_pLogValues.m_NrmFileName.assign(Path);
	g_pLogValues.m_DailPrinted = true;

	if(g_pLogValues.m_DailPrinted)
	{
		FILE *fp;
		fopen_s(&fp, g_pLogValues.m_NrmFileName.c_str(), "a+");
		g_pLogValues.m_DailPrinted = false;
		fprintf_s(fp, "L %s: LogMessege log file session started (file \"UKRCOOP_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);
		fclose(fp);
	}
}

void LM::LogToFiles(FILE *fp, const char *mes, va_list ap){
	char buffer[3072];
	g_pSM->FormatArgs(buffer, sizeof(buffer), mes, ap);

	char date[15];
	g_pUkrCoop.GetCurDate(date, sizeof(date));

	fprintf_s(fp, "L %s: %s\n", date, buffer);
	g_SMAPI->ConPrintf("L %s: %s\n", date, buffer);
}

void LM::LogToFileEx(const char *mes, ...)
{
	char date[32];
	FILE *fp;
	tm curtime = g_pUkrCoop.GetCurDate(date, sizeof(date));

	if(g_pLogValues.m_NrmCurDay != curtime.tm_mday){
		char patch[PLATFORM_MAX_PATH];
		g_pSM->BuildPath(Path_SM, patch, sizeof(patch), "/logs/UKRCOOP_%04d%02d%02d.log", curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);
		g_pLogValues.m_NrmFileName.assign(patch);
		g_pLogValues.m_NrmCurDay = curtime.tm_mday;
		g_pLogValues.m_DailPrinted = true;
	}

	fopen_s(&fp, g_pLogValues.m_NrmFileName.c_str(), "a+");
	if(fp){
		if(g_pLogValues.m_DailPrinted){
			g_pLogValues.m_DailPrinted = false;
			fprintf_s(fp, "L %s: LogMessege log file session started (file \"UKRCOOP_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);
		}
		va_list ap;
		va_start(ap, mes);
		LogToFiles(fp, mes, ap);
		va_end(ap);
		fclose(fp);
	}
}
