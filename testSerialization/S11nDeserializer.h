#ifndef S11NDESERIALIZER_H_INCLUDED
#define S11NDESERIALIZER_H_INCLUDED

#include "S11nCommon.h"

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

	static bool str2bool(const char* str)
	{
		static const char* tr[] = { "yes", "true", "t", "on", "1", NULL };
		const char** t = tr;
		while(t && *t)
		{
			if(0 == strcmp(*t, str))
				return true;
			++t;
		}
		return false;
	}

	static CString FromUtf8(const char * s)
	{
		return CString(CStringA(s));
	}

	static CStringA ToUtf8(CString s)
	{
		return CStringA(CString(s));
	}

	template<class RecType>
	class Deserializer: public IDeserializer
	{
	private:
		class Helper
		{
		private:
			inline bool check_name(const char * name) { return 0 == strcmp(m_name, name); }
			inline void store(CString& var) { var = FromUtf8(m_value); }
			inline void store(long& var)    { var = atol(m_value); }
			inline void store(unsigned long& var) { var = atol(m_value); }
			inline void store(bool& var)    { var = str2bool(m_value); }
		private:
			const char* m_name;
			const char* m_value;
		public:
			Helper(const char* name, const char* val):m_name(name),m_value(val) { }
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
		virtual void data(const char* path, const char* value)
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
		public:
			Helper(Serialization::IWriter& writer): writer(writer) { }
			template<typename T> inline void field(const T& var, const char* name) { std::stringstream ss; ss << var; writer.data(name, ss.str().c_str(), 's'); }
			template<> inline void field<bool>(const bool& var, const char* name) { writer.data(name, var ? "true" : "false", 'b'); }
			template<> inline void field<CString>(const CString& var, const char* name) { writer.data(name, ToUtf8(var), 's'); }
		private:
			Serialization::IWriter& writer;
		};

	private:
		RecType& m_item;
		bool m_completion_flag;
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
