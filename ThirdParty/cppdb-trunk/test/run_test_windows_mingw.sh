#!/bin/bash

run_test()
{
	if $1 "$2" &>report.txt
	then
		echo "Passed: $1 $2"
	else
		echo "Failed: $1 $2"
		cat report.txt >>fail.txt
	fi
	cat report.txt >>all.txt
}


rm -f all.txt fail.txt 

for STR in \
	'sqlite3:db=test.db' \
	'odbc:Driver=SQL Server Native Client 10.0;Databse=test;Server=localhost;UID=root;PWD=rootroot;@engine=mssql' \
	'odbc:Driver=SQL Server Native Client 10.0;Databse=test;Server=localhost;UID=root;PWD=rootroot;@engine=mssql;@utf=wide' \

do
	for SUFFIX in '' ';@use_prepared=off' ';@pool_size=5' ';@use_prepared=off;@pool_size=5'
	do
		run_test ./test_backend "$STR$SUFFIX"
		run_test ./test_basic "$STR$SUFFIX"
	done
done

run_test ./test_caching


