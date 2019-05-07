#ifndef bulit_num_heder
#define bulit_num_heder

class CBulitNumber
{
public:
	CBulitNumber(void);
	int GetBulitNum(void);
	const char *GetBuletNum(void);
private:
	void ComputeBulitNumber(void);

	int m_nBulitNum;
	char c_nBuletNum[64];
};
extern const char *bulit_number_char(void);
extern int bulit_number(void);

#endif // !bulit_num_heder
