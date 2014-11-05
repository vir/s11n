#include "stdafx.h"
#include "S11nXML.h"
#include <stdexcept>

static CString FromUtf8(const char * s)
{
	return CString(CStringA(s));
}


Serialization::XMLParser::XMLParser(Serialization::IDeserializer& deserializer) : sink(deserializer)
{
	parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, StartElementHandler, EndElementHandler);
	XML_SetCharacterDataHandler(parser, CharacterDataHandler);
	XML_SetUserData(parser, this);
}

Serialization::XMLParser::~XMLParser()
{
	XML_ParserFree(parser);
}

void Serialization::XMLParser::parse(const char* buf, size_t len, bool final)
{
	XML_Parse(parser, buf, len, final);
}

void Serialization::XMLParser::parse(const char* xml)
{
	parse(xml, ::strlen(xml), true);
}

void Serialization::XMLParser::StartElementHandler(void *userData, const XML_Char *name, const XML_Char **atts)
{
	Serialization::XMLParser * self = reinterpret_cast<Serialization::XMLParser *>(userData);
	// flush cdata accumulator first
	if(self->m_cdata.length()) {
		self->sink.data(self->m_path.c_str(), FromUtf8(self->m_cdata.c_str()));
		self->m_cdata.clear();
	}

	// append element name to path
	self->m_path += '/';
	self->m_path += name;
	//printf("start element(%p, %d, %s, %s, %p), path=%s\n", userdata, parent, nspace, name, attrs, self->m_path.c_str());

	// extract attributes
	for(int i = 0; atts[i]; i+=2) {
		//printf("Attr %d: %s = %s\n", i, atts[i], atts[i+1]);
		self->sink.data((self->m_path + "/@" + atts[i]).c_str(), FromUtf8(atts[i+1]));
	}
	//return ++(self->m_counter);
}

void Serialization::XMLParser::EndElementHandler(void *userData, const XML_Char *name)
{
	Serialization::XMLParser * self = reinterpret_cast<Serialization::XMLParser *>(userData);
	//printf("endelem(%p, %d, %s, %s), path = %s\n", userdata, state, nspace, name, self->m_path.c_str());
	// flush cdata accumulator first
	if(self->m_cdata.length()) {
		self->sink.data(self->m_path.c_str(), FromUtf8(self->m_cdata.c_str()));
		self->m_cdata.clear();
	}

	// check for valid element
	unsigned int slash = self->m_path.rfind('/');
	if(slash == std::string::npos || self->m_path.substr(slash + 1) != name)
		throw std::exception("bad xml");

	// notify descentants
	self->sink.endelem(self->m_path.c_str());

	// strip trailing path portion
	self->m_path.erase(slash);
}

void Serialization::XMLParser::CharacterDataHandler(void *userData, const XML_Char *s, int len)
{
	Serialization::XMLParser * self = reinterpret_cast<Serialization::XMLParser *>(userData);
	//printf("cdata(%p, %d, %p, %d), path = %s\n", userdata, state, buf, len, self->m_path.c_str());
	self->m_cdata.append(s, len);
}



static std::string escape_xml(const char* s)
{
	size_t len = 0;
	for(const char* t = s; *t; ++t)
	{
		switch(*t) {
		case '\"': len += 6; break;
		case '&': len += 5; break;
		case '<': case '>': len += 4; break;
		}
	}
	std::string res;
	res.reserve(len);
	while(*s)
	{
		switch(*s)
		{
		case '&':  res.append("&amp;");       break;
		case '\"': res.append("&quot;");      break;
		case '<':  res.append("&lt;");        break;
		case '>':  res.append("&gt;");        break;
		default:   res.append(s, 1);          break;
		}
		++s;
	}
	return res;
}



void Serialization::XMLWriter::startelem(const char* name)
{
	for(unsigned int i = 0; i < level; ++i)
		stream << '\t';
	stream << "<" << name;
	++level;
}

void Serialization::XMLWriter::data(const char* name, LPCTSTR val, char format)
{
	if(name[0] == '@')
	{
		stream << " " << name + 1 << "=\"" << escape_xml(encode(val)) << '\"';
	}
	else
	{
		for(unsigned int i = 0; i < level; ++i)
			tmp += '\t';
		tmp += "<";
		tmp += name;
		tmp += ">";
		tmp += escape_xml(encode(val));
		tmp += "</";
		tmp += name;
		tmp += ">\n";
	}
}

void Serialization::XMLWriter::endelem(const char* name)
{
	--level;
	stream << ">\n" << tmp;
	for(unsigned int i = 0; i < level; ++i)
		stream << '\t';
	stream << "</" << name << ">\n";
}

CStringA Serialization::XMLWriter::encode(LPCTSTR s)
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
