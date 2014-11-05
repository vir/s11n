#ifndef S11NDESERIALIZER_H_INCLUDED
#define S11NDESERIALIZER_H_INCLUDED

#include "S11nCommon.h"
#include <sstream>

namespace Serialization {

	class CPathTokenizer
	{
	private:
		char* m_buf;
		char* m_pos;
	public:
		CPathTokenizer(const char* str):m_buf(::_strdup(str)) { reset(); }
		CPathTokenizer(const CPathTokenizer& oth):m_buf(::_strdup(oth.m_buf)) { reset(); }
		~CPathTokenizer() { ::free(m_buf); }
		void reset() { m_pos = m_buf; }
		bool eat_token(const char* token, bool from_root = false)
		{
			if(from_root && *m_pos != '/')
				return false;
			if(*m_pos == '/')
				++m_pos;
			if(*m_pos == '*') // json wildcard
			{
				m_pos += 2;
				return true;
			}
			char* p = m_pos;
			while(*token && *p && *token == *p) { ++token; ++p; }
			if(*token)
				return false;
			if(*p && *p != '/')
				return false;
			m_pos = p;
			return true;
		}
		bool empty() const { return !*m_pos; }
		const char* tail(bool strip_slash = false) { return (strip_slash && *m_pos == '/') ? m_pos+1 : m_pos; }
	};

#if 0
	// works for std::vector or std::list, may be some other container(s)
	template<class TableType>
	class CSaxVectorDeserializer: public Neon::XMLResponseBodyConsumer
	{
		typedef typename TableType::value_type ItemType;
	private:
		TableType& m_table;
		ItemType* m_current_item;
		bool m_completion_flag;
	public:
		explicit CSaxVectorDeserializer(TableType& tableref)
			: m_table(tableref)
			, m_current_item(NULL)
			, m_completion_flag(false)
		{
		}
		bool complete() const { return m_completion_flag; }
	private:
		void enshure_element_exists()
		{
			if(m_current_item)
				return;
			m_table.push_back(ItemType());
			m_current_item = &(*m_table.rbegin());
			m_completion_flag = false;
		}
	public:
		virtual void data(const std::string & path, const std::string & value, const char* ns)
		{
			CPathTokenizer p(path.c_str());
			if(! p.eat_token(TableType::xml_element_name()))
				return;
			if(p.empty())
				return;
			enshure_element_exists();
			CSaxRecordDeserializer<ItemType>(*m_current_item).data(p.tail(), value, ns);
		}
		virtual void endelem(const std::string & path, const char* ns)
		{
			CPathTokenizer p(path.c_str());
			if(! p.eat_token(TableType::xml_element_name(), true))
				return;
			if(p.empty())
			{
				m_completion_flag = true;
				return;
			}
			CSaxRecordDeserializer<ItemType> r(*m_current_item);
			r.endelem(p.tail(), ns);
			if(r.complete())
				m_current_item = NULL;
		}
	};
#endif

	template<class RecType>
	class Deserializer: public IDeserializer
	{
	private:
		static bool str2bool(LPCTSTR str)
		{
			static LPCTSTR tr[] = { _T("true"), _T("yes"), _T("t"), _T("on"), _T("1"), NULL };
			LPCTSTR* t = tr;
			while(t && *t)
			{
				if(0 == _tcscmp(*t, str))
					return true;
				++t;
			}
			return false;
		}

		class Helper
		{
		private:
			inline bool check_name(const char * name) { return 0 == strcmp(m_name, name); }
			inline void store(CString& var)           { var = m_value; }
			inline void store(long& var)              { var = _ttol(m_value); }
			inline void store(unsigned long& var)     { var = _ttol(m_value); }
			inline void store(bool& var)              { var = str2bool(m_value); }
		private:
			const char* m_name;
			LPCTSTR m_value;
		public:
			Helper(const char* name, LPCTSTR val):m_name(name),m_value(val) { }
			template<typename T> inline void field(T& var, const char* name) { if(check_name(name)) store(var); }
		};

	private:
		RecType& m_item;
		bool m_completion_flag;
	public:
		explicit Deserializer(RecType& itemref)
			: m_item(itemref)
			, m_completion_flag(false)
		{
		}
		bool complete() const { return m_completion_flag; }
	public:
		virtual void data(const char* path, LPCTSTR value)
		{
			CPathTokenizer p(path);
			if(! p.eat_token(RecType::xml_element_name()))
				return;
			m_item.serialization(Deserializer::Helper(p.tail(true), value));
			m_completion_flag = false;
		}
		virtual void endelem(const char* path)
		{
			CPathTokenizer p(path);
			if(! p.eat_token(RecType::xml_element_name()))
				return;
			if(! p.empty())
				return;
			m_completion_flag = true;
		}
	};


	template<class RecType>
	class Serializer
	{
	private:
		class Helper
		{
			typedef std::basic_ostringstream<TCHAR, std::char_traits<TCHAR> > TOSS;
		public:
			Helper(Serialization::IWriter& writer): writer(writer) { }
			template<typename T> inline void field(const T& var, const char* name) { TOSS ss; ss << var; writer.data(name, ss.str().c_str(), 's'); }
			template<> inline void field<bool>(const bool& var, const char* name) { writer.data(name, var ? _T("true") : _T("false"), 'b'); }
			template<> inline void field<CString>(const CString& var, const char* name) { writer.data(name, var, 's'); }
			template<> inline void field<BSTR>(const BSTR& var, const char* name) { field(CString(var), name); }
			template<> inline void field<long>(const long& var, const char* name) { TOSS ss; ss << var; writer.data(name, ss.str().c_str(), 'n'); }
			template<> inline void field<unsigned long>(const unsigned long& var, const char* name) { TOSS ss; ss << var; writer.data(name, ss.str().c_str(), 'n'); }
		private:
			Serialization::IWriter& writer;
		};

	private:
		RecType& m_item;
	public:
		explicit Serializer(RecType& itemref)
			: m_item(itemref)
		{
		}
		void write(Serialization::IWriter& writer)
		{
			writer.startelem(RecType::xml_element_name());
			m_item.serialization(Serializer::Helper(writer));
			writer.endelem(RecType::xml_element_name());
		}
	public:
	};

} // namespace Serialization

#endif /* S11NDESERIALIZER_H_INCLUDED */
