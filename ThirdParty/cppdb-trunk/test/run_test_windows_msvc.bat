@echo off
SET CS=sqlite3:db=test.db

echo %CS%
echo "----------BASIC-----------"
test_basic "%CS%"
echo "----------BACKEND-----------"
test_backend "%CS%"
echo "%CS%;@pool_size=5"
echo "----------BASIC-----------"
test_basic "%CS%;@pool_size=5"
echo "----------BACKEND-----------"
test_backend "%CS%;@pool_size=5"
echo "%CS%;@use_prepared=off"
echo "----------BASIC-----------"
test_basic "%CS%;@use_prepared=off"
echo "----------BACKEND-----------"
test_backend "%CS%;@use_prepared=off"


SET CS=odbc:Driver=SQL Server Native Client 10.0;Databse=test;Server=localhost;UID=root;PWD=rootroot;@engine=mssql

echo %CS%
echo "----------BASIC-----------"
test_basic "%CS%"
echo "----------BACKEND-----------"
test_backend "%CS%"
echo "----------BASIC-----------"
test_basic "%CS%;@pool_size=5"
echo "----------BACKEND-----------"
test_backend "%CS%;@pool_size=5"
echo "%CS%;@use_prepared=off"
echo "----------BASIC-----------"
test_basic "%CS%;@use_prepared=off"
echo "----------BACKEND-----------"
test_backend "%CS%;@use_prepared=off"

SET CS=odbc:Driver=SQL Server Native Client 10.0;Databse=test;Server=localhost;UID=root;PWD=rootroot;@engine=mssql;@utf=wide

echo %CS%
echo "----------BASIC-----------"
test_basic "%CS%"
echo "----------BACKEND-----------"
echo "%CS%;@pool_size=5"
echo "----------BASIC-----------"
test_basic "%CS%;@pool_size=5"
echo "----------BACKEND-----------"
test_backend "%CS%;@pool_size=5"
test_backend "%CS%"
echo "%CS%;@use_prepared=off"
echo "----------BASIC-----------"
test_basic "%CS%;@use_prepared=off"
echo "----------BACKEND-----------"
test_backend "%CS%;@use_prepared=off"
