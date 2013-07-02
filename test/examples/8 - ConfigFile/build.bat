call setenv /x86 /debug
pushd msvs\ConfigFileExample
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
msbuild /target:Clean %CONFIGURATION% ConfigFileExample.sln
msbuild %CONFIGURATION% ConfigFileExample.sln
copy %GRAVITY_HOME%\Win32\Debug2010\bin\* .\Debug2010
popd
copy Gravity.ini .\msvs\ConfigFileExample\Debug2010
