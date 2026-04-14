#ifndef _LOG_MESSEGE_H_
#define _LOG_MESSEGE_H_

#include "extension.h"

class LM
{
public:
    LM();
    void InitLogMesseg(void);
    int LogToFileEx(bool silent, const char *mes, ...);

private:
    void LogToFiles(FILE *fp, const char *mes, va_list ap, bool silent = false);
    void PrintDeilMessage(tm* curtime, FILE* fp);
    void UpdateFileName(tm* curtime);

    CLogInit info;
};

extern LM	*m_sLog;
#endif
