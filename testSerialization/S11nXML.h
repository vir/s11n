#ifndef S11NXML_H_INCLUDED
#define S11NXML_H_INCLUDED

#include "S11nMain.h"
#include <string>
#include <stack>

namespace Serialization {

#include "expat.h"

	class XMLParser
	{
	public:
		XMLParser(Serialization::IDeserializer& deserializer);
		~XMLParser();
		void parse(const char* buf, size_t len, bool final);
		void parse(const char* xml);
	private:
		static void StartElementHandler(void *userData, const XML_Char *name, const XML_Char **atts);
		static void EndElementHandler(void *userData, const XML_Char *name);
		static void CharacterDataHandler(void *userData, const XML_Char *s, int len);
	private:
		IDeserializer& sink;
		XML_Parser parser;
		std::string m_path, m_cdata;
	};

	class XMLWriter:public Serialization::IWriter
	{
	public:
		XMLWriter(std::ostream& output_stream, UINT encoding = CP_UTF8)
			: stream(output_stream)
			, level(0)
			, enc(encoding)
			, attrs(false)
		{
		}
	public:
		virtual void startelem(const char* name, char type);
		virtual void data(const char* name, LPCTSTR val, char format);
		virtual void endelem(const char* name);
	private:
		CStringA encode(LPCTSTR s);
		std::ostream& stream;
		unsigned int level;
		UINT enc;
		bool attrs;
	};

} // namespace Serialization

#endif /* S11NXML_H_INCLUDED */
