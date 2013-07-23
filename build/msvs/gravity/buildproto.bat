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

mkdir ..\..\..\src\api\cpp\protobuf\
for %%i in (..\..\..\src\api\protobufs\*.proto) do ..\..\..\ThirdParty\protobuf-2.4.1\vsprojects\Release\protoc.exe --cpp_out=..\..\..\src\api\cpp\protobuf\ --proto_path=..\..\..\src\api\protobufs\ %%i