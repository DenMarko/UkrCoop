#ifndef _CHAT_LOG_H_
#define _CHAT_LOG_H_

#include "extension.h"

class chat_log
{
public:
	/**
	 *	Ініціалізує функцію чат лог і создає файл лога!
	 */
	void InitChatLog(void);

	/**
	 *	*******Чат лог*******
	 *	Добавляем повідомлення в файл лога
	 *	і якщо не був созданий файл лога создаем!
	 *	@return повертає значеня true якщо успішно i false якщо ні!
	 */
	void ChatLogMsg(const char *msg, ...);
private:
	logvalues g_pLogChatValues;
	void ChatLogToFile(FILE *fp, const char* msg, va_list ap);
};
#endif