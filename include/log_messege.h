#ifndef _LOG_MESSEGE_H_
#define _LOG_MESSEGE_H_

#include "extension.h"
#include <mutex>
#include <string>

class LM
{
public:
    LM();
    ~LM();

    fire_and_forget InitLogMesseg(void);

    template<typename... Args>
    fire_and_forget LogToFileEx(bool silent, const char *mes, Args&&... args)
    {
        char buffer[4096];
        if constexpr (sizeof...(args) > 0)
        {
            std::snprintf(buffer, sizeof(buffer), mes, std::forward<Args>(args)...);
        }
        else
        {
            std::snprintf(buffer, sizeof(buffer), "%s", mes);
        }

        SourceHook::String message(buffer);

        auto res = co_await WriteToLog(std::move(message), silent);
        if(!res)
            Msg("Failed to write to LogMessege: ERROR: %d\n", to_string(res.error()));

        co_return;
    }

    task<bool> LogToFileAsync(bool silent, SourceHook::String message);
private:
    task<bool> WriteToLog(SourceHook::String message, bool silent = false);
    void UpdateFileInfoIfNeeded(const tm* curtime);

    CLogInit info;
    std::mutex m_mutex;
    AsyncFileAuto m_file;
};

extern LM	*m_sLog;
#endif
