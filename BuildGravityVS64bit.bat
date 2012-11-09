Rem ember to run this from the 64 BIT Visual Studio Command Prompt.  
mkdir bin64
mkdir lib64
mkdir include
mkdir include\protobuf

Rem Build ThirdParty
cd ThirdParty

mkdir bin64
mkdir lib64

cd protobuf-2.4.1\vsprojects
msbuild libprotobuf.vcxproj /p:Configuration=Release /p:Platform=x64 || goto build_fail
msbuild protoc.vcxproj /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy x64\Release\libprotobuf.lib ..\..\lib64
copy x64\Release\protoc.exe ..\..\bin64

cd ..\..\zeromq-3.2.1\builds\msvc11
msbuild msvc11.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy ..\..\lib\x64\libzmq.lib ..\..\..\lib64\libzmq.lib
copy ..\..\bin\x64\libzmq.dll ..\..\..\bin64\libzmq.dll

cd ..\..\..\iniparser\build
msbuild iniparser.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
Rem lib File from this project is built directly into the lib directory

cd ..\..\..
copy ThirdParty\pthreads\lib\x64\pthreadVC2.lib ThirdParty\lib64
copy ThirdParty\pthreads\bin\x64\pthreadVC2.dll ThirdParty\bin64
copy ThirdParty\pthreads\include\*.h include\

Rem Build gravity
cd build\msvs\gravity
msbuild gravity.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail

Rem Build components
cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy x64\Release\ServiceDirectory.exe ..\..\..\..\bin64
copy ..\..\..\..\src\components\cpp\ServiceDirectory\ServiceDirectory.ini ..\..\..\..\bin64
cd ..\..\..\..\

cd build\msvs\components\LogRecorder
msbuild LogRecorder.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy x64\Release\LogRecorder.exe ..\..\..\..\bin64
cd ..\..\..\..\

cd build\msvs\components\ConfigServer
msbuild ConfigServer.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy x64\Release\ConfigServer.exe ..\..\..\..\bin64
cd ..\..\..\..\

where /q cmake
if not errorlevel 1 (

cd ThirdParty\cppdb-trunk
mkdir buildMSVS11-64bit
cd buildMSVS11-64bit
cmake -G"Visual Studio 11 Win64" ..
msbuild cppdb.vcxproj /p:Configuration=Release /p:Platform=x64 || goto build_fail
copy Release\cppdb.lib ..\..\lib64
copy Release\cppdb.dll ..\..\bin64
cd ..\..\..

cd build\msvs\components\Archiver
msbuild Archiver.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy x64\Release\Archiver.exe ..\..\..\..\bin64
copy ..\..\..\..\src\components\cpp\Archiver\GravityArchiver.ini ..\..\..\..\bin64
cd ..\..\..\..\

cd build\msvs\components\Playback
msbuild Playback.sln /p:Configuration=Release /p:Platform="x64" || goto build_fail
copy x64\Release\Playback.exe ..\..\..\..\bin64
copy ..\..\..\..\src\components\cpp\Playback\GravityPlayback.ini ..\..\..\..\bin64
cd ..\..\..\..\

) else (
echo "Missing Cmake.  Skipping Archiver/Playback"
)

Rem Copy files to output directory.  
copy Thirdparty\bin64\* bin64
copy Thirdparty\lib64\libprotobuf.lib lib64

copy src\api\cpp\*.h include
copy src\api\cpp\protobuf\GravityDataProductPB.pb.h include\protobuf

xcopy /s /q /y ThirdParty\protobuf-2.4.1\src\*.h include
cd include\google
cd ..\..

goto end

:build_fail
@echo Build Failed

:end
