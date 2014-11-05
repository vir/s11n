#ifndef S11NJSON_H_INCLUDED
#define S11NJSON_H_INCLUDED

#include <iosfwd>
#include <string>
#include "S11nCommon.h"

namespace Serialization {

	class JSONParser
	{
		enum State { TOP, ARRAY, MAP };
	public:
		JSONParser(Serialization::IDeserializer& deserializer);
		void parse(const char* json);
	protected:
		void skip_space(const char*& p);
		void parse_array(const char*& json);
		void parse_object(const char*& json);
		void parse_string(const char*& json, std::string& s);
	private:
		Serialization::IDeserializer& sink;
		std::string path;
		State state;
	};

	class JSONWriter: public Serialization::IWriter
	{
	public:
		JSONWriter(std::ostream& stream)
			: stream(stream)
			, need_comma(false)
		{
		}
	public:
		virtual void startelem(const char* name);
		virtual void data(const char* name, const char* val, char format);
		virtual void endelem(const char* name);
	private:
		std::ostream& stream;
		bool need_comma;
	};

} // namespace Serialization

#endif /* S11NJSON_H_INCLUDED */
