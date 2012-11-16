Rem ember to run this from the Visual Studio 2010 Command Prompt.  
set OLD_CD=%CD%

set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK   
set BIN_DIR=bin2010-64bit
set LIB_DIR=lib2010-64bit
set BUILD_DIR=x64\Release2010
set ZIP_OUT_NAME=gravity-MSVC10.zip

mkdir %BIN_DIR%
mkdir %LIB_DIR%
mkdir include
mkdir include\protobuf

Rem Build ThirdParty
cd ThirdParty

mkdir %BIN_DIR%
mkdir %LIB_DIR%

Rem Build Third party libs (these have the most variation and may require separate configuration).  
cd protobuf-2.4.1\vsprojects
msbuild libprotobuf.vcxproj %CONFIGURATION% || goto build_fail
msbuild protoc.vcxproj %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\libprotobuf.lib ..\..\%LIB_DIR% || goto build_fail
copy %BUILD_DIR%\protoc.exe ..\..\%BIN_DIR% || goto build_fail
cd ..\..
xcopy /s /q /y protobuf-2.4.1\src\*.h include

cd zeromq-3.2.1\builds\msvc
msbuild msvc10.sln /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK  || goto build_fail
copy ..\..\lib\x64\libzmq.lib ..\..\..\%LIB_DIR%\libzmq.lib || goto build_fail
copy ..\..\bin\x64\libzmq.dll ..\..\..\%BIN_DIR%\libzmq.dll || goto build_fail

cd ..\..\..\iniparser\build
msbuild iniparser.sln %CONFIGURATION% || goto build_fail
Rem lib File from this project is built directly into the lib directory

where /q cmake
if not errorlevel 1 (

cd ThirdParty\cppdb-trunk
mkdir buildMSVS10
cd buildMSVS10
cmake -G"Visual Studio 10" ..
msbuild cppdb.vcxproj %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\cppdb.lib ..\..\%LIB_DIR% || goto build_fail
copy %BUILD_DIR%\cppdb.dll ..\..\%BIN_DIR% || goto build_fail
cd ..\..\..

)

cd ..\..\..
copy ThirdParty\pthreads\lib\x64\pthreadVC2.lib ThirdParty\%LIB_DIR% || goto build_fail
copy ThirdParty\pthreads\bin\x64\pthreadVC2.dll ThirdParty\%BIN_DIR% || goto build_fail
copy ThirdParty\pthreads\include\*.h include\ || goto build_fail

REM Files Copied: 
REM protoc.exe
REM libzmq.dll
REM cppdb.dll
REM libprotobuf.lib

REM libzmq.lib
REM iniparser.lib
REM cppdb.lib
REM pthreadVCE2.lib

call BuildGravityVSCommon.bat
if errorlevel 1 goto build_fail
@echo Build Succeeded

goto end

:build_fail
@echo Build Failed

:end
@cd %OLD_CD%