#ifndef _HEADER_ACTION_IMPL_INCLUDE_
#define _HEADER_ACTION_IMPL_INCLUDE_

#include "CActionShared.h"

class Autoswap
{
public:
	Autoswap() = delete;
	explicit Autoswap(const void* action);
	~Autoswap();

private:
	Autoswap(Autoswap&&) = delete;
	Autoswap(const Autoswap&) = delete;
	Autoswap& operator=(const Autoswap&) = delete;
	Autoswap& operator=(Autoswap&&) = delete;
private:
	void* m_action;
};

bool BeginActionProcessing(INextBotAction_ptr action);
bool StopActionProcessing(INextBotAction_ptr action);
void StopActionProcessing();


#endif