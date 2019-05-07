#ifndef _LOG_MESSEGE_H_
#define _LOG_MESSEGE_H_

#include "extension.h"

class LM
{
public:
	/**
	 *	�������� ������� ��� � ������ ���� ����!
	 */
	void InitLogMesseg(void);

	/**
	 *	*******��� ��������*******
	 *	��������� ����������� � ���� ����
	 *	� ���� �� ��� �������� ���� ���� �������!
	 *	@return ������� ������� 1 ���� ������ i 0 ���� �!
	 */
	void LogToFileEx(const char *mes, ...);
private:
	logvalues g_pLogValues;
	void LogToFiles(FILE *fp, const char *mes, va_list ap);
};

extern LM			m_sLog;
#endif