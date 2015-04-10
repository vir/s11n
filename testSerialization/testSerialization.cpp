// testSerialization.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "S11nMain.h"
#include "S11nXML.h"
#include "S11nJSON.h"
#include <sstream>
#include <vector>
#include <iostream>

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
		"\"@attr\": \"first\", "
		"\"stringfield\": \"Hi, here&there\", "
		"\"longfield\": 123454321, "
		"\"boolfield\": true"
		"}";

	std::istringstream ss(xml);
	Serialization::JSONParser p1(ss);
	struct TestStruct1 ts1;
	Serialization::Deserializer<TestStruct1> d1(ts1);
	p1.parse(d1);

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

class CTestStructList: public std::vector<struct TestStruct1>
{
public:
	static const char* xml_element_name() { return "struct-list"; }
};

bool test3()
{
	const char * xml = // "<?xml version=\"1.0\" ?>\n"
		"<struct-list>\n"
		"\t<struct attr=\"first\">\n"
		"\t\t<stringfield>Hi, here&amp;there</stringfield>\n"
		"\t\t<longfield>123454321</longfield>\n"
		"\t\t<boolfield>true</boolfield>\n"
		"\t</struct>\n"
		"\t<struct attr=\"second\">\n"
		"\t\t<stringfield>Oh, shit!</stringfield>\n"
		"\t\t<longfield>666</longfield>\n"
		"\t\t<boolfield>false</boolfield>\n"
		"\t</struct>\n"
		"\t<struct attr=\"third\">\n"
		"\t\t<stringfield>OMFG</stringfield>\n"
		"\t\t<longfield>1998</longfield>\n"
		"\t\t<boolfield>true</boolfield>\n"
		"\t</struct>\n"
		"</struct-list>\n";

	CTestStructList v1;
	Serialization::TableDeserializer<CTestStructList> d1(v1);
	Serialization::XMLParser p1(d1);
	p1.parse(xml);

	if(v1.size() != 3)
		return false;
	if(v1[0].attr != _T("first"))
		return false;
	if(v1[0].str != _T("Hi, here&there"))
		return false;
	if(v1[0].lng != 123454321L)
		return false;
	if(! v1[0].bol)
		return false;

	std::ostringstream x;
	Serialization::TableSerializer<CTestStructList> s1(v1);
	Serialization::XMLWriter w1(x);
	s1.write(w1);
	if(x.str() != xml)
	{
		std::cout << "EXP: " << xml << std::endl << "GOT: " << x.str() << std::endl;
		return false;
	}

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int rc = 0;
	if(! test1())
		++rc;
	if(! test2())
		++rc;
	if(! test3())
		++rc;
	return rc;
}

