#ifndef UTILS_HEDERS
#define UTILS_HEDERS
#pragma once

#include "extension.h"

bool Translate(char *buffer, size_t maxlength, const char *format, unsigned int numparams, size_t *pOutLength, ...)
{
	va_list ap;
	unsigned int i;
	const char *fail_phrase;
	void *params[MAX_TRANSLATE_PARAMS];

	if (numparams > MAX_TRANSLATE_PARAMS)
	{
		assert(false);
		return false;
	}

	va_start(ap, pOutLength);
	for (i = 0; i < numparams; i++)
	{
		params[i] = va_arg(ap, void *);
	}
	va_end(ap);

	if (!ipharases->FormatString(buffer, maxlength, format, params, numparams, pOutLength, &fail_phrase))
	{
		if (fail_phrase != NULL)
		{
			g_IUkrCoop->UkrCoop_LogMessage("[UkrCoop] Could not find core phrase: %s", fail_phrase);
		}
		else {
			g_IUkrCoop->UkrCoop_LogMessage("[UkrCoop] Unknown fatal error while translating a core phrase.");
		}
		return false;
	}
	return true;
}

size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(buffer, maxlength, fmt, ap);
	va_end(ap);

	if (len >= maxlength)
	{
		buffer[maxlength - 1] = '\0';
		return (maxlength - 1);
	}
	else
		return len;
}


#endif //UTILS_HEDERS