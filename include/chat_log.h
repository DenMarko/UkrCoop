#ifndef _CHAT_LOG_H_
#define _CHAT_LOG_H_

#include "extension.h"
#include <mutex>
#include <string>

class chat_log
{
public:
    chat_log() = default;
    ~chat_log();

    fire_and_forget InitChatLog();

    template<typename... Args>
    fire_and_forget ChatLogMsg(const char* format, Args&&... args)
    {
        char buffer[4096];
        std::snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);

        SourceHook::String message(buffer);

        auto result = co_await WriteToLog(std::move(message));
        if(!result)
            Msg("Failed to write to ChatLog: ERROR: %d\n", to_string(result.error()));

        co_return;
    }

    task<bool> ChatLogMsgAsync(SourceHook::String message);

private:
    task<bool> WriteToLog(SourceHook::String message);

    void UpdateFileInfoIfNeeded(const tm* current_time);

    CLogInit    info;
    std::mutex  m_mutex;
    AsyncFileAuto m_file;
};

extern chat_log* m_sChatLog;

#endif