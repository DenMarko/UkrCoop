#include "bulitnum.h"
#include "extension.h"

char data[] = __DATE__;

char *mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
char mond[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//int V_strncasecmp (const char *s1, const char *s2, int n)
//{
//	while ( n-- > 0 )
//	{
//		int c1 = *s1++;
//		int c2 = *s2++;
//
//		if (c1 != c2)
//		{
//			if (c1 >= 'a' && c1 <= 'z')
//				c1 -= ('a' - 'A');
//			if (c2 >= 'a' && c2 <= 'z')
//				c2 -= ('a' - 'A');
//			if (c1 != c2)
//				return c1 < c2 ? -1 : 1;
//		}
//		if ( c1 == '\0' )
//			return 0;
//	}
//	
//	return 0;
//}

CBulitNumber::CBulitNumber(void)
{
	this->ComputeBulitNumber();
	g_pUkrCoop.UTIL_Format(c_nBuletNum, sizeof(c_nBuletNum), "%i", this->m_nBulitNum);
	g_pUkrCoop.UTIL_Format(c_nVersionBulitNum, sizeof(c_nVersionBulitNum), "1.1.30 (Assembly %i)", this->m_nBulitNum);
}

int CBulitNumber::GetBulitNum(void)
{
	return m_nBulitNum;
}

const char *CBulitNumber::GetBuletVersionNum(void)
{
	return c_nVersionBulitNum;
}

const char *CBulitNumber::GetBulitsNum()
{
	return c_nBuletNum;
}

void CBulitNumber::ComputeBulitNumber(void)
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

	m_nBulitNum -= 35739;
}

static CBulitNumber cBulNum;

int bulit_number(void)
{
	return cBulNum.GetBulitNum();
}
const char *bulit_version_number_char(void)
{
	return cBulNum.GetBuletVersionNum();
}

const char *bulit_number_char(void)
{
	return cBulNum.GetBulitsNum();
}
