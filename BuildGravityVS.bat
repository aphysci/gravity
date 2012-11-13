Rem ember to run this from the Visual Studio Command Prompt.  
mkdir bin
mkdir lib
mkdir include
mkdir include\protobuf

Rem Build ThirdParty
cd ThirdParty

mkdir bin
mkdir lib

cd protobuf-2.4.1\vsprojects
msbuild libprotobuf.vcxproj /p:Configuration=Release || goto build_fail
msbuild protoc.vcxproj /p:Configuration=Release || goto build_fail
copy Release\libprotobuf.lib ..\..\lib
copy Release\protoc.exe ..\..\bin

cd ..\..\zeromq-3.2.1\builds\msvc11
msbuild msvc11.sln /p:Configuration=Release || goto build_fail
copy ..\..\lib\Win32\libzmq.lib ..\..\..\lib\libzmq.lib
copy ..\..\bin\Win32\libzmq.dll ..\..\..\bin\libzmq.dll

cd ..\..\..\iniparser\build
msbuild iniparser.sln /p:Configuration=Release || goto build_fail
Rem lib File from this project is built directly into the lib directory

cd ..\..\..
copy ThirdParty\pthreads\lib\pthreadVCE2.lib ThirdParty\lib
copy ThirdParty\pthreads\bin\pthreadVCE2.dll ThirdParty\bin
copy ThirdParty\pthreads\include\*.h include\

Rem Build gravity
cd build\msvs\gravity
msbuild gravity.sln /p:Configuration=Release || goto build_fail

Rem Build Components
cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln /p:Configuration=Release || goto build_fail
copy Release\ServiceDirectory.exe ..\..\..\..\bin
copy ..\..\..\..\src\components\cpp\ServiceDirectory\ServiceDirectory.ini ..\..\..\..\bin
cd ..\..\..\..\

cd build\msvs\components\LogRecorder
msbuild LogRecorder.sln /p:Configuration=Release || goto build_fail
copy Release\LogRecorder.exe ..\..\..\..\bin
cd ..\..\..\..\

cd build\msvs\components\ConfigServer
msbuild ConfigServer.sln /p:Configuration=Release || goto build_fail
copy Release\ConfigServer.exe ..\..\..\..\bin
cd ..\..\..\..\

where /q cmake
if not errorlevel 1 (

cd ThirdParty\cppdb-trunk
mkdir buildMSVS11
cd buildMSVS11
cmake -G"Visual Studio 11" ..
msbuild cppdb.vcxproj /p:Configuration=Release || goto build_fail
copy Release\cppdb.lib ..\..\lib
copy Release\cppdb.dll ..\..\bin
cd ..\..\..

cd build\msvs\components\Archiver
msbuild Archiver.sln /p:Configuration=Release || goto build_fail
copy Release\Archiver.exe ..\..\..\..\bin
copy ..\..\..\..\src\components\cpp\Archiver\GravityArchiver.ini ..\..\..\..\bin
cd ..\..\..\..\

cd build\msvs\components\Playback
msbuild Playback.sln /p:Configuration=Release || goto build_fail
copy Release\Playback.exe ..\..\..\..\bin
copy ..\..\..\..\src\components\cpp\Playback\GravityPlayback.ini ..\..\..\..\bin
cd ..\..\..\..\

) else (
echo "Missing Cmake.  Skipping Archiver/Playback"
)

Rem Copy files to output directory.  
copy Thirdparty\bin\* bin
copy Thirdparty\lib\libprotobuf.lib lib

copy src\api\cpp\*.h include
copy src\api\cpp\protobuf\GravityDataProductPB.pb.h include\protobuf

xcopy /s /q /y ThirdParty\protobuf-2.4.1\src\*.h include
cd include\google
cd ..\..

REM Move third party libs into gravity lib dir
copy ThirdParty\guava-13.0.1\guava-13.0.1.jar lib
copy ThirdParty\lib\* lib

REM Build the Java code
cd build\msvs\java
msbuild java.sln /p:Configuration=Release || goto build_fail
copy src\api\java\gravity.jar ..\..\..\lib
cd ..\..\..

REM Build the MATLAB-specific code
cd src\api\MATLAB
nmake /f nmake.mak
cd ..\..\..
mkdir lib\MATLAB
copy src\api\MATLAB\*.jar lib\MATLAB
mkdir include\MATLAB
copy src\api\MATLAB\*.m include\MATLAB

@echo Zipping files
AddToZip include gravity-MSVC11.zip
AddToZip lib gravity-MSVC11.zip
AddToZip bin gravity-MSVC11.zip

@echo Build Succeeded

goto end

:build_fail
@echo Build Failed

:end
