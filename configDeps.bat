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

if not defined PROTOBUF_HOME (
	echo You must define PROTOBUF_HOME
	goto done
	end
)

if not defined ZMQ_HOME (
	echo You must define ZMQ_HOME
	goto done
	end
)

if not defined PTHREADS_HOME (
	echo You must define PTHREADS_HOME
	goto done
	end
)

:menu
echo.
echo ===========================
echo ===== BUILD OPTIONS =======
echo.
echo 1 - VS2010 32-bit
echo 2 - VS2010 64-bit
echo 3 - VS2012 32-bit
echo 4 - VS2012 64-bit
echo 5 - VS2013 64-bit
echo Q - Quit

echo Build Selection:	
choice /c:12345Q>nul

if errorlevel 6 goto done
if errorlevel 5 goto VS201364
if errorlevel 4 goto VS201264
if errorlevel 3 goto VS201232
if errorlevel 2 goto VS201064
if errorlevel 1 goto VS201032

echo NO SELECTION
goto done

:VS201032
echo ===== Configuring Gravity Dependencies for VS2010 32-bit =====
echo.
set PLATFORM=Win32
set PTHREAD_PLATFORM=x86
set PTHREAD_LIB=pthreadVCE2
set CONFIGURATION=Release2010
set DEBUG_CONFIGURATION=Debug2010
set VERSION=v100
goto config

:VS201064
echo ===== Configuring Gravity Dependencies for VS2010 64-bit =====
echo.
set PLATFORM=x64
set PTHREAD_PLATFORM=x64
set PTHREAD_LIB=pthreadVC2
set CONFIGURATION=Release2010
set DEBUG_CONFIGURATION=Debug2010
set VERSION=v100
goto config

:VS201232
echo ===== Configuring Gravity Dependencies for VS2012 32-bit =====
echo.
set PLATFORM=Win32
set PTHREAD_PLATFORM=x86
set PTHREAD_LIB=pthreadVCE2
set CONFIGURATION=Release2012
set DEBUG_CONFIGURATION=Debug2012
set VERSION=v110
goto config

:VS201264
echo ===== Configuring Gravity Dependencies for VS2012 64-bit =====
echo.
set PLATFORM=x64
set PTHREAD_PLATFORM=x64
set PTHREAD_LIB=pthreadVC2
set CONFIGURATION=Release2012
set DEBUG_CONFIGURATION=Debug2012
set VERSION=v110
goto config

:VS201364
echo ===== Configuring Gravity Dependencies for VS2013 64-bit =====
echo.
set PLATFORM=x64
set PTHREAD_PLATFORM=x64
set PTHREAD_LIB=pthreadVC2
set CONFIGURATION=Release2013
set DEBUG_CONFIGURATION=Debug2013
set VERSION=v120
goto config

:config
echo ===== Configuring Gravity Dependencies =====
echo.

set GRAVITY_DEPS=%PLATFORM%\%CONFIGURATION%\deps
rd /s /q %GRAVITY_DEPS%
md %GRAVITY_DEPS%

echo ===== Copying ZMQ libs =====
echo.
copy /y "%ZMQ_HOME%\lib\libzmq-%VERSION%-mt-3_2_3.lib" %GRAVITY_DEPS%
copy /y "%ZMQ_HOME%\lib\libzmq-%VERSION%-mt-gd-3_2_3.lib" %GRAVITY_DEPS%
copy /y "%ZMQ_HOME%\bin\libzmq-%VERSION%-mt-3_2_3.dll" %GRAVITY_DEPS%
copy /y "%ZMQ_HOME%\bin\libzmq-%VERSION%-mt-gd-3_2_3.dll" %GRAVITY_DEPS%
pushd %GRAVITY_DEPS%
fsutil hardlink create libzmq.lib libzmq-%VERSION%-mt-3_2_3.lib
fsutil hardlink create libzmq_d.lib libzmq-%VERSION%-mt-gd-3_2_3.lib
fsutil hardlink create libzmq.dll libzmq-%VERSION%-mt-3_2_3.dll
fsutil hardlink create libzmq_d.dll libzmq-%VERSION%-mt-gd-3_2_3.dll
popd


echo ===== Copying Protobuf libs =====
echo.
if %PLATFORM% EQU x64 (
	copy /y %PROTOBUF_HOME%\vsprojects\x64\release\libprotobuf.lib %GRAVITY_DEPS%
	copy /y %PROTOBUF_HOME%\vsprojects\x64\debug\libprotobuf_d.lib %GRAVITY_DEPS%
) else (
	copy /y %PROTOBUF_HOME%\vsprojects\release\libprotobuf.lib %GRAVITY_DEPS%
	copy /y %PROTOBUF_HOME%\vsprojects\debug\libprotobuf_d.lib %GRAVITY_DEPS%
)

echo ===== Copying PThreads libs =====
echo.
copy /y %PTHREADS_HOME%\lib\%PTHREAD_PLATFORM%\%PTHREAD_LIB%.lib %GRAVITY_DEPS%
copy /y %PTHREADS_HOME%\dll\%PTHREAD_PLATFORM%\%PTHREAD_LIB%.dll %GRAVITY_DEPS%

echo ===== Build Java Protobuf Runtime lib =====
echo.
pushd %PROTOBUF_HOME%\java
copy /y %PROTOBUF_HOME%\protoc.exe ..\src
call mvn package
popd
copy /y %PROTOBUF_HOME%\java\target\protobuf-java-2.5.0.jar %GRAVITY_DEPS%\protobuf-java.jar

echo ===== Copy to Debug configuration =====
echo.
set DEBUG_DEPS=%PLATFORM%\%DEBUG_CONFIGURATION%\deps
md %DEBUG_DEPS%
xcopy %GRAVITY_DEPS% %DEBUG_DEPS% /S /Y

:done
