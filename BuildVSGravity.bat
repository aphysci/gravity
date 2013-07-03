@echo off
setlocal

set Clean=1

if not defined GRAVITY_HOME (
   echo You must define GRAVITY_HOME
   goto build_fail
   end
)

if not defined BOOST_HOME (
   echo You must define BOOST_HOME
   goto build_fail
   end
)

if not defined LEX_CMD (
   set LEX_CMD=win_flex.exe
)

if not defined YACC_CMD (
   set YACC_CMD=win_bison.exe
)


:menu
echo.
echo ===========================
echo ===== BUILD OPTIONS =======
echo.
::echo 1 - Release VS2012 32-bit
::echo 2 - Debug VS2012 32-bit
::echo 3 - Release VS2012 64-bit
::echo 4 - Debug VS2012 64-bit
echo 1 - Release VS2010 32-bit
echo 2 - Debug VS2010 32-bit
::echo 3 - Release VS2010 64-bit
::echo 4 - Debug VS2010 64-bit
echo 3 - Gravity Components (VS2010 32-bit executables)
::echo 6 - Java 32R
::echo 7 - Java 32D
::echo 8 - Java 64R
::echo 9 - Java 64D
echo Q - Quit

echo Build Selection:	
::choice /c:123456789Q>nul
choice /c:123Q>nul

::if errorlevel 10 goto done
::if errorlevel 9 goto Java64D
::if errorlevel 8 goto Java64R
::if errorlevel 7 goto Java32D
::if errorlevel 6 goto Java32R
if errorlevel 4 goto done
if errorlevel 3 goto GravityComponents
::if errorlevel 4 goto VS201064D
::if errorlevel 3 goto VS201064R
if errorlevel 2 goto VS201032D
if errorlevel 1 goto VS201032R
::if errorlevel 4 goto VS201264D
::if errorlevel 3 goto VS201264R
::if errorlevel 2 goto VS201232D
::if errorlevel 1 goto VS201232R

echo NO SELECTION
goto done

:VS201232R
echo ========== BUILDING VS2012 32-bit Release ==========
call setenv /x86 /release
set CONFIGURATION= /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v110 
goto build

:VS201232D
echo ========== BUILDING VS2012 32-bit Debug ==========
call setenv /x86 /debug
set CONFIGURATION= /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v110 
goto build

:VS201264R
echo ========== BUILDING VS2012 64-bit Release ==========
call setenv /x64 /release
set CONFIGURATION= /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v110
goto build

:VS201264D
echo ========== BUILDING VS2012 64-bit Debug ==========
call setenv /x64 /debug
set CONFIGURATION= /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=v110 
goto build

:Java32R
call setenv /x86 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK
set PROTOC_DIR=Win32\Release2010\bin
set GRAVITY_LIB_PATH=..\..\..\Win32\Release2010\lib
set GRAVITY_CONFIG=RELEASE
goto java

:Java32D
call setenv /x86 /debug
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
set PROTOC_DIR=Win32\Debug2010\bin
set GRAVITY_LIB_PATH=..\..\..\Win32\Debug2010\lib
set GRAVITY_CONFIG=DEBUG
goto java

:Java64R
call setenv /x64 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
set PROTOC_DIR=x64\Release2010\bin
set GRAVITY_LIB_PATH=..\..\..\x64\Release2010\lib
set GRAVITY_CONFIG=RELEASE
goto java

:Java64D
call setenv /x64 /debug
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
set PROTOC_DIR=x64\Debug2010\bin
set GRAVITY_LIB_PATH=..\..\..\x64\Debug2010\lib
set GRAVITY_CONFIG=DEBUG
goto java

:VS201032R
echo ========== BUILDING VS2010 32-bit Release ==========
call setenv /x86 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK
set PROTOC_DIR=Win32\Release2010\bin
set GRAVITY_LIB_PATH=..\..\..\Win32\Release2010\lib
set GRAVITY_CONFIG=RELEASE
goto build

:VS201032D
echo ========== BUILDING VS2010 32-bit Debug ==========
call setenv /x86 /debug
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
set PROTOC_DIR=Win32\Debug2010\bin
set GRAVITY_LIB_PATH=..\..\..\Win32\Debug2010\lib
set GRAVITY_CONFIG=DEBUG
goto build

:VS201064R
echo ========== BUILDING VS2010 64-bit Release ==========
call setenv /x64 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
set PROTOC_DIR=x64\Release2010\bin
set GRAVITY_LIB_PATH=..\..\..\x64\Release2010\lib
set GRAVITY_CONFIG=RELEASE
goto build

:VS201064D
echo ========== BUILDING VS2010 64-bit Debug ==========
call setenv /x64 /debug
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
set PROTOC_DIR=x64\Debug2010\bin
set GRAVITY_LIB_PATH=..\..\..\x64\Debug2010\lib
set GRAVITY_CONFIG=DEBUG
goto build

:GravityComponents

:: 32-bit release for all the components
call setenv /x86 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK

echo ========== BUILDING ServiceDirectory ==========
pushd build\msvs\components\ServiceDirectory
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% ServiceDirectory.sln || goto build_fail
)
msbuild /target:ServiceDirectory %CONFIGURATION% ServiceDirectory.sln || goto build_fail
popd

goto SkipArchiverAndPlayback

echo ========== BUILDING Archiver ==========
pushd build\msvs\components\Archiver
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% Archiver.sln || goto build_fail
)
msbuild /target:Archiver %CONFIGURATION% Archiver.sln || goto build_fail
popd


echo ========== BUILDING Playback ==========
pushd build\msvs\components\Playback
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% Playback.sln || goto build_fail
)
msbuild /target:Playback %CONFIGURATION% Playback.sln || goto build_fail
popd

:SkipArchiverAndPlayback

echo ========== BUILDING ConfigServer ==========
pushd build\msvs\components\ConfigServer
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% ConfigServer.sln || goto build_fail
)
msbuild /target:ConfigServer %CONFIGURATION% ConfigServer.sln || goto build_fail
popd

echo ========== BUILDING LogRecorder ==========
pushd build\msvs\components\LogRecorder
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% LogRecorder.sln || goto build_fail
)
msbuild /target:LogRecorder %CONFIGURATION% LogRecorder.sln || goto build_fail
popd

goto menu

:build
:: Build Protobufs
pushd ThirdParty\protobuf-2.4.1\vsprojects
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% protobuf.sln || goto build_fail
)
	msbuild /target:libprotobuf;protoc %CONFIGURATION% protobuf.sln || goto build_fail
popd

:: Build keyvalue parser
pushd src\keyvalue_parser\
%LEX_CMD% -o .\lex.yy.c .\keyvalue.l || goto build_fail
%YACC_CMD% -dt .\keyvalue.y -o .\y.tab.c || goto build_fail
popd
pushd src\keyvalue_parser\msvs\keyvalue_parser
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% keyvalue_parser.sln || goto build_fail
)
msbuild %CONFIGURATION% keyvalue_parser.sln || goto build_fail
popd

:: Build ZMQ
pushd ThirdParty\zeromq-3.2.1\builds\msvc11
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% msvc11.sln || goto build_fail
)
msbuild /target:libzmq %CONFIGURATION% msvc11.sln || goto build_fail
popd

:: Build Gravity
pushd build\msvs\gravity
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% gravity.sln || goto build_fail
)
msbuild /target:gravity %CONFIGURATION% gravity.sln || goto build_fail
popd

:: Populate the include directory
md include
copy src\api\cpp\*.h include
md include\protobuf
copy src\api\cpp\protobuf\GravityDataProductPB.pb.h include\protobuf
copy src\api\cpp\protobuf\GravityMetricsDataPB.pb.h include\protobuf
xcopy /s /y ThirdParty\protobuf-2.4.1\src\*.h include 
xcopy /s /y ThirdParty\pthreads\include\*.h include 
md include\MATLAB
copy src\api\MATLAB\*.m include\MATLAB
::md lib\MATLAB
::copy src\api\MATLAB\*.jar lib\MATLAB

:: Copy the protoc executable
copy %PROTOC_DIR%\protoc.exe ThirdParty\bin

echo.
echo ================================
echo ======= BUILD SUCCESSFUL =======
echo ================================
echo.

echo.
echo ================================
echo ======== BUILDING JAVA =========
echo ================================
echo.
:java
:: Build Java
pushd src\api\java
make -f Makefile clean
make -f Makefile
move gravity.jar %GRAVITY_LIB_PATH%
if %GRAVITY_CONFIG% == RELEASE (
	move libgravity_wrap.dll %GRAVITY_LIB_PATH%\..\bin
	move libgravity_wrap.lib %GRAVITY_LIB_PATH%
	move libgravity_wrap.exp %GRAVITY_LIB_PATH%
) else (
	move libgravity_wrap_d.dll %GRAVITY_LIB_PATH%\..\bin
	move libgravity_wrap_d.lib %GRAVITY_LIB_PATH%
	move libgravity_wrap_d.exp %GRAVITY_LIB_PATH%
)
popd

echo.
echo ================================
echo ======= BUILDING MATLAB ========
echo ================================
echo.
pushd src\api\MATLAB
md build
javac -d build -cp %GRAVITY_LIB_PATH%/gravity.jar;../../../ThirdParty/guava-13.0.1/guava-13.0.1.jar MATLABGravitySubscriber.java
jar cf MATLABGravitySubscriber.jar -C build .
md %GRAVITY_LIB_PATH%\MATLAB
copy MATLABGravitySubscriber.jar %GRAVITY_LIB_PATH%\MATLAB
copy ..\..\..\ThirdParty\guava-13.0.1\guava-13.0.1.jar %GRAVITY_LIB_PATH%
copy ..\..\..\ThirdParty\lib\protobuf-java-2.4.1.jar %GRAVITY_LIB_PATH%
popd

goto menu

:build_fail
::\cd ..\..\..
popd
echo Build Failed

:done
