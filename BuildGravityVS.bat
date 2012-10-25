Rem ember to run this from the Visual Studio Command Prompt.  
mkdir bin
mkdir lib
mkdir include

Rem Build ThirdParty
cd ThirdParty

mkdir bin
mkdir lib

cd protobuf-2.4.1\vsprojects
msbuild libprotobuf.vcxproj /p:Configuration=Release
msbuild protoc.vcxproj /p:Configuration=Release
copy Release\libprotobuf.lib ..\..\lib
copy Release\protoc.exe ..\..\bin

cd ..\..\zeromq-3.2.1\builds\msvc11
msbuild msvc11.sln /p:Configuration=Release
copy ..\..\lib\Win32\libzmq.lib ..\..\..\lib\libzmq.lib
copy ..\..\bin\Win32\libzmq.dll ..\..\..\bin\libzmq.dll

cd ..\..\..\iniparser\build
msbuild iniparser.sln /p:Configuration=Release
Rem lib File from this project is built directly into the lib directory

cd ..\..\..
copy ThirdParty\pthreads\lib\pthreadVCE2.lib ThirdParty\lib
copy ThirdParty\pthreads\bin\pthreadVCE2.dll ThirdParty\bin

Rem Build gravity
cd build\msvs\gravity
msbuild gravity.sln /p:Configuration=Release
copy Release\gravity.dll ..\..\..\bin
copy Release\gravity.lib ..\..\..\lib

cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln /p:Configuration=Release
copy Release\ServiceDirectory.exe ..\..\..\..\bin
copy ..\..\..\..\src\components\cpp\ServiceDirectory\ServiceDirectory.ini ..\..\..\..\bin
cd ..\..\..\..\

Rem Copy files to output directory.  
copy Thirdparty\bin\* bin
copy Thirdparty\lib\libprotobuf.lib lib

copy src\api\cpp\*.h include

xcopy /s /q /y ThirdParty\protobuf-2.4.1\src\*.h include
cd include\google
cd ..\..
