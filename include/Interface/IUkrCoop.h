#ifndef __INCLUDE_SMEX_I_UKRCOOP
#define __INCLUDE_SMEX_I_UKRCOOP

#include "IShareSys.h"

#define SMINTERFACE_UKRCOOP_NAME "IUkrCoop"
#define SMINTERFACE_UKRCOOP_VERSION 1

namespace SourceMod
{
	class UkrCoop_Interface : public SMInterface
	{
	public:
        virtual const char* GetInterfaceName() = 0;
        virtual unsigned int GetInterfaceVersion() = 0;
	public:
		virtual void UCLogMessage(const char *msg, ...) = 0;
	};
}

#endif
