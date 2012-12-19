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


:menu
echo.
echo ===========================
echo ===== BUILD OPTIONS =======
echo.
echo 1 - Release VS2012 32-bit
echo 2 - Debug VS2012 32-bit
echo 3 - Release VS2012 64-bit
echo 4 - Debug VS2012 64-bit
echo 5 - Release VS2010 32-bit
echo 6 - Debug VS2010 32-bit
echo 7 - Release VS2010 64-bit
echo 8 - Debug VS2010 64-bit
echo 9 - Gravity Components (32-bit executables)
echo Q - Quit

echo Build Selection:	
choice /c:123456789Q>nul

if errorlevel 10 goto done
if errorlevel 9 goto GravityComponents
if errorlevel 8 goto VS201064D
if errorlevel 7 goto VS201064R
if errorlevel 6 goto VS201032D
if errorlevel 5 goto VS201032R
if errorlevel 4 goto VS201264D
if errorlevel 3 goto VS201264R
if errorlevel 2 goto VS201232D
if errorlevel 1 goto VS201232R

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
goto build

:VS201032D
echo ========== BUILDING VS2010 32-bit Debug ==========
call setenv /x86 /debug
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
goto build

:VS201064R
echo ========== BUILDING VS2010 64-bit Release ==========
call setenv /x64 /release
set CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
goto build

:VS201064D
echo ========== BUILDING VS2010 64-bit Debug ==========
call setenv /x64 /debug
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
goto build

:GravityComponents

:: 32-bit release for all the components
call setenv /x86 /release
set CONFIGURATION= /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v110 

echo ========== BUILDING ServiceDirectory ==========
pushd build\msvs\components\ServiceDirectory
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% ServiceDirectory.sln || goto build_fail
)
msbuild /target:ServiceDirectory %CONFIGURATION% ServiceDirectory.sln || goto build_fail
popd

goto skip

echo ========== BUILDING Archiver ==========
pushd build\msvs\components\Archiver
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% Archiver.sln || goto build_fail
)
msbuild /target:Archiver %CONFIGURATION% Archiver.sln || goto build_fail
popd

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

echo ========== BUILDING Playback ==========
pushd build\msvs\components\Playback
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% Playback.sln || goto build_fail
)
msbuild /target:Playback %CONFIGURATION% Playback.sln || goto build_fail
popd

:skip
goto menu

:build
:: Build Protobufs
pushd ThirdParty\protobuf-2.4.1\vsprojects
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% protobuf.sln || goto build_fail
)
	msbuild /target:libprotobuf;protoc %CONFIGURATION% protobuf.sln || goto build_fail
popd

:: Build iniparser
pushd ThirdParty\iniparser\build
if %Clean% EQU 1 (
	msbuild /target:Clean %CONFIGURATION% iniparser.sln || goto build_fail
)
msbuild /target:iniparser %CONFIGURATION% iniparser.sln || goto build_fail
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
xcopy /s /y ThirdParty\protobuf-2.4.1\src\*.h include 
xcopy /s /y ThirdParty\pthreads\include\*.h include 
md include\MATLAB
copy src\api\MATLAB\*.m include\MATLAB
::md lib\MATLAB
::copy src\api\MATLAB\*.jar lib\MATLAB

echo.
echo ================================
echo ======= BUILD SUCCESSFUL =======
echo ================================
echo.
goto menu

:build_fail
::\cd ..\..\..
popd
echo Build Failed

:done
