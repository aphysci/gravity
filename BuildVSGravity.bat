@echo off
setlocal

VERSION_CHOICE=

if [%1]==[] (
	echo "Please Select ... ?? TODO"
	echo "1.  Release VS2012 32-bit"
	echo "2.  Debug VS2012 32-bit"
	echo "3.  Release VS2012 64-bit"
	echo "4.  Debug VS2012 64-bit"
	echo "5.  Release VS2010 32-bit"
	echo "6.  Debug VS2010 32-bit"
	echo "7.  Release VS2010 64-bit"
	echo "8.  Debug VS2010 64-bit"
	echo "9.  All"
	
	choice /c 123456789 /n /m "Version Number: "
	VERSION_CHOICE=%ERRORLEVEL%
) else (
	VERSION_CHOICE=%1
)


if %VERSION_CHOICE% EQU 1 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 2 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 3 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 4 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 5 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Release2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 6 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 7 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 8 (
	cd build\msvs
	CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
) else if %VERSION_CHOICE% EQU 9 (
	cd build\msvs
	
	CONFIGURATION= /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=v110 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Release2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Release2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	
	CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=x64 /p:PlatformToolset=Windows7.1SDK 
	msbuild gravity.sln %CONFIGURATION% || goto build_fail
	cd ..\..
)

echo "Build Succeded"
exit 0

:build_failed
cd ..\..
echo "Build Failed"
exit -1 