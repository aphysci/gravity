::** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
::**
::** Gravity is free software; you can redistribute it and/or modify
::** it under the terms of the GNU Lesser General Public License as published by
::** the Free Software Foundation; either version 3 of the License, or
::** (at your option) any later version.
::**
::** This program is distributed in the hope that it will be useful,
::** but WITHOUT ANY WARRANTY; without even the implied warranty of
::** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
::** GNU Lesser General Public License for more details.
::**
::** You should have received a copy of the GNU Lesser General Public 
::** License along with this program;
::** If not, see <http://www.gnu.org/licenses/>.
::**

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

if not defined PROTOBUF_HOME (
   echo You must define PROTOBUF_HOME
   goto build_fail
   end
)

if not defined ZMQ_HOME (
   echo You must define ZMQ_HOME
   goto build_fail
   end
)

if not defined PTHREADS_HOME (
   echo You must define PTHREADS_HOME
   goto build_fail
   end
)

if not defined GRAVITY_THIRD_PARTY_LIBS (
    echo You must define GRAVITY_THIRD_PARTY_LIBS
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
echo 1 - Release VS2010 32-bit
echo 2 - Debug VS2010 32-bit
echo 3 - Gravity Components (VS2010 32-bit executables)
echo 4 - Debug VS2010 64-bit
echo 5 - Release VS2010 64-bit
echo 6 - Gravity Components (VS2010 64-bit executables)
echo Q - Quit

echo Build Selection:	
::choice /c:123456789Q>nul
choice /c:123456Q>nul

if errorlevel 7 goto done
if errorlevel 6 goto GravityComponents64
if errorlevel 5 goto VS201064R
if errorlevel 4 goto VS201064D
if errorlevel 3 goto GravityComponents
if errorlevel 2 goto VS201032D
if errorlevel 1 goto VS201032R


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

:GravityComponents64

:: 64-bit release for all the components
call setenv /x64 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK
goto BuildGravityComponents

:GravityComponents

:: 32-bit release for all the components
call setenv /x86 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK

:BuildGravityComponents

echo ========== BUILDING ServiceDirectory ==========
pushd build\msvs\components\ServiceDirectory
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% ServiceDirectory.sln || goto build_fail
)
msbuild /target:ServiceDirectory %CONFIGURATION% ServiceDirectory.sln || goto build_fail
popd

::goto SkipArchiverAndPlayback

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
:: Build keyvalue parser
pushd build\msvs\keyvalue_parser
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% keyvalue_parser.sln || goto build_fail
)
msbuild %CONFIGURATION% keyvalue_parser.sln || goto build_fail
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
xcopy /s /y %PROTOBUF_HOME%\src\*.h include 
xcopy /s /y %PTHREADS_HOME%\include\*.h include 
md include\MATLAB
copy src\api\MATLAB\*.m include\MATLAB
::md lib\MATLAB
::copy src\api\MATLAB\*.jar lib\MATLAB

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
:: Build libgravity_wrap
pushd build\msvs\libgravity_wrap
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% libgravity_wrap.sln || goto build_fail
)
msbuild %CONFIGURATION% libgravity_wrap.sln || goto build_fail
popd
pushd src\api\java
move gravity.jar %GRAVITY_LIB_PATH%
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
popd

goto menu

:build_fail
popd
echo Build Failed

:done
