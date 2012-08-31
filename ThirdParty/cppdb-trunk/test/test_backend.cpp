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
#include <cppdb/driver_manager.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <stdlib.h>

#include "test.h"


bool test_64bit_integer = true;
bool test_blob = true;
bool wide_api = false;
bool pq_oid = false;

int sizes[]={	0, // General 0 length string
		61,62,63,64,65,66,67, // mysql buffer size
		510,511,512,513,514, // ODBC buffer size in wchars
		1020,1021,1022,1023,1024,1025,1026,1027, // ODBC buffer size
		4094,4095,4096,4097,4098// postgresql driver buffer size for LO
	    };



/*
void test_template(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
}
*/

void test1(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	try {
		stmt = sql->prepare("drop table test");
		stmt->exec(); 
	}catch(...) {}

	if(sql->engine()=="mssql" && wide_api) {
		stmt = sql->prepare("create table test ( x integer not null, y nvarchar(1000) )");
	}
	else
		stmt = sql->prepare("create table test ( x integer not null, y varchar(1000) )");
	
	std::cout << "Basic select " << std::endl;

	stmt->exec();
	stmt = sql->prepare("select * from test");
	res = stmt->query();
	TEST(!res->next());
	stmt  = sql->prepare("insert into test(x,y) values(10,'foo?')");
	stmt->exec();
	stmt = sql->prepare("select x,y from test");

	res = stmt->query();
	TEST(res->next());
	TEST(res->cols()==2);
	int iv;
	std::string sv;
	TEST(res->fetch(0,iv));
	TEST(iv==10);
	TEST(res->fetch(1,sv));
	TEST(sv=="foo?");
	TEST(!res->next());
	res.reset();
	stmt = sql->prepare("insert into test(x,y) values(20,NULL)");
	stmt->exec();
	stmt = sql->prepare("select y from test where x=?");
	stmt->bind(1,20);
	res = stmt->query();
	TEST(res->next());
	TEST(res->is_null(0));
	sv="xxx";
	TEST(!res->fetch(0,sv));
	TEST(sv=="xxx");
	TEST(!res->next());
	res.reset();
	stmt->reset();
	stmt->bind(1,10);
	res = stmt->query();
	TEST(res->next());
	sv="";
	TEST(!res->is_null(0));
	TEST(res->fetch(0,sv));
	TEST(sv=="foo?");
	stmt = sql->prepare("DELETE FROM test");
	stmt->exec();
	std::cout << "Unicode Test" << std::endl;
	if(sql->engine()!="mssql" || wide_api) {
		std::string test_string = "Pease שלום Мир ﺱﻼﻣ";
		stmt = sql->prepare("insert into test(x,y) values(?,?)");
		stmt->bind(1,15);
		stmt->bind(2,test_string);
		stmt->exec();
		stmt = sql->prepare("select x,y from test");
		res = stmt->query();
		TEST(res->next());
		sv="";
		res->fetch(1,sv);
		TEST(sv==test_string);
	}
	else {
		std::cout << "This does not support unicode, skipping" << std::endl;
	}
}

void test2(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;

	try {
		stmt = sql->prepare("drop table test");
		stmt->exec();
		stmt.reset();
	}catch(...) {}
	stmt = 0;

	std::cout << "Testing sequences types" << std::endl;
	if(sql->engine() == "sqlite3") {
		stmt = sql->prepare("create table test ( id integer primary key autoincrement not null, n integer)");
	}
	else if(sql->engine() == "mysql") {
		stmt = sql->prepare("create table test ( id integer primary key auto_increment not null, n integer)");
	}
	else if(sql->engine() == "postgresql" )  {
		stmt = sql->prepare("create table test ( id  serial  primary key not null, n integer)");
	}
	else if(sql->engine() == "mssql" )  {
		stmt = sql->prepare("create table test ( id integer identity(1,1) primary key not null,n integer)");
	}
	
	if(stmt) {
		stmt->exec();
		stmt = sql->prepare("insert into test(n) values(?)");
		stmt->bind(1,10);
		stmt->exec();
		TEST(stmt->sequence_last("test_id_seq") == 1);
		stmt->reset();
		stmt->bind(1,20);
		stmt->exec();
		TEST(stmt->sequence_last("test_id_seq") == 2);
		stmt = sql->prepare("drop table test");
		stmt->exec();
		stmt.reset();
	}
	else {
		std::cout << "This engine " << sql->engine() << " does not support sequences, skipping" << std::endl;
	}
}


void test3(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	
	std::cout << "Testing data types" << std::endl;
	try {
		stmt = sql->prepare("drop table test");
		stmt->exec();
		stmt.reset();
	}catch(...) {}

	if(sql->engine() == "mssql") 
		stmt = sql->prepare("create table test ( i bigint, r real, t datetime, s varchar(5000), bl varbinary(max))");
	else if(sql->engine() == "mysql") 
		stmt = sql->prepare("create table test ( i bigint, r real, t datetime default null, s varchar(5000), bl blob) Engine = innodb");
	else if(sql->engine() == "postgresql") {
		if(pq_oid) {
			stmt = sql->prepare("create table test ( i bigint, r real, t timestamp, s varchar(5000), bl oid)");
			stmt->exec();
			stmt = sql->prepare(	"CREATE TRIGGER t_test BEFORE UPDATE OR DELETE ON test "
						"FOR EACH ROW EXECUTE PROCEDURE lo_manage(bl)");
		}
		else {
			stmt = sql->prepare("create table test ( i bigint, r real, t timestamp, s varchar(5000), bl bytea)");
		}
	}
	else if(sql->engine() == "sqlite3") {
		stmt = sql->prepare("create table test ( i integer, r real, t timestamp, s varchar(5000), bl blob)");
	}
	else {
		test_64bit_integer = false;
		test_blob= false;
		stmt = sql->prepare("create table test ( i integer, r real, t timestamp, s varchar(5000))");
	}
	stmt->exec();
	if(sql->engine()=="mssql")
		stmt = sql->prepare("insert into test values(?,?,?,?,cast(? as varbinary(max)))");
	else if(test_blob)
		stmt = sql->prepare("insert into test values(?,?,?,?,?)");
	else 
		stmt = sql->prepare("insert into test values(?,?,?,?)");
	stmt->bind_null(1);
	stmt->bind_null(2);
	stmt->bind_null(3);
	stmt->bind_null(4);
	if(test_blob)
		stmt->bind_null(5);
	stmt->exec();
	TEST(stmt->affected()==1);
	if(pq_oid) {
		sql->begin();
	}
	stmt->reset();
	stmt->bind(1,10);
	stmt->bind(2,3.14);
	std::time_t now=time(0);
	std::tm t=*std::localtime(&now);
	stmt->bind(3,t);
	stmt->bind(4,"'to be' \\'or not' to be");
	std::istringstream iss;
	iss.str(std::string("\xFF\0\xFE\1\2",5));
	if(test_blob)
		stmt->bind(5,iss);
	stmt->exec();
	if(pq_oid) {
		sql->commit();
		sql->begin();
	}
	if(test_blob)
		stmt = sql->prepare("select i,r,t,s,bl from test");
	else
		stmt = sql->prepare("select i,r,t,s from test");
	res = stmt->query();
	{
		TEST(res->cols()==test_blob ? 5 : 4);
		TEST(res->column_to_name(0)=="i");
		TEST(res->column_to_name(1)=="r");
		TEST(res->column_to_name(2)=="t");
		TEST(res->column_to_name(3)=="s");
		if(test_blob)
			TEST(res->column_to_name(4)=="bl");

		TEST(res->name_to_column("i")==0);
		TEST(res->name_to_column("r")==1);
		TEST(res->name_to_column("t")==2);
		TEST(res->name_to_column("s")==3);
		if(test_blob)
			TEST(res->name_to_column("bl")==4);
		TEST(res->name_to_column("x")==-1);
		
		TEST(res->next());

		std::ostringstream oss;
		int i=-1; double r=-1; std::tm t=std::tm(); std::string s="def";
		TEST(res->is_null(0));
		TEST(res->is_null(1));
		TEST(res->is_null(2));
		TEST(res->is_null(3));
		if(test_blob)
			TEST(res->is_null(4));
		TEST(!res->fetch(0,i));
		TEST(!res->fetch(1,r));
		TEST(!res->fetch(2,t));
		TEST(!res->fetch(3,s));
		if(test_blob)
			TEST(!res->fetch(4,oss));
		TEST(i==-1);
		TEST(r==-1);
		TEST(t.tm_year == 0);
		TEST(s=="def");
		TEST(oss.str()=="");
		TEST(res->has_next() == cppdb::backend::result::next_row_unknown || res->has_next() == cppdb::backend::result::next_row_exists);
		TEST(res->next());
		TEST(res->fetch(0,i));
		TEST(res->fetch(1,r));
		TEST(res->fetch(2,t));
		TEST(res->fetch(3,s));
		if(test_blob)
			TEST(res->fetch(4,oss));
		TEST(!res->is_null(0));
		TEST(!res->is_null(1));
		TEST(!res->is_null(2));
		TEST(!res->is_null(3));
		if(test_blob)
			TEST(!res->is_null(4));
		TEST(i==10);
		TEST(3.1399 <= r && r <= 3.1401);
		TEST(mktime(&t)==now);
		TEST(s=="'to be' \\'or not' to be");
		if(test_blob)
			TEST(oss.str() == std::string("\xFF\0\xFE\1\2",5));
		TEST(res->has_next() == cppdb::backend::result::next_row_unknown || res->has_next() == cppdb::backend::result::last_row_reached);
		TEST(!res->next());
	}
	if(pq_oid) {
		sql->commit();
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	TEST(stmt->affected()==2);
}

void test4(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;

	std::cout << "Tesing transactions" << std::endl;
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	sql->begin();
	stmt = sql->prepare("insert into test(i) values(10)");
	stmt->exec();
	stmt = sql->prepare("insert into test(i) values(20)");
	stmt->exec();
	sql->commit();
	stmt = sql->prepare("select count(*) from test");
	res = stmt->query();
	TEST(res->next());
	int iv;
	TEST(res->fetch(0,iv) && iv==2);
	res.reset();
	stmt.reset();
	iv=-1;
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	sql->begin();
	stmt = sql->prepare("insert into test(i) values(10)");
	stmt->exec();
	stmt = sql->prepare("insert into test(i) values(20)");
	stmt->exec();
	sql->rollback();
	stmt = sql->prepare("select count(*) from test");
	res = stmt->query();
	TEST(res->next());
	iv=-1;
	TEST(res->fetch(0,iv));
	TEST(iv==0);
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
}

void test5(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;

	std::cout << "Tesing variable length data handing: text" << std::endl;
	sql->begin();
	stmt = sql->prepare("insert into test(i,s) values(?,?)");
	for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
		int size = sizes[i];
		std::string value;
		value.reserve(size);
		srand(i);
		for(int j=0;j<size;j++) {
			value+=char(rand() % 26 + 'a');
		}
		stmt->bind(1,size);
		stmt->bind(2,value);
		stmt->exec();
		stmt->reset();
	}
	sql->commit();
	stmt = sql->prepare("select s from test where i=?");
	for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
		int size = sizes[i];
		std::string value;
		value.reserve(size);
		srand(i);
		for(int j=0;j<size;j++) {
			value+=char(rand() % 26 + 'a');
		}
		stmt->bind(1,size);
		res = stmt->query();
		TEST(res->next());
		std::string v;
		TEST(res->fetch(0,v));
		TEST(v==value);
		res.reset();
		stmt->reset();
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
}

void test6(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;

	if(test_blob) {	
		std::cout << "Tesing variable length data handing: blob" << std::endl;

		sql->begin();
		if(sql->engine()=="mssql")
			stmt = sql->prepare("insert into test(i,bl) values(?,cast(? as varbinary(max)))");
		else
			stmt = sql->prepare("insert into test(i,bl) values(?,?)");
		for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
			int size = sizes[i];
			std::stringstream value;
			srand(i);
			for(int j=0;j<size;j++) {
				value << char(rand() % 26 + 'a');
			}
			stmt->bind(1,size);
			stmt->bind(2,value);
			stmt->exec();
			stmt->reset();
		}
		sql->commit();

		stmt = sql->prepare("select bl from test where i=?");
		for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
			int size = sizes[i];
			std::string value;
			value.reserve(size);
			srand(i);
			for(int j=0;j<size;j++) {
				value+=char(rand() % 26 + 'a');
			}
			if(pq_oid)
				sql->begin();
			stmt->bind(1,size);
			res = stmt->query();
			TEST(res->next());
			std::ostringstream v;
			TEST(res->fetch(0,v));
			TEST(v.str()==value);
			res.reset();
			stmt->reset();
			if(pq_oid)
				sql->commit();
		}
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();

}


void test7(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	std::cout << "Testing integer ranges" << std::endl;
	
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- short" << std::endl;
		stmt=sql->prepare("insert into test(i) values(32768)");
		stmt->exec();
		stmt=sql->prepare("select i from test");
		res = stmt->query();
		res->next();
		short s=0;
		THROWS(res->fetch(0,s),cppdb::bad_value_cast);
		unsigned short us=0;
		TEST(res->fetch(0,us));
		TEST(us==32768);
	}
	if(test_64bit_integer) {
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- int" << std::endl;
		stmt=sql->prepare("insert into test(i) values(2147483648)");
		stmt->exec();
		stmt=sql->prepare("select i from test");
		res = stmt->query();
		res->next();
		int i=0;
		THROWS(res->fetch(0,i),cppdb::bad_value_cast);
		unsigned ui=0;
		TEST(res->fetch(0,ui));
		TEST(ui==2147483648U);
		if(sizeof(long)==8) {
			long l=0;
			TEST(res->fetch(0,l));
			TEST(l==2147483648L);
		}
		long long ll=0;
		TEST(res->fetch(0,ll));
		TEST(ll==2147483648LL);
	}
	if(test_64bit_integer && sizeof(long)==4) {
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- long" << std::endl;
		stmt=sql->prepare("insert into test(i) values(2147483648)");
		stmt->exec();
		stmt=sql->prepare("select i from test");
		res = stmt->query();
		res->next();
		long i=0;
		THROWS(res->fetch(0,i),cppdb::bad_value_cast);
		unsigned long ul=0;
		TEST(res->fetch(0,ul));
		TEST(ul==2147483648UL);
		long long ll=0;
		TEST(res->fetch(0,ll));
		TEST(ll==2147483648LL);
	}
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- unsigned short,int,long,long long" << std::endl;
		stmt=sql->prepare("insert into test(i) values(-1)");
		stmt->exec();
		stmt=sql->prepare("select i from test");
		res = stmt->query();
		res->next();
		unsigned short us=0;
		short s=0;
		unsigned ui=0;
		int i=0;
		unsigned long ul=0;
		long l=0;
		unsigned long long ull=0;
		long long ll=0;
		THROWS(res->fetch(0,us),cppdb::bad_value_cast);
		THROWS(res->fetch(0,ui),cppdb::bad_value_cast);
		THROWS(res->fetch(0,ul),cppdb::bad_value_cast);
		THROWS(res->fetch(0,ull),cppdb::bad_value_cast);
		TEST(res->fetch(0,s) && s==-1);
		TEST(res->fetch(0,i) && i==-1);
		TEST(res->fetch(0,l) && l==-1);
		TEST(res->fetch(0,ll) && ll==-1);
	}
	std::cout <<"Testing floating -> integer conversions" << std::endl;
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		stmt=sql->prepare("insert into test(r) values(33000.11)");
		stmt->exec();
		stmt=sql->prepare("select r from test");
		res = stmt->query();
		res->next();
		int i=0;
		short s=0;
		TEST(res->fetch(0,i) && (i==33000 || i==33001));
		THROWS(res->fetch(0,s),cppdb::bad_value_cast);
		res.reset();
		stmt=sql->prepare("delete from test");
		stmt->exec();
		stmt=sql->prepare("insert into test(r) values(-1e12)");
		stmt->exec();
		long long ll = 0;
		unsigned long long ull;
		stmt=sql->prepare("select r from test");
		res = stmt->query();
		res->next();
		THROWS(res->fetch(0,i),cppdb::bad_value_cast);
		TEST(res->fetch(0,ll) && ll==-1000000000000LL);
		THROWS(res->fetch(0,ull),cppdb::bad_value_cast);
	}
	stmt=sql->prepare("delete from test");
	stmt->exec();
}

void test8(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	std::cout << "Testing conversions on fetch" << std::endl;
	
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- string to integer" << std::endl;
		stmt=sql->prepare("insert into test(s) values('10')");
		stmt->exec();
		stmt=sql->prepare("select s from test");
		res = stmt->query();
		res->next();
		int i=0;
		TEST(res->fetch(0,i));
		TEST(i==10);
	}
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- string to date-time" << std::endl;
		stmt=sql->prepare("insert into test(s) values('2008-02-03 11:22:33')");
		stmt->exec();
		stmt=sql->prepare("select s from test");
		res = stmt->query();
		res->next();
		std::tm t=std::tm();;
		TEST(res->fetch(0,t));
		TEST(t.tm_year == 2008 - 1900);
		TEST(t.tm_mon == 2 - 1);
		TEST(t.tm_mday == 3);
		TEST(t.tm_hour == 11);
		TEST(t.tm_min == 22);
		TEST(t.tm_sec == 33);
	}
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- integer to string" << std::endl;
		stmt=sql->prepare("insert into test(i) values(10)");
		stmt->exec();
		stmt=sql->prepare("select i from test");
		res = stmt->query();
		res->next();
		int i=0;
		TEST(res->fetch(0,i));
		TEST(i==10);
	}

}

void test9(cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	std::cout << "Testing conversions on bind" << std::endl;
	
	{
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- integer to string" << std::endl;
		stmt=sql->prepare("insert into test(s) values(?)");
		stmt->bind(1,10);
		stmt->exec();
		stmt=sql->prepare("select s from test");
		res = stmt->query();
		res->next();
		std::string s;
		TEST(res->fetch(0,s));
		TEST(s=="10");
	}
	if(sql->engine()!="mssql") {
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::tm t=std::tm();;
		t.tm_year = 2008 - 1900;
		t.tm_mon = 2 - 1;
		t.tm_mday = 3;
		t.tm_hour = 11;
		t.tm_min = 22;
		t.tm_sec = 33;
		mktime(&t);

		std::cout << "- date-time to string" << std::endl;
		stmt=sql->prepare("insert into test(s) values(?)");
		stmt->bind(1,t);
		stmt->exec();
		stmt=sql->prepare("select s from test");
		res = stmt->query();
		res->next();
		std::string s;
		TEST(res->fetch(0,s));
		TEST(s=="2008-02-03 11:22:33");
	}
	if(sql->engine()!="mssql"){
		stmt=sql->prepare("delete from test");
		stmt->exec();
		std::cout << "- string to date-time" << std::endl;
		stmt=sql->prepare("insert into test(t) values(?)");
		stmt->bind(1,"2008-02-03 11:22:33");
		stmt->exec();
		stmt=sql->prepare("select t from test");
		res = stmt->query();
		res->next();
		std::tm t=std::tm();;
		TEST(res->fetch(0,t));
		TEST(t.tm_year == 2008 - 1900);
		TEST(t.tm_mon == 2 - 1);
		TEST(t.tm_mday == 3);
		TEST(t.tm_hour == 11);
		TEST(t.tm_min == 22);
		TEST(t.tm_sec == 33);
	}

}


void run_test(void (*func)(cppdb::ref_ptr<cppdb::backend::connection>),cppdb::ref_ptr<cppdb::backend::connection> sql)
{
	try {
		func(sql);
	}
	catch(cppdb::cppdb_error const &e) {
		std::cerr << "Catched exception " << e.what() << std::endl;
		std::cerr << "Last tested line " << last_line  << std::endl;
		failed++;
		return;
	}
	passed++;
}

void test(std::string conn_str)
{
	cppdb::ref_ptr<cppdb::backend::connection> sql(cppdb::driver_manager::instance().connect(conn_str));
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	wide_api = (sql->driver()=="odbc" && conn_str.find("utf=wide")!=std::string::npos);
	
	std::cout << "Basic setup" << std::endl;

	if(sql->engine() == "postgresql") {
		if(sql->driver() == "odbc")
			pq_oid = true;
		else if(sql->driver()=="postgresql" && conn_str.find("@blob=bytea") == std::string::npos)
			pq_oid = true;
	}

	run_test(test1,sql);
	run_test(test2,sql);
	run_test(test3,sql);
	run_test(test4,sql);
	run_test(test5,sql);
	run_test(test6,sql);
	run_test(test7,sql);
	run_test(test8,sql);
	run_test(test9,sql);
}



int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "Usage: test_backend connection_string" << std::endl;
		return 1;
	}
	std::string cs = argv[1];
	try {
		test(cs);
	}
	CATCH_BLOCK();
	SUMMARY();
}
