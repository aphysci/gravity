Rem ember to run this from the Visual Studio Command Prompt.  
mkdir bin
mkdir lib

cd ThirdParty

mkdir bin
mkdir lib

cd protobuf-2.4.1\vsprojects
msbuild libprotobuf.vcproj /p:Configuration=Release
msbuild protoc.vcproj /p:Configuration=Release
copy Release\libprotobuf.lib ..\..\lib
copy Release\protoc.exe ..\..\bin

cd ..\..\zeromq-3.2.0\builds\msvc
msbuild msvc.sln /p:Configuration=Release
copy Release\libzmq.lib ..\..\..\lib
copy ..\..\lib\libzmq.dll ..\..\..\bin

cd ..\..\..\iniparser\build
msbuild iniparser.sln /p:Configuration=Release
Rem lib File from this project is built directly into the lib directory

cd ..\..\..

cd build\msvs\gravity
msbuild gravity.sln /p:Configuration=Release
copy Release\gravity.dll ..\..\..\bin
copy Release\gravity.lib ..\..\..\lib

cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln /p:Configuration=Release
copy Release\ServiceDirectory.exe ..\..\..\..\bin
copy ..\..\..\..\src\components\cpp\ServiceDirectory\ServiceDirectory.ini ..\..\..\..\bin
cd ..\..\..\..\
