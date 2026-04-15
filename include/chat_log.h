#ifndef _CHAT_LOG_H_
#define _CHAT_LOG_H_

#include "extension.h"
#include <mutex>
#include <string>

class chat_log
{
public:
    task<void> InitChatLog();

    void ChatLogMsg(const char* format, ...);

    task<bool> ChatLogMsgAsync(SourceHook::String message);

private:
    task<bool> WriteToLog(SourceHook::String message); // const char* — не сирий вказівник

    void UpdateFileInfoIfNeeded(const tm* current_time);

    CLogInit    info;
    std::mutex  m_mutex;
};

extern chat_log* m_sChatLog;

#endif