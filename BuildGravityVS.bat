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

cd build\msvs\components\LogRecorder
msbuild LogRecorder.sln /p:Configuration=Release || goto build_fail
copy Release\LogRecorder.exe ..\..\..\..\bin
cd ..\..\..\..\

cd build\msvs\components\ConfigServer
msbuild ConfigServer.sln /p:Configuration=Release || goto build_fail
copy Release\ConfigServer.exe ..\..\..\..\bin
cd ..\..\..\..\

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

IF DEFINED MSYSTEM (
REM Build the Java code
cd src\api\java
make
cd ..\..\..
copy src\api\java\lib* lib
copy src\api\java\gravity.jar lib

REM Build the MATLAB-specific code
cd src\api\MATLAB
make
cd ..\..\..
mkdir lib\MATLAB
copy src\api\MATLAB\*.jar lib\MATLAB
mkdir include\MATLAB
copy src\api\MATLAB\*.m include\MATLAB
)

goto end

:build_fail
@echo Build Failed

:end
