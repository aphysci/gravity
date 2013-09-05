::** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
::**
::** Gravity is free software; you can redistribute it and/or modify
::** it under the terms of the GNU Lesser General Public License as published by
::** the Free Software Foundation; either version 3 of the License, or
::** (at your option) any later version.
::**
::** This program is distributed in the hope that it will be useful,
::** but WITHOUT ANY WARRANTY; without even the implied warranty of
::** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
::** GNU Lesser General Public License for more details.
::**
::** You should have received a copy of the GNU Lesser General Public 
::** License along with this program;
::** If not, see <http://www.gnu.org/licenses/>.
::**

call setenv /x86 /debug
pushd msvs\ConfigFileExample
set CONFIGURATION= /p:Configuration=Debug2010 /p:Platform=Win32 /p:PlatformToolset=Windows7.1SDK 
msbuild /target:Clean %CONFIGURATION% ConfigFileExample.sln
msbuild %CONFIGURATION% ConfigFileExample.sln
copy %GRAVITY_HOME%\Win32\Debug2010\bin\* .\Debug2010
popd
copy Gravity.ini .\msvs\ConfigFileExample\Debug2010
