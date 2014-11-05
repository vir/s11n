#include "stdafx.h"
#include "S11nJSON.h"
#include <string>
#include <sstream>

Serialization::JSONParser::JSONParser(Serialization::IDeserializer& deserializer)
	: sink(deserializer)
	, state(TOP)
{
}

void Serialization::JSONParser::parse(const char* json)
{
	while(*json)
	{
		skip_space(json);
		switch (*json)
		{
		case '[':
			parse_array(json);
			++json;
			break;
		case '{':
			parse_object(json);
			++json;
			break;
		default:
			ASSERT(0);
			break;
		}
		++json;
	}
}

void Serialization::JSONParser::skip_space(const char*& p)
{
	while(*p && isspace(*p))
		++p;
}

void Serialization::JSONParser::parse_array(const char*& json)
{
	ASSERT(0);
}

void Serialization::JSONParser::parse_object(const char*& json)
{
	std::string key, val;
	path += "/*";
	ASSERT(*json == '{');
	++json;

	while(*json && *json != '}')
	{
		int c;
		skip_space(json);
		parse_string(json, key);
		++json;
		skip_space(json);
		if(*json != ':')
			throw std::exception("bad object");
		++json;
		skip_space(json);
		switch (*json)
		{
		case '\"':
			parse_string(json, val);
			break;
		default:
			c = strcspn(json, ",} \t\r\n");
			val.assign(json, c);
			json += c;
			break;
		}
		sink.data((path + '/' + key).c_str(), val.c_str());
		++json;
		skip_space(json);
		if(*json == ',')
		{
			++json;
			skip_space(json);
		}
	}
	sink.endelem(path.c_str());
}

void Serialization::JSONParser::parse_string(const char*& json, std::string& s)
{
	std::ostringstream ss;
	if(*json != '\"')
		throw std::exception("bad json string");
	++json;
	while(*json && *json != '\"')
	{
		if(*json == '\\')
		{
			++json;
			switch (*json)
			{
			case '\"':
			case '/':
			case '\\':
				ss << *json;
				break;
			case 'b':
				ss << '\x8';
				break;
			case 'f':
				ss << '\xC';
				break;
			case 'n':
				ss << '\n';
				break;
			case 'r':
				ss << '\r';
				break;
			case 't':
				ss << '\t';
				break;
			case 'u':
			default:
				ASSERT(0);
				break;
			}
		}
		else
		{
			ss << *json;
		}
		++json;
	}
	s = ss.str();
}




static std::string json_escape(const char* s)
{
	return s;
}

void Serialization::JSONWriter::startelem(const char* name)
{
	stream << "{";
	need_comma = false;
}

void Serialization::JSONWriter::data(const char* name, const char* val, char format)
{
	if(need_comma)
		stream << ", ";
	stream << "\"" << json_escape(name) << "\": ";
	switch (format)
	{
	case 'b':
	case 'n':
		stream << val;
		break;
	default:
		stream << '"' << json_escape(val) << '"';
		break;
	}
	need_comma = true;
}

void Serialization::JSONWriter::endelem(const char* name)
{
	stream << "}";
	need_comma = true;
}
