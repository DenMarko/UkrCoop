#ifndef _LOG_MESSEGE_H_
#define _LOG_MESSEGE_H_

#include "extension.h"

class LM
{
public:
	/**
	 *	Ініціалізує функцію лог і создаєм файл лога!
	 */
	void InitLogMesseg(void);

	/**
	 *	*******Лог менеджер*******
	 *	Добавляем повідомлення в файл лога
	 *	і якщо не був созданий файл лога создаем!
	 *	@return повертає значеня 1 якщо успішно i 0 якщо ні!
	 */
	void LogToFileEx(const char *mes, ...);
private:
	logvalues g_pLogValues;
	void LogToFiles(FILE *fp, const char *mes, va_list ap);
};

extern LM			m_sLog;
#endif