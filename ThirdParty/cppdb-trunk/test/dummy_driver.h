///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2010-2011  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  Distributed under:
//
//                   the Boost Software License, Version 1.0.
//              (See accompanying file LICENSE_1_0.txt or copy at 
//                     http://www.boost.org/LICENSE_1_0.txt)
//
//  or (at your opinion) under:
//
//                               The MIT License
//                 (See accompanying file MIT.txt or a copy at
//              http://www.opensource.org/licenses/mit-license.php)
//
///////////////////////////////////////////////////////////////////////////////
#include <cppdb/backend.h>

namespace dummy {

	int results = 0;
	int statements = 0;
	int connections = 0;
	int drivers = 0;

	class result : public cppdb::backend::result {
	public:
		virtual next_row has_next()
		{
			return next_row_unknown;
		}
		virtual bool next() 
		{
			if(called_)
				return false;
			called_ = true;
			return true;
		}
		virtual bool fetch(int,short &)
		{
			return false;
		}
		virtual bool fetch(int,unsigned short &){ return false; }
		virtual bool fetch(int,int &){ return false; }
		virtual bool fetch(int,unsigned &){ return false; }
		virtual bool fetch(int,long &){ return false; }
		virtual bool fetch(int,unsigned long &){ return false; }
		virtual bool fetch(int,long long &){ return false; }
		virtual bool fetch(int,unsigned long long &){ return false; }
		virtual bool fetch(int,float &){ return false; }
		virtual bool fetch(int,double &){ return false; }
		virtual bool fetch(int,long double &){ return false; }
		virtual bool fetch(int,std::string &){ return false; }
		virtual bool fetch(int,std::ostream &){ return false; }
		virtual bool fetch(int,std::tm &){ return false; }
		virtual bool is_null(int){ return true; }
		virtual int cols() { return 10; }
		virtual int name_to_column(std::string const &) {
			return -1;
		}
		virtual std::string column_to_name(int) 
		{
			throw cppdb::not_supported_by_backend("unsupported");
		}

		// End of API
		result() : called_(false) 
		{
			results++;
		}
		~result()
		{
			results--;
		}
		
	private:
		bool called_;
	};

	class statement : public cppdb::backend::statement {
	public:
		virtual void reset(){}
		std::string const &sql_query() { return q_; }
		virtual void bind(int,std::string const &) {}
		virtual void bind(int,char const *){}
		virtual void bind(int,char const *,char const *){}
		virtual void bind(int,std::tm const &){}
		virtual void bind(int,std::istream &){}
		virtual void bind(int,int){}
		virtual void bind(int,unsigned){}
		virtual void bind(int,long){}
		virtual void bind(int,unsigned long){}
		virtual void bind(int,long long){}
		virtual void bind(int,unsigned long long){}
		virtual void bind(int,double){}
		virtual void bind(int,long double){}
		virtual void bind_null(int){}
		virtual long long sequence_last(std::string const &/*sequence*/) { throw cppdb::not_supported_by_backend("unsupported"); }
		virtual unsigned long long affected() { return 0; }
		virtual result *query() { return new result(); }
		virtual void exec() {}
		statement(std::string const &q) : q_(q) 
		{
			statements++;
		}
		~statement()
		{
			statements--;
		}
	private:
		std::string q_;
	};



	extern "C" {
		typedef cppdb::backend::connection *cppdb_backend_connect_function(cppdb::connection_info const &ci);
	}


	class connection : public cppdb::backend::connection {
	public:
		connection(cppdb::connection_info const &info) : cppdb::backend::connection(info) 
		{
			connections++;
		}
		~connection()
		{
			connections--;
		}
		virtual void begin(){}
		virtual void commit(){}
		virtual void rollback(){}
		virtual statement *prepare_statement(std::string const &q) { return new statement(q); }
		virtual statement *create_statement(std::string const &q) { return new statement(q); }
		virtual std::string escape(std::string const &) { throw cppdb::not_supported_by_backend("not supported"); }
		virtual std::string escape(char const *) { throw cppdb::not_supported_by_backend("not supported"); }
		virtual std::string escape(char const *,char const *) { throw cppdb::not_supported_by_backend("not supported"); }
		virtual std::string driver() { return "dummy"; }
		virtual std::string engine() { return "dummy"; }
		
	};


	class loadable_driver : public cppdb::backend::loadable_driver {
	public:
		loadable_driver() 
		{
			drivers++;
		}
		bool in_use()
		{
			return connections > 0;
		}
		connection *open(cppdb::connection_info const &cs)
		{
			return new connection(cs);
		}
		~loadable_driver()
		{
			drivers--;
		}
	};

} // dummy

extern "C" {
	cppdb::backend::connection *dummy_connect_function(cppdb::connection_info const &ci)
	{
		return new dummy::connection(ci);
	}
}
