#ifndef S11NCOMMON_H_INCLUDED
#define S11NCOMMON_H_INCLUDED

#include <tchar.h>

namespace Serialization {

	struct IDeserializer
	{
		virtual void data(const char* path, LPCTSTR value) = 0;
		virtual void endelem(const char* path) = 0;
	};

	struct IWriter
	{
		virtual void startelem(const char* name) = 0;
		virtual void data(const char* name, LPCTSTR val, char format) = 0;
		virtual void endelem(const char* name) = 0;
	};
	
} // namespace Serialization

#endif /* S11NCOMMON_H_INCLUDED */
