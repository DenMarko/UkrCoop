#include "log_messege.h"
#include "LuaBridge/LuaBridge.h"

template<typename T> using result = expected<T, error_code>;

// Портативний thread-safe localtime
static tm safe_localtime(const time_t* t)
{
    tm result{};
    localtime_r(t, &result);
    return result;
}

// Конструктор LM::LM() реєструє клас LM у LuaBridge як Log з методом LogMessage, 
// який викликає LogToFileEx, та встановлює поточний об’єкт як глобальну змінну Log у стані Lua.
LM::LM()
{
	luabridge::getGlobalNamespace(g_Sample.GetLuaState())
		.beginClass<LM>("Log")
			.addFunction("LogMessage", std::function<int(LM*, const char*)>([](LM* pThis, const char* msg)
			{
				pThis->LogToFileEx(true, "[LUA] %s", msg);
				return 1;
			}))
		.endClass();
	luabridge::setGlobal(g_Sample.GetLuaState(), this, "Log");
}

LM::~LM()
{
	if(m_file.is_open())
		m_file.close();
}

// LM::InitLogMesseg() — корутина fire_and_forget,
// яка на фоні через g_ThreadPool.schedule() ініціалізує щоденний лог-файл UKRCOOP_YYYYMMDD.log,
// встановлює імʼя файлу й дату в info, а потім записує початковий заголовок у лог.
fire_and_forget LM::InitLogMesseg(void)
{
	co_await g_ThreadPool.schedule();

	time_t t = time(NULL);
	tm curtime = safe_localtime(&t); // копія, не вказівник

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		info.m_NrmCurDay = curtime.tm_mday;

		char path[PLATFORM_MAX_PATH];
		g_pSM->BuildPath(Path_SM, path, sizeof(path), "/logs/UKRCOOP_%04d%02d%02d.log", curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday);

		info.m_NrmFileName.assign(path);
		info.m_DailPrinted = true;
	}

	char header[512];
	snprintf(
		header, 
		sizeof(header), 
		"LogMessege log file session started (file \"logs/UKRCOOP_%04d%02d%02d.log\") (Version \"%s\")", 
		curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday, SMEXT_CONF_VERSION);

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		info.m_DailPrinted = false;
	}

	auto res = co_await WriteToLog(header, true);
	if(!res)
	{
		Msg("Failed to initialize LogMessege: ERROR: %d\n", to_string(res.error()));
	}

	co_return;
}

// LM::LogToFileAsync — це асинхронна корутина, 
// яка планує виконання у g_ThreadPool, 
// потім викликає WriteToLog(message, silent) і повертає результат запису у лог як bool (з огляду на декларацію, із підтримкою error_code).
task<bool> LM::LogToFileAsync(bool silent, SourceHook::String message)
{
	co_await g_ThreadPool.schedule();
    co_return co_await WriteToLog(message, silent);
}

// LM::WriteToLog — це корутинна функція, 
// що під потоком безпеки оновлює інформацію про файл, 
// додає до лог-файлу рядок з міткою часу та повідомленням, 
// при потребі вставляє рядок початку сесії і, якщо silent хибний, 
// виводить те саме повідомлення на консоль; у разі неможливості відкрити файл повертає помилку.
task<bool> LM::WriteToLog(SourceHook::String message, bool silent)
{
	time_t t = time(NULL);
	tm curtime = safe_localtime(&t); // копія, не вказівник

	SourceHook::String filename;
	bool need_update = false;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		UpdateFileInfoIfNeeded(&curtime);
		filename = info.m_NrmFileName;
		need_update = info.m_DailPrinted;
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

	char data[64];
	strftime(data, sizeof(data), "%d.%m.%Y %H:%M:%S", &curtime);

	if(need_update)
	{
		char header[512];
		int len = snprintf(header, sizeof(header),
			"LogMessege log file session started (file \"logs/UKRCOOP_%04d%02d%02d.log\") (Version \"%s\")",
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
	int log_len = snprintf(log_entry, sizeof(log_entry), "L [%s] %s\n", data, message.c_str());
	auto write_res = co_await m_file.write(log_entry, static_cast<size_t>(log_len));
	if(!write_res) {
		co_return result<bool>::err(write_res.error());
	}

	if (!silent) g_SMAPI->ConPrintf("[%s] %s\n", data, message.c_str());

	auto flush_res = co_await m_file.flush();
	if(!flush_res) {
		co_return result<bool>::err(flush_res.error());
	}

    co_return result<bool>::ok(true);
}

// LM::UpdateFileInfoIfNeeded — це допоміжна функція, 
// яка перевіряє, чи змінилася дата, 
// і якщо так, оновлює імʼя файлу та дату в info, 
// а також встановлює прапорець для вставки рядка початку сесії при наступному записі.
void LM::UpdateFileInfoIfNeeded(const tm *curtime)
{
	if (info.m_NrmCurDay != curtime->tm_mday)
	{
		char Path[PLATFORM_MAX_PATH];
		g_pSM->BuildPath(Path_SM, Path, sizeof(Path), 
		"/logs/UKRCOOP_%04d%02d%02d.log", 
		curtime->tm_year + 1900, 
		curtime->tm_mon + 1, 
		curtime->tm_mday);

		if(m_file.is_open())
			m_file.close();

		info.m_NrmFileName.assign(Path);
		info.m_NrmCurDay = curtime->tm_mday;
		info.m_DailPrinted = true;
	}
}
