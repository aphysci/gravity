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
# 'odbc:@engine=sqlite3;Driver=Sqlite3;database=/tmp/test.db' \
#	'postgresql:dbname=test' \

for STR in \
	'odbc:@engine=mysql;UID=root;PWD=bamas;Database=test;Driver=MySQL'
#	'postgresql:dbname=test;@blob=bytea' \
#	'sqlite3:db=test.db' \
#	'mysql:user=root;password=root;database=test' \
#	'odbc:@engine=postgresql;Database=test;Driver=Postgresql ANSI' \

do
	for SUFFIX in '' ';@use_prepared=off' ';@pool_size=5' ';@use_prepared=off;@pool_size=5'
	do
		run_test ./test_backend "$STR$SUFFIX"
		run_test ./test_basic "$STR$SUFFIX"
	done
done

run_test ./test_caching
