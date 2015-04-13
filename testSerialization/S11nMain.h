#ifndef S11NDESERIALIZER_H_INCLUDED
#define S11NDESERIALIZER_H_INCLUDED

#include <sstream>

namespace Serialization {

	/* some Interfaces to allow plugging different parsers and writers */

	struct IDeserializer
	{
		virtual void data(const char* path, LPCTSTR value) = 0;
		virtual bool endelem(const char* path) = 0;
	};

	struct IWriter
	{
		virtual void startelem(const char* name, char type) = 0;
		virtual void data(const char* name, LPCTSTR val, char format) = 0;
		virtual void endelem(const char* name) = 0;
	};

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
		bool has_slash() const { return NULL != ::strchr(m_pos + 1, '/'); }
	};


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
			Deserializer& m_owner;
		public:
			Helper(Deserializer& owner, const char* name, LPCTSTR val):m_owner(owner), m_name(name),m_value(val) { }
			template<typename T> inline void field(T& var, const char* name) { if(check_name(name)) store(var); }
			template<typename T> void complex(T& sub, const char* name)
			{
				CPathTokenizer p(m_name);
				if(! p.eat_token(name))
					return;
				if(! m_owner.m_proxy)
					m_owner.m_proxy = new Deserializer<T>(sub, name);
				m_owner.m_proxy->data(m_name, m_value);
			}
			template<typename T> void table(T& lst, const char* name)
			{
				CPathTokenizer p(m_name);
				if(! p.eat_token(name))
					return;
				if(! m_owner.m_proxy)
					m_owner.m_proxy = new TableDeserializer<T>(lst, name);
				m_owner.m_proxy->data(m_name, m_value);
			}
		};

	private:
		RecType& m_item;
		const char* m_element_name;
		IDeserializer* m_proxy;
	public:
		explicit Deserializer(RecType& itemref, const char* element_name = RecType::xml_element_name())
			: m_item(itemref)
			, m_element_name(element_name)
			, m_proxy(NULL)
		{
		}
		~Deserializer()
		{
			delete m_proxy;
		}
	public:
		virtual void data(const char* path, LPCTSTR value)
		{
			CPathTokenizer p(path);
			if(! p.eat_token(m_element_name))
				return;
			m_item.serialization(Deserializer::Helper(*this, p.tail(true), value));
		}
		virtual bool endelem(const char* path)
		{
			CPathTokenizer p(path);
			if(! p.eat_token(m_element_name))
				return false;
			if(p.empty())
				return true;
			if(m_proxy && m_proxy->endelem(p.tail()))
			{
				delete m_proxy;
				m_proxy = NULL;
			}
			return false;
		}
	};

	template<class TableType>
	class TableDeserializer: public IDeserializer
	{
		typedef typename TableType::value_type ItemType;
	private:
		TableType& m_table;
		const char* m_element_name;
		ItemType* m_current_item;
	public:
		explicit TableDeserializer(TableType& tableref, const char* element_name/* = TableType::xml_element_name()*/)
			: m_table(tableref)
			, m_element_name(element_name)
			, m_current_item(NULL)
		{
		}
	private:
		void enshure_element_exists()
		{
			if(m_current_item)
				return;
			m_table.push_back(ItemType());
			m_current_item = &(*m_table.rbegin());
		}
	public:
		virtual void data(const char* path, LPCTSTR value)
		{
			CPathTokenizer p(path);
			if(! p.eat_token(m_element_name))
				return;
			if(p.empty())
				return;
			enshure_element_exists();
			Deserializer<ItemType>(*m_current_item).data(p.tail(), value);
		}
		virtual bool endelem(const char* path)
		{
			CPathTokenizer p(path);
			if(! p.eat_token(m_element_name, true))
				return false;
			if(p.empty())
				return true;
			Deserializer<ItemType> r(*m_current_item);
			if(r.endelem(p.tail()))
				m_current_item = NULL;
			return false;
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
			template<typename T> void complex(T& sub, const char* name) { Serializer<T>(sub, name).write2(writer, 'm'); }
			template<typename T> void table(T& sub, const char* name) { TableSerializer<T>(sub, name).write(writer); }
		private:
			Serialization::IWriter& writer;
		};

	private:
		RecType& m_item;
		const char* m_element_name;
	public:
		explicit Serializer(RecType& itemref, const char* element_name = RecType::xml_element_name())
			: m_item(itemref)
			, m_element_name(element_name)
		{
		}
		void write2(Serialization::IWriter& writer, char type)
		{
			writer.startelem(m_element_name, type);
			m_item.serialization(Serializer::Helper(writer));
			writer.endelem(m_element_name);
		}
		inline void write(Serialization::IWriter& writer)
		{
			write2(writer, 's');
		}
	};

	template<class TableType>
	class TableSerializer
	{
		typedef typename TableType::value_type ItemType;
	public:
		explicit TableSerializer(TableType& c, const char* element_name/* = TableType::xml_element_name()*/)
			: m_item(c)
			, m_element_name(element_name)
		{
		}
		void write(Serialization::IWriter& writer)
		{
			writer.startelem(m_element_name, 'a');
			for(TableType::iterator it = m_item.begin(); it != m_item.end(); ++it)
				Serializer<ItemType>(*it).write(writer);
			writer.endelem(m_element_name);
		}
	private:
		TableType& m_item;
		const char* m_element_name;
	};

} // namespace Serialization

#endif /* S11NDESERIALIZER_H_INCLUDED */
