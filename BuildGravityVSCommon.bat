Rem Build gravity

Rem put directory containing protoc in the path.  
setlocal PATH=%PATH%;%OLD_CD%\%BIN_DIR%

cd build\msvs\gravity
msbuild gravity.sln %CONFIGURATION% || goto build_fail

Rem Build components
cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln %CONFIGURATION% || goto build_fail
cd ..\..\..\..\

cd build\msvs\components\LogRecorder
msbuild LogRecorder.sln %CONFIGURATION% || goto build_fail
cd ..\..\..\..\

cd build\msvs\components\ConfigServer
msbuild ConfigServer.sln %CONFIGURATION% || goto build_fail
cd ..\..\..\..\

Rem Copy files to output directory.  
copy build\msvs\components\ServiceDirectory\%BUILD_DIR%\ServiceDirectory.exe %BIN_DIR% || goto build_fail
copy build\msvs\components\LogRecorder\%BUILD_DIR%\LogRecorder.exe %BIN_DIR% || goto build_fail
copy build\msvs\components\ConfigServer\%BUILD_DIR%\ConfigServer.exe %BIN_DIR% || goto build_fail

copy configs\ServiceDirectory.ini %BIN_DIR% || goto build_fail
copy configs\config_file.ini %BIN_DIR% || goto build_fail

xcopy /s /q /y Thirdparty\include\*.h include || goto build_fail
copy Thirdparty\%BIN_DIR%\* %BIN_DIR% || goto build_fail
copy Thirdparty\%LIB_DIR%\libprotobuf.lib %LIB_DIR% || goto build_fail

copy src\api\cpp\*.h include || goto build_fail
copy src\api\cpp\protobuf\GravityDataProductPB.pb.h include\protobuf || goto build_fail

Rem Try to build Archiver/Playback
where /q cmake
if not errorlevel 1 (

cd build\msvs\components\Archiver
msbuild Archiver.sln %CONFIGURATION% || goto build_fail
cd ..\..\..\..\

cd build\msvs\components\Playback
msbuild Playback.sln %CONFIGURATION% || goto build_fail
cd ..\..\..\..\

copy build\msvs\components\Archiver\%BUILD_DIR%\Archiver.exe %BIN_DIR% || goto build_fail
copy build\msvs\components\Playback\%BUILD_DIR%\Playback.exe %BIN_DIR% || goto build_fail
copy configs\GravityArchiver.ini %BIN_DIR% || goto build_fail
copy configs\GravityPlayback.ini %BIN_DIR% || goto build_fail

) else (
@echo "Missing Cmake.  Skipping Archiver/Playback"
)

REM Move third party libs into gravity lib dir
copy ThirdParty\guava-13.0.1\guava-13.0.1.jar %LIB_DIR% || goto build_fail

REM Build the Java code
if DEFINED JAVA_HOME (
cd build\msvs\java
msbuild java.sln %CONFIGURATION% || goto build_fail
copy src\api\java\gravity.jar ..\..\..\%LIB_DIR%
cd ..\..\..

REM Build the MATLAB-specific code
cd src\api\MATLAB
nmake /f nmake.mak
cd ..\..\..
mkdir %LIB_DIR%\MATLAB
copy src\api\MATLAB\*.jar %LIB_DIR%\MATLAB
mkdir include\MATLAB
copy src\api\MATLAB\*.m include\MATLAB
)
else
(
@echo JAVA_HOME not set.  Skipping Java and Matlab build.  
)

@echo Zipping files
del %ZIP_OUT_NAME%
AddToZip include %ZIP_OUT_NAME%
AddToZip %LIB_DIR% %ZIP_OUT_NAME%
AddToZip %BIN_DIR% %ZIP_OUT_NAME%

goto end

:build_fail
@exit /B 1

:end