// testSerialization.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "S11nMain.h"
#include "S11nXML.h"
#include "S11nJSON.h"
#include <sstream>
#include <vector>
#include <iostream>
#include <list>

struct TestStruct1
{
	TestStruct1(): lng(0), bol(false) { }
	static const char* xml_element_name() { return "struct"; }
	template<class Helper>void serialization(Helper& sr)
	{
		sr.field(attr, "@attr"); // attrs should go first
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
	Serialization::TableDeserializer<CTestStructList> d1(v1, v1.xml_element_name());
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
	Serialization::TableSerializer<CTestStructList> s1(v1, v1.xml_element_name());
	Serialization::XMLWriter w1(x);
	s1.write(w1);
	if(x.str() != xml)
	{
		std::cout << "EXP: " << xml << std::endl << "GOT: " << x.str() << std::endl;
		return false;
	}

	return true;
}

struct TestStruct2
{
	TestStruct2() { }
	static const char* xml_element_name() { return "super"; }
	template<class Helper>void serialization(Helper& sr)
	{
		sr.field(name, "@name"); // attrs should go first
		sr.complex(sub, "sub");
		sr.complex(sec, "another");
	}
	CString name;
	TestStruct1 sub, sec;
};

bool test4() // structure in structure
{
	const char * xml = // "<?xml version=\"1.0\" ?>\n"
		"<super name=\"supername\">\n"
		"\t<sub attr=\"first\">\n"
		"\t\t<stringfield>Hi, here&amp;there</stringfield>\n"
		"\t\t<longfield>123454321</longfield>\n"
		"\t\t<boolfield>true</boolfield>\n"
		"\t</sub>\n"
		"\t<another attr=\"second\">\n"
		"\t\t<stringfield>Good bye</stringfield>\n"
		"\t\t<longfield>999666</longfield>\n"
		"\t\t<boolfield>false</boolfield>\n"
		"\t</another>\n"
		"</super>\n";

	struct TestStruct2 ts1;
	Serialization::Deserializer<TestStruct2> d1(ts1);
	Serialization::XMLParser p1(d1);
	p1.parse(xml);

	if(ts1.name != _T("supername"))
		return false;
	if(ts1.sub.attr != _T("first"))
		return false;
	if(ts1.sub.str != _T("Hi, here&there"))
		return false;
	if(ts1.sub.lng != 123454321L)
		return false;
	if(! ts1.sub.bol)
		return false;
	if(ts1.sec.attr != _T("second"))
		return false;
	if(ts1.sec.str != _T("Good bye"))
		return false;
	if(ts1.sec.lng != 999666)
		return false;
	if(ts1.sec.bol)
		return false;

	std::ostringstream x;
	Serialization::Serializer<TestStruct2> s1(ts1);
	Serialization::XMLWriter w1(x);
	s1.write(w1);
	if(x.str() != xml)
	{
		std::cout << "EXP: " << xml << std::endl << "GOT: " << x.str() << std::endl;
		return false;
	}

	// build json
	x.str("");
	Serialization::JSONWriter w2(x);
	s1.write(w2);
	CStringA json(x.str().c_str());

	// parse json
	struct TestStruct2 ts2;
	Serialization::Deserializer<TestStruct2> d2(ts2);
	std::istringstream sss;
	sss.str((const char*)json);
	Serialization::JSONParser p2(sss);
	p2.parse(d2);

	// build xml again and check
	x.str("");
	Serialization::Serializer<TestStruct2> s3(ts2);
	Serialization::XMLWriter w3(x);
	s3.write(w3);
	if(x.str() != xml)
	{
		std::cout << "EXP: " << xml << std::endl << "GOT: " << x.str() << std::endl;
		return false;
	}
	return true;
}

struct TestStructWithList
{
	CString name;
	std::list<TestStruct1> list;
	static const char* xml_element_name() { return "tswl"; }
	template<class Helper>void serialization(Helper& sr)
	{
		sr.field(name, "@name"); // attrs should go first
		sr.table(list, "list");
	}
};

bool test5() // struct with list
{
	const char * xml = // "<?xml version=\"1.0\" ?>\n"
		"<tswl name=\"somename\">\n"
		"\t<list>\n"
		"\t\t<struct attr=\"first\">\n"
		"\t\t\t<stringfield>Hi, here&amp;there</stringfield>\n"
		"\t\t\t<longfield>123454321</longfield>\n"
		"\t\t\t<boolfield>true</boolfield>\n"
		"\t\t</struct>\n"
		"\t\t<struct attr=\"second\">\n"
		"\t\t\t<stringfield>Oh, shit!</stringfield>\n"
		"\t\t\t<longfield>666</longfield>\n"
		"\t\t\t<boolfield>false</boolfield>\n"
		"\t\t</struct>\n"
		"\t\t<struct attr=\"third\">\n"
		"\t\t\t<stringfield>OMFG</stringfield>\n"
		"\t\t\t<longfield>1998</longfield>\n"
		"\t\t\t<boolfield>true</boolfield>\n"
		"\t\t</struct>\n"
		"\t</list>\n"
		"</tswl>\n";

	struct TestStructWithList ts;
	Serialization::Deserializer<TestStructWithList> d1(ts);
	Serialization::XMLParser p1(d1);
	p1.parse(xml);

	if(ts.name != _T("somename"))
		return false;
	if(ts.list.size() != 3)
		return false;
	if(ts.list.begin()->attr != _T("first"))
		return false;
	if(ts.list.rbegin()->str != _T("OMFG"))
		return false;
	if(ts.list.rbegin()->lng != 1998)
		return false;

	std::ostringstream jsonw;
	Serialization::Serializer<TestStructWithList> s1(ts);
	Serialization::JSONWriter wj(jsonw);
	s1.write(wj);

	CStringA json = jsonw.str().c_str();

	std::istringstream jsonr;
	jsonr.str((const char*)json);
	struct TestStructWithList ts2;
	Serialization::Deserializer<TestStructWithList> d2(ts2);
	Serialization::JSONParser pj(jsonr);
	pj.parse(d2);

	std::ostringstream x;
	Serialization::Serializer<TestStructWithList> s2(ts2);
	Serialization::XMLWriter w1(x);
	s2.write(w1);
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
	if(! test4())
		++rc;
	if(! test5())
		++rc;
	return rc;
}

