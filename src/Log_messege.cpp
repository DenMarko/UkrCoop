#include "log_messege.h"
#include "LuaBridge/LuaBridge.h"

LM::LM()
{
	luabridge::getGlobalNamespace(g_Sample.GetLuaState())
		.beginClass<LM>("Log")
			.addFunction("LogMessage", std::function<int(LM*, const char*)>([](LM* pThis, const char* msg) { return pThis->LogToFileEx(true, "[LUA] %s", msg) ? 1 : 0; }))
		.endClass();
	luabridge::setGlobal(g_Sample.GetLuaState(), this, "Log");
}

void LM::InitLogMesseg(void){
	time_t t = time(NULL);
	tm *curtime = localtime(&t);

	UpdateFileName(curtime);

	if(info.m_DailPrinted)
	{
		FILE *fp = fopen(info.m_NrmFileName.c_str(), "a+");
		PrintDeilMessage(curtime, fp);
		fclose(fp);
	}
}

void LM::LogToFiles(FILE *fp, const char *mes, va_list ap, bool silent) {
    char buffer[3072];
    g_pSM->FormatArgs(buffer, sizeof(buffer), mes, ap);

	char date[64];
    g_Sample.GetCurTime(date, sizeof(date));

    fprintf(fp, "L [%s] %s\n", date, buffer);
    if (!silent) g_SMAPI->ConPrintf("[%s] %s\n", date, buffer);
}

int LM::LogToFileEx(bool silent ,const char *mes, ...)
{
	time_t t = time(NULL);
	tm *curtime = localtime(&t);

	if(info.m_NrmCurDay != curtime->tm_mday)
	{
		UpdateFileName(curtime);
	}

	FILE *fp = fopen(info.m_NrmFileName.c_str(), "a+");
	if(!fp) return false;
	
	if(info.m_DailPrinted)
	{
		PrintDeilMessage(curtime, fp);
	}
	va_list ap;
	va_start(ap, mes);
	LogToFiles(fp, mes, ap, silent);
	va_end(ap);
	fclose(fp);
	return true;
}

void LM::UpdateFileName(tm* curtime)
{
	char Path[PLATFORM_MAX_PATH];
	g_pSM->BuildPath(Path_SM, Path, sizeof(Path), "/logs/UKRCOOP_%04d%02d%02d.log", curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
	info.m_NrmFileName.assign(Path);
	info.m_NrmCurDay = curtime->tm_mday;
	info.m_DailPrinted = true;
}

void LM::PrintDeilMessage(tm* curtime, FILE* fp)
{
	char date[64];
	info.m_DailPrinted = false;
	strftime(date, sizeof(date), "%d.%m.%Y %H:%M:%S", curtime);
	fprintf(fp, "%s: LogMessege log file session started (file \"UKRCOOP_%04d%02d%02d.log\") (Version \"%s\")\n", date, curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday, SMEXT_CONF_VERSION);
}