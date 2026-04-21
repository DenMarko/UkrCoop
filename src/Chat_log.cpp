#include "chat_log.h"
#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <string>

template<typename T> using result = expected<T, error_code>;

// Портативний thread-safe localtime
static tm safe_localtime(const time_t* t)
{
    tm result{};
    localtime_r(t, &result);
    return result;
}

chat_log::~chat_log()
{
    if(m_file.is_open())
        m_file.close();
}

// chat_log::InitChatLog() — це асинхронна coroutine fire_and_forget,
// яка переходить на g_ThreadPool, визначає поточну дату,
// під захистом м’ютексу встановлює ім’я файлу та стан логування,
// а потім записує заголовок сесії в файл чату й повідомляє про помилку, якщо запис не вдався.
fire_and_forget chat_log::InitChatLog()
{
    co_await g_ThreadPool.schedule();

    time_t t = time(nullptr);
    tm curtime = safe_localtime(&t); // копія, не вказівник

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        info.m_NrmCurDay = curtime.tm_mday;

        char path[PLATFORM_MAX_PATH];
        g_pSM->BuildPath(Path_SM, path, sizeof(path),
            "/logs/CHAT_LOG_%04d%02d%02d.log",
            curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);

        info.m_NrmFileName.assign(path);
        info.m_DailPrinted = true;
    }

    char header[512];
    snprintf(header, sizeof(header),
        "ChatLog log file session started (file \"/logs/CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")",
        curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        info.m_DailPrinted = false;
    }

    auto res = co_await WriteToLog(header);
    if(!res)
        Msg("Failed to initialize ChatLog: ERROR: %d\n", to_string(res.error()));

    co_return;
}

// Асинхронна функція-член chat_log::ChatLogMsgAsync приймає SourceHook::String message, 
// спочатку чекає планування в g_ThreadPool, 
// а потім записує повідомлення в лог, повертаючи task<bool, error_code>.
task<bool> chat_log::ChatLogMsgAsync(SourceHook::String message)
{
    co_await g_ThreadPool.schedule();
    co_return co_await WriteToLog(message);
}

// Метод chat_log::UpdateFileInfoIfNeeded перевіряє, 
// чи відрізняється день у переданому current_time від збереженого, 
// і якщо так, генерує нове ім’я щоденного лог-файлу, 
// оновлює ім’я файлу, поточний день та встановлює прапорець m_DailPrinted.
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

        if(m_file.is_open())
            m_file.close();

        info.m_NrmFileName.assign(path);
        info.m_NrmCurDay = current_time->tm_mday;
        info.m_DailPrinted = true;
    }
}

// chat_log::WriteToLog асинхронно зберігає рядок у файл логів чату, 
// підхоплюючи поточний час, 
// оновлюючи інформацію про файл під захистом м’ютекса та за потреби додаючи заголовок сесії. 
// Функція повертає task<bool, error_code>, де успіх позначається true, 
// а помилка відкриття файлу повертає відповідний error_code.
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
    }

    if(!m_file.is_open())
    {
        auto open_res = co_await m_file.open(
            filename.c_str(),
            OpenFlags::Write | OpenFlags::Append | OpenFlags::Create
        );

        if (!open_res) {
            co_return result<bool>::err(open_res.error());
        }
    }

    char date[64];
    strftime(date, sizeof(date), "%d.%m.%Y %H:%M:%S", &curtime);

    if (need_header)
    {
        char header[512];
        int len = snprintf(header, sizeof(header),
            "ChatLog log file session started (file \"/logs/CHAT_LOG_%04d%02d%02d.log\") (Version \"%s\")",
            curtime.tm_year + 1900, 
            curtime.tm_mon + 1, 
            curtime.tm_mday, 
            SMEXT_CONF_VERSION
        );
        
        auto write_res = co_await m_file.write(header, static_cast<size_t>(len));
        if(!write_res) {
            co_return result<bool>::err(write_res.error());
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            info.m_DailPrinted = false;
        }
    }

    char log_entry[4096];
    int len = snprintf(log_entry, sizeof(log_entry), "[%s] %s\n", date, message.c_str());
    auto write_res = co_await m_file.write(log_entry, static_cast<size_t>(len));
    if(!write_res) {
        co_return result<bool>::err(write_res.error());
    }

    auto flush_res = co_await m_file.flush();
    if(!flush_res) {
        co_return result<bool>::err(flush_res.error());
    }

    co_return result<bool>::ok(true);
}