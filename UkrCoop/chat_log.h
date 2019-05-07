#ifndef _CHAT_LOG_H_
#define _CHAT_LOG_H_

#include "extension.h"

class chat_log
{
public:
	/**
	 *	�������� ������� ��� ��� � ����� ���� ����!
	 */
	void InitChatLog(void);

	/**
	 *	*******��� ���*******
	 *	��������� ����������� � ���� ����
	 *	� ���� �� ��� �������� ���� ���� �������!
	 *	@return ������� ������� true ���� ������ i false ���� �!
	 */
	void ChatLogMsg(const char *msg, ...);
private:
	logvalues g_pLogChatValues;
	void ChatLogToFile(FILE *fp, const char* msg, va_list ap);
};
#endif