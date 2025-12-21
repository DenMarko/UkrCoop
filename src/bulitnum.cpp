#include "extension.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"

char data[] = __DATE__;

char *mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
char mond[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

class CBulitNumber
{
public:
	CBulitNumber(void)
	{
		ComputeBulitNumber();
		sprintf(c_nBuletNum, "%i", this->m_nBulitNum);
		sprintf(szVersion, "1.%d.%03d", this->m_nBulitNum / 1000, this->m_nBulitNum % 1000);
		sprintf(c_nFullVersion, "%s (Build Number %i)", szVersion, this->m_nBulitNum);
	}

	int GetBulitNum(void) const
	{
		return m_nBulitNum;
	}

	const char *GetBuletNum(void) const
	{
		return c_nBuletNum;
	}

	const char *GetBulitVersionNum(void) const
	{
		return c_nFullVersion;
	}

	const char *GetBuiltVecrsion(void) const
	{
		return szVersion;
	}
private:
	void ComputeBulitNumber(void)
	{
		int m = 0;
		int d = 0;
		int y = 0;

		for(m = 0; m < 11; m++)
		{
			if(V_strncasecmp(&data[0], mon[m], 3) == 0)
				break;
			d += mond[m];
		}

		d += atoi(&data[4]) - 1;
		y = atoi(&data[7]) - 1900;

		m_nBulitNum = d + int((y - 1) * 365.25);

		if(((y % 4) == 0) && m > 1)
		{
			m_nBulitNum += 1;
		}

		m_nBulitNum -= 40542; // 1 січня 2012 приблизна дата створення сервера  
	}

	int m_nBulitNum;
	char szVersion[64];
	char c_nBuletNum[64];
	char c_nFullVersion[125];
};

static CBulitNumber cBulNum;
int bulit_number(void)
{
	return cBulNum.GetBulitNum();
}

const char *bulit_version_number_char(void)
{
    return cBulNum.GetBulitVersionNum();
}

const char *bulit_number_char(void)
{
	return cBulNum.GetBuletNum();
}

const char *build_version()
{
	return cBulNum.GetBuiltVecrsion();
}