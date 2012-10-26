Rem ember to run this from the Visual Studio Command Prompt.  
mkdir debug_bin
mkdir debug_lib
mkdir include

Rem Build ThirdParty
cd ThirdParty

mkdir debug_bin
mkdir debug_lib

cd protobuf-2.4.1\vsprojects
msbuild libprotobuf.vcxproj /p:Configuration=Debug || goto build_fail
msbuild protoc.vcxproj /p:Configuration=Debug || goto build_fail
copy Debug\libprotobuf.lib ..\..\debug_lib
copy Debug\protoc.exe ..\..\debug_bin

REM TODO: check this!!!
cd ..\..\zeromq-3.2.1\builds\msvc11
msbuild msvc11.sln /p:Configuration=Debug || goto build_fail
copy ..\..\lib\Win32\libzmq_d.lib ..\..\..\debug_lib\libzmq_d.lib
copy ..\..\bin\Win32\libzmq_d.dll ..\..\..\debug_bin\libzmq_d.dll

cd ..\..\..\iniparser\build
msbuild iniparser.sln /p:Configuration=Debug || goto build_fail
Rem lib File from this project is built directly into the lib directory

cd ..\..\..
copy ThirdParty\pthreads\lib\pthreadVCE2.lib ThirdParty\debug_lib
copy ThirdParty\pthreads\bin\pthreadVCE2.dll ThirdParty\debug_bin

Rem Build gravity
cd build\msvs\gravity
msbuild gravity.sln /p:Configuration=Debug || goto build_fail
copy Debug\gravity.dll ..\..\..\debug_bin
copy Debug\gravity.lib ..\..\..\debug_lib

cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln /p:Configuration=Debug || goto build_fail
copy Debug\ServiceDirectory.exe ..\..\..\..\debug_bin
copy ..\..\..\..\src\components\cpp\ServiceDirectory\ServiceDirectory.ini ..\..\..\..\debug_bin
cd ..\..\..\..\

Rem Copy files to output directory.  
copy Thirdparty\debug_bin\* debug_bin
copy Thirdparty\lib\libprotobuf.lib debug_lib

copy src\api\cpp\*.h include

xcopy /s /q /y ThirdParty\protobuf-2.4.1\src\*.h include
cd include\google
cd ..\..

goto end

:build_fail
@echo Build Failed

:end
