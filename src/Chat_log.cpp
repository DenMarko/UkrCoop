#include "chat_log.h"
#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <string>

// Портативний thread-safe localtime
static tm safe_localtime(const time_t* t)
{
    tm result{};
#if defined(_WIN32)
    localtime_s(&result, t);
#else
    localtime_r(t, &result);
#endif
    return result;
}

// ------------------------------------------------------------------
task<void> chat_log::InitChatLog()
{
    co_await g_ThreadPool.schedule();

    time_t t = time(nullptr);
    tm curtime = safe_localtime(&t); // копія, не вказівник

    std::string filename;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        info.m_NrmCurDay = curtime.tm_mday;

        char path[PLATFORM_MAX_PATH];
        g_pSM->BuildPath(Path_SM, path, sizeof(path),
            "/logs/CHAT_LOG_%04d%02d%02d.log",
            curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);

        info.m_NrmFileName.assign(path);
        info.m_DailPrinted = true;
        filename = path;
    }

    char header[512];
    snprintf(header, sizeof(header),
        "ChatLog log file session started (file \"/logs/CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")\n",
        curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        info.m_DailPrinted = false;
    }

    co_await WriteToLog(header);

    co_return;
}

// ------------------------------------------------------------------
void chat_log::ChatLogMsg(const char* format, ...)
{
    char buffer[3072];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    SourceHook::String message(buffer);

    [this, msg = std::move(message)]() mutable -> fire_and_forget {
        co_await WriteToLog(msg);
		co_return;
    }();
}

// ------------------------------------------------------------------
task<bool> chat_log::ChatLogMsgAsync(SourceHook::String message)
{
    co_await g_ThreadPool.schedule();
    co_return co_await WriteToLog(message);
}

// ------------------------------------------------------------------
void chat_log::UpdateFileInfoIfNeeded(const tm* current_time)
{
    if (info.m_NrmCurDay != current_time->tm_mday)
    {
        char path[PLATFORM_MAX_PATH];
        g_pSM->BuildPath(Path_SM, path, sizeof(path),
            "/logs/CHAT_LOG_%04d%02d%02d.log",
            current_time->tm_year + 1900,
            current_time->tm_mon + 1,
            current_time->tm_mday);

        info.m_NrmFileName.assign(path);
        info.m_NrmCurDay = current_time->tm_mday;
        info.m_DailPrinted = true;
    }
}

// ------------------------------------------------------------------
task<bool> chat_log::WriteToLog(SourceHook::String message)
{
    time_t t = time(nullptr);
    tm curtime = safe_localtime(&t); // thread-safe копія

    SourceHook::String filename;
    bool need_header = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        UpdateFileInfoIfNeeded(&curtime);
        filename    = info.m_NrmFileName;
        need_header = info.m_DailPrinted;
        if (need_header)
            info.m_DailPrinted = false;
    }

    FILE* fp = fopen(filename.c_str(), "a+");
    if (!fp)
        co_return false;

    if (need_header)
    {
        char date[64];
        strftime(date, sizeof(date), "%d.%m.%Y %H:%M:%S", &curtime);
        fprintf(fp,
            "L [%s] ChatLog log file session started (file \"/logs/CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")\n",
            date, curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);
    }

    char date[64];
    strftime(date, sizeof(date), "%d.%m.%Y %H:%M:%S", &curtime);
    fprintf(fp, "L [%s] %s\n", date, message.c_str());

    fclose(fp);
    co_return true;
}