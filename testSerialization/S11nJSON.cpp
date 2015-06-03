#include "stdafx.h"
#include "S11nJSON.h"
#include <string>
#include <sstream>


Serialization::JSONParser::JSONParser(std::istream& stream, UINT encoding /*= CP_UTF8*/)
	: stream(stream), enc(encoding)
	, buf(NULL), json(NULL), deadline(NULL)
{
	buf = new char[bufsize + 1];
	buf[bufsize] = '\0'; // just to be on safe side
	json = buf;
	std::streamsize read = stream.readsome(buf, bufsize);
	deadline = buf + read;
	eof = stream.eof();
}

Serialization::JSONParser::~JSONParser()
{
	delete[] buf;
}

void Serialization::JSONParser::parse(const char* name /* = "*" */)
{
	skip_space();
	switch (*json)
	{
	case '\"':
		parse_string(name);
		break;
	case '[':
		parse_array(name);
		break;
	case '{':
		parse_object(name);
		break;
	default:
		parse_value(name);
		break;
	}
}

void Serialization::JSONParser::skip_space()
{
	while(*json && json != deadline)
	{
		if(json - buf > bufsize / 2)
		{
			size_t waste = (json - buf);
			::memmove(buf, json, deadline - json);
			json = buf;
			deadline -= waste;
			if(! eof)
				deadline += stream.readsome(deadline, waste);
			eof = stream.eof();
		}
		if(! isspace(*json))
			break;
		++json;
	}
}

void Serialization::JSONParser::parse_array(const char* name /* = "*" */)
{
	path += "/";
	path += name;
	ASSERT(*json == '[');
	++json;

	while(*json && *json != ']' && json != deadline)
	{
		parse();
		skip_space();
		if(*json == ',')
		{
			++json;
			skip_space();
		}
	}
	ASSERT(*json == ']');
	++json;
	sink->endelem(path.c_str());
	path.erase(path.find_last_of('/'));
}

void Serialization::JSONParser::parse_object(const char* name /* = "*" */)
{
	std::string key;
	path += "/";
	path += name;
	ASSERT(*json == '{');
	++json;

	while(*json && *json != '}' && json != deadline)
	{
		skip_space();
		parse_string2(key);
		skip_space();
		if(*json != ':')
			throw std::exception("bad object");
		++json;
		parse(key.c_str());
		skip_space();
		if(*json == ',')
		{
			++json;
			skip_space();
		}
	}
	ASSERT(*json == '}');
	++json;
	sink->endelem(path.c_str());
	path.erase(path.find_last_of('/'));
}

void Serialization::JSONParser::parse_string(const char* name /*= "*"*/)
{
	std::string val;
	parse_string2(val);
	sink->data((path + '/' + name).c_str(), decode(val.c_str()));
}

void Serialization::JSONParser::parse_string2(std::string& s)
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
	ASSERT(*json == '\"');
	if(*json == '\"')
		++json;
	s = ss.str();
}

bool Serialization::JSONParser::parse_value(const char* name /*= "*"*/)
{
	int c = strcspn(json, ",} \t\r\n");
	if(0 == c)
		return false;
	std::string val;
	val.assign(json, c);
	json += c;
	sink->data((path + "/" + name).c_str(), decode(val.c_str()));
	return true;
}

CString Serialization::JSONParser::decode(const char* s)
{
#ifndef _UNICODE
# error "ANSI Build not implemented"
#endif
	int len = ::MultiByteToWideChar(enc, 0, s, -1, NULL, 0);
	CString res;
	len = ::MultiByteToWideChar(enc, 0, s, -1, res.GetBuffer(len), len);
	res.ReleaseBuffer(len);
	return res;
}


static std::string json_escape(const char* s)
{
	std::ostringstream ss;
	while(*s)
	{
		switch(*s)
		{
		case '\\':
		case '\'':
		case '\"':
			ss << "\\" << *s;
			break;
		case 0x08:
			ss << "\\b";
			break;
		case 0x0C:
			ss << "\\f";
			break;
		case 0x0A:
			ss << "\\n";
			break;
		case 0x0D:
			ss << "\\r";
			break;
		case 0x09:
			ss << "\\t";
			break;
		default:
			ss << *s;
			break;
		}
		++s;
	}
	return ss.str();
}


void Serialization::JSONWriter::startelem(const char* name, char type)
{
	if(need_comma)
		stream << ", ";
	switch (type)
	{
	case 'a':
		stream << "\"" << json_escape(name) << "\": ["; //
		::strcat_s(braces, "]");
		break;
	case 'm':
		stream << "\"" << json_escape(name) << "\": ";
		/* no break */
	default:
		stream << "{";
		::strcat_s(braces, "}");
		break;
	}
	need_comma = false;
}

void Serialization::JSONWriter::data(const char* name, LPCTSTR val, char format)
{
	if(need_comma)
		stream << ", ";
	stream << "\"" << json_escape(name) << "\": ";
	switch (format)
	{
	case 'b':
	case 'n':
		stream << encode(val);
		break;
	case 'z':
		stream << "null";
		break;
	default:
		stream << '"' << json_escape(encode(val)) << '"';
		break;
	}
	need_comma = true;
}

void Serialization::JSONWriter::endelem(const char* name)
{
	char* p = braces;
	ASSERT(*p);
	if(!*p)
		return;
	while(p[1])
		++p;
	stream << p;
	*p = '\0';
	need_comma = true;
}

CStringA Serialization::JSONWriter::encode(LPCTSTR s)
{
#ifndef _UNICODE
# error "ANSI Build not implemented"
#endif
	int len = ::WideCharToMultiByte(enc, 0, s, -1, NULL, 0, NULL, NULL);
	CStringA res;
	len = ::WideCharToMultiByte(enc, 0, s, -1, res.GetBuffer(len), len, NULL, NULL);
	res.ReleaseBuffer(len);
	return res;
}

