#ifndef _CHAT_LOG_H_
#define _CHAT_LOG_H_

#include "extension.h"

class chat_log
{
public:
    void InitChatLog(void);
    bool ChatLogMsg(const char *msg, ...);
private:
    void ChatLogToFile(FILE *fp, const char* msg, va_list ap);

    CLogInit info;
};

extern chat_log  *m_sChatLog;
#endif
