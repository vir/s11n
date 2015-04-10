#ifndef S11NJSON_H_INCLUDED
#define S11NJSON_H_INCLUDED

#include <iosfwd>
#include <string>
#include "S11nMain.h"

namespace Serialization {

	class JSONParser
	{
		const static size_t bufsize = 4096;
	public:
		JSONParser(std::istream& stream, UINT encoding = CP_UTF8);
		~JSONParser();
		void parse(Serialization::IDeserializer& obj)
		{
			sink = &obj;
			parse();
			sink = NULL;
		}
	protected:
		void parse();
		void skip_space();
		void parse_array();
		void parse_object();
		void parse_string(std::string& s);
	private:
		CString decode(const char* s);
		std::istream& stream;
		char* buf;
		char* json;
		char* deadline;
		Serialization::IDeserializer* sink;
		std::string path;
		UINT enc;
		bool eof;
	};

	class JSONWriter: public Serialization::IWriter
	{
	public:
		JSONWriter(std::ostream& stream, UINT encoding = CP_UTF8)
			: stream(stream)
			, need_comma(false)
			, enc(encoding)
		{
		}
	public:
		virtual void startelem(const char* name);
		virtual void data(const char* name, LPCTSTR val, char format);
		virtual void endelem(const char* name);
	private:
		CStringA encode(LPCTSTR s);
		std::ostream& stream;
		bool need_comma;
		UINT enc;
	};

} // namespace Serialization

#endif /* S11NJSON_H_INCLUDED */
