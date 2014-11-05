// testSerialization.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "S11nDeserializer.h"
#include "S11nXML.h"
#include "S11nJSON.h"
#include <sstream>

struct TestStruct1
{
	TestStruct1(): lng(0), bol(false) { }
	static const char* xml_element_name() { return "struct"; }
	template<class Helper>void serialization(Helper& sr)
	{
		sr.field(attr, "@attr");
		sr.field(str, "stringfield");
		sr.field(lng, "longfield");
		sr.field(bol, "boolfield");
	}
	CString attr;
	CString str;
	long lng;
	bool bol;
};

bool test1()
{
	const char * xml = // "<?xml version=\"1.0\" ?>\n"
		"<struct attr=\"first\">\n"
		"\t<stringfield>Hi, here&amp;there</stringfield>\n"
		"\t<longfield>123454321</longfield>\n"
		"\t<boolfield>true</boolfield>\n"
		"</struct>\n";

	struct TestStruct1 ts1;
	Serialization::Deserializer<TestStruct1> d1(ts1);
	Serialization::XMLParser p1(d1);
	p1.parse(xml);

	if(ts1.attr != _T("first"))
		return false;
	if(ts1.str != _T("Hi, here&there"))
		return false;
	if(ts1.lng != 123454321L)
		return false;
	if(! ts1.bol)
		return false;

	std::ostringstream x;
	Serialization::Serializer<TestStruct1> s1(ts1);
	Serialization::XMLWriter w1(x);
	s1.write(w1);
	if(x.str() != xml)
		return false;

	return true;
}

bool test2()
{
	const char * xml = "{"
		"\"@attr\": \"first\","
		"\"stringfield\": \"Hi, here&there\","
		"\"longfield\": 123454321,"
		"\"boolfield\": true\n"
		"}";

	struct TestStruct1 ts1;
	Serialization::Deserializer<TestStruct1> d1(ts1);
	Serialization::JSONParser p1(d1);
	p1.parse(xml);

	if(ts1.attr != _T("first"))
		return false;
	if(ts1.str != _T("Hi, here&there"))
		return false;
	if(ts1.lng != 123454321L)
		return false;
	if(! ts1.bol)
		return false;

	std::ostringstream x;
	Serialization::Serializer<TestStruct1> s1(ts1);
	Serialization::JSONWriter w1(x);
	s1.write(w1);
	if(x.str() != xml)
		return false;

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int rc = 0;
	if(! test1())
		++rc;
	if(! test2())
		++rc;
	return rc;
}

