Rem Build gravity
cd build\msvs\gravity
msbuild gravity.sln %CONFIGURATION% || goto build_fail

Rem Build components
cd ..\..\..\build\msvs\components\ServiceDirectory
msbuild ServiceDirectory.sln %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\ServiceDirectory.exe ..\..\..\..\%BIN_DIR%
copy ..\..\..\..\src\components\cpp\ServiceDirectory\ServiceDirectory.ini ..\..\..\..\%BIN_DIR%
cd ..\..\..\..\

cd build\msvs\components\LogRecorder
msbuild LogRecorder.sln %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\LogRecorder.exe ..\..\..\..\%BIN_DIR%
cd ..\..\..\..\

cd build\msvs\components\ConfigServer
msbuild ConfigServer.sln %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\ConfigServer.exe ..\..\..\..\%BIN_DIR%
cd ..\..\..\..\

where /q cmake
if not errorlevel 1 (

cd build\msvs\components\Archiver
msbuild Archiver.sln %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\Archiver.exe ..\..\..\..\%BIN_DIR%
copy ..\..\..\..\src\components\cpp\Archiver\GravityArchiver.ini ..\..\..\..\%BIN_DIR%
cd ..\..\..\..\

cd build\msvs\components\Playback
msbuild Playback.sln %CONFIGURATION% || goto build_fail
copy %BUILD_DIR%\Playback.exe ..\..\..\..\%BIN_DIR%
copy ..\..\..\..\src\components\cpp\Playback\GravityPlayback.ini ..\..\..\..\%BIN_DIR%
cd ..\..\..\..\

) else (
echo "Missing Cmake.  Skipping Archiver/Playback"
)

Rem Copy files to output directory.  
xcopy /s /q /y Thirdparty\include\*.h include
copy Thirdparty\%BIN_DIR%\* %BIN_DIR%
copy Thirdparty\%LIB_DIR%\libprotobuf.lib %LIB_DIR%

copy src\api\cpp\*.h include
copy src\api\cpp\protobuf\GravityDataProductPB.pb.h include\protobuf

REM Move third party libs into gravity lib dir
copy ThirdParty\guava-13.0.1\guava-13.0.1.jar %LIB_DIR%
copy ThirdParty\%LIB_DIR%\* %LIB_DIR%

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
AddToZip include %ZIP_OUT_NAME%
AddToZip %LIB_DIR% %ZIP_OUT_NAME%
AddToZip %BIN_DIR% %ZIP_OUT_NAME%

goto end

:build_fail
@exit /B 1

:end