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
#include <cppdb/frontend.h>
#include <iostream>
#include <ctime>


int main()
{
	try {
		cppdb::session sql("sqlite3:db=db.db");
		
		sql << "DROP TABLE IF EXISTS test" << cppdb::exec;

		sql<<	"CREATE TABLE test ( "
			"   id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
			"   n    INTEGER,"
			"   f    REAL, "
			"   t    TIMESTAMP,"
			"   name TEXT "
			")  " << cppdb::exec;
		
		std::time_t now_time = std::time(0);
		
		std::tm now = *std::localtime(&now_time);

		cppdb::statement stat;
		
		stat = sql << 
			"INSERT INTO test(n,f,t,name) "
			"VALUES(?,?,?,?)"
			<< 10 << 3.1415926565 << now <<"Hello 'World'";

		stat.exec();
		std::cout<<"ID: "<<stat.last_insert_id() << std::endl;
		std::cout<<"Affected rows "<<stat.affected()<<std::endl;
		
		stat.reset();

		stat.bind(20);
		stat.bind_null();
		stat.bind(now);
		stat.bind("Hello 'World'");
		stat.exec();


		cppdb::result res = sql << "SELECT id,n,f,t,name FROM test";

		while(res.next()) {
			double f=-1;
			int id,k;
			std::tm atime;
			std::string name;
			res >> id >> k >> f >> atime >> name;
			std::cout <<id << ' '<<k <<' '<<f<<' '<<name<<' '<<asctime(&atime)<< std::endl;
		}

		res = sql << "SELECT n,f FROM test WHERE id=?" << 1 << cppdb::row;
		if(!res.empty()) {
			int n = res.get<int>("n");
			double f=res.get<double>(1);
			std::cout << "The values are " << n <<" " << f << std::endl;
		}
	}
	catch(std::exception const &e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
