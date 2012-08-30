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
#include <cppdb/driver_manager.h>
#include <cppdb/conn_manager.h>
#include "test.h"
#include "dummy_driver.h" 

#if ( defined(WIN32) || defined(_WIN32) || defined(__WIN32) ) && !defined(__CYGWIN__)
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <windows.h>
void sleep(int x)
{
	Sleep(1000*x);
}
#else
# include <stdlib.h>
# include <unistd.h>
#endif

void test_driver_manager()
{
	cppdb::ref_ptr<cppdb::backend::connection> c1,c2,c3,c4;
	cppdb::driver_manager &dm = cppdb::driver_manager::instance();
	std::cout << "Testing drivers collection" << std::endl;
	cppdb::connections_manager &cm = cppdb::connections_manager::instance();
	dm.install_driver("dummy",new dummy::loadable_driver());
	TEST(dummy::drivers==1);
	dm.collect_unused();
	TEST(dummy::drivers==0);
	dm.install_driver("dummy",new dummy::loadable_driver());
	c1=dm.connect("dummy:");
	TEST(dummy::connections==1);
	dm.collect_unused();
	TEST(dummy::drivers==1);
	TEST(dummy::connections==1);
	c2=dm.connect("dummy:");
	dm.collect_unused();
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	c1=0;
	TEST(dummy::connections==1);
	c2=0;
	TEST(dummy::connections==0);
	TEST(dummy::drivers==1);
	dm.collect_unused();
	TEST(dummy::drivers==0);
	THROWS(c1=dm.connect("dummy:"),cppdb::cppdb_error);
	std::cout << "Testing connection pooling" << std::endl;
	dm.install_driver("dummy",new dummy::loadable_driver());
	c1=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==1);
	c2=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==2);
	c3=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==3);
	c1.reset();
	TEST(dummy::connections==3);
	c2.reset();
	TEST(dummy::connections==3);
	c3.reset();
	TEST(dummy::connections==2);
	c3=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	sleep(3);
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==1);
	TEST(dummy::drivers==1);
	c3.reset();
	TEST(dummy::connections==1);
	TEST(dummy::drivers==1);
	sleep(3);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==0);
	TEST(dummy::drivers==0);
}

void test_stmt_cache()
{
	cppdb::ref_ptr<cppdb::backend::connection> c;
	cppdb::ref_ptr<cppdb::backend::statement> s1,s2,s3;

	cppdb::driver_manager &dm = cppdb::driver_manager::instance();
	dm.install_driver("dummy",new dummy::loadable_driver());
	c=dm.connect("dummy:@use_prepared=off");
	s1=c->prepare("test1");
	s2=c->prepare("test2");
	TEST(dummy::statements==2);
	s1.reset();
	TEST(dummy::statements==1);
	s2.reset();
	TEST(dummy::statements==0);
	c=dm.connect("dummy:@use_prepared=on;@stmt_cache_size=3");
	s1=c->prepare("test1");
	s1=c->prepare("test2");
	s1=c->prepare("test3");
	TEST(dummy::statements==3);
	s1=c->prepare("test4");
	TEST(dummy::statements==4);
	s1=c->prepare("test5");
	TEST(dummy::statements==4);
	s1.reset();
	s1=c->prepare("test3");
	TEST(dummy::statements==3);
	s1.reset();
	TEST(dummy::statements==3);
	c->clear_cache();
	TEST(dummy::statements==0);
	s1=c->prepare("test");
	s1.reset();
	s1=c->prepare("test");
	TEST(dummy::statements==1);
	s1=c->prepare("test1");
	TEST(dummy::statements==2);
	s2=c->prepare("test1");
	TEST(dummy::statements==3);
	s1.reset();
	s2.reset();
	TEST(dummy::statements==2);

}

int main()
{
	try {
		test_driver_manager();
	}
	CATCH_BLOCK()
	try {
		test_stmt_cache();
	}
	CATCH_BLOCK()
	SUMMARY();

}
