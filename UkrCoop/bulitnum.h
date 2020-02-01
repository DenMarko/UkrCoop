#ifndef bulit_num_heder
#define bulit_num_heder

class CBulitNumber
{
public:
	CBulitNumber(void);
	int GetBulitNum(void);
	const char *GetBuletVersionNum(void);
	const char *GetBulitsNum(void);
private:
	void ComputeBulitNumber(void);

	int m_nBulitNum;
	char c_nBuletNum[64];
	char c_nVersionBulitNum[128];
};
extern const char *bulit_version_number_char(void);
extern const char *bulit_number_char(void);
extern int bulit_number(void);

#endif // !bulit_num_heder
