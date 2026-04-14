#ifndef INCLUDE_SMEX_GEOIP
#define INCLUDE_SMEX_GEOIP
#include "IShareSys.h"

#define SMINTERFACE_GEOIP_NAME "IGeoIP"
#define SMINTERFACE_GEOIP_VERSION 1

namespace SourceMod
{
	class IGeoIP : public SMInterface
	{
	public:
		virtual const char *GetInterfaceName() = 0;
		virtual unsigned int GetInterfaceVersion() = 0;

	public:
		virtual const char *GetGeoIPCode2(char *ip) = 0;
		virtual const char *GetGeoIPCode3(char *ip) = 0;
		virtual const char *GetGeoIPCountry(char *ip) = 0;
	};
}

#endif // !INCLUDE_SMEX_GEOIP
