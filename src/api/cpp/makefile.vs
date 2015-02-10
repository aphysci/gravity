# (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#
# Gravity is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program;
# If not, see <http://www.gnu.org/licenses/>.

#Tools
CC=cl
LINK=link

#Naming
LIB_EXT=dll
OBJ_EXT=obj

#Compiler Flags
INCLUDES=//I "$(ZMQ_INCLUDE_DIR)" //I "$(PROTOBUF_INCLUDE_DIR)" //I "$(INIPARSE_INCLUDE_DIR)" //I "$(EZOPTION_PARSER_DIR)" //I $(PTHREAD_INCLUDE)
LIBDIRS=//LIBPATH:"..\..\..\ThirdParty\lib"
OS_SPECIFIC_FLAGS=//nologo //EHsc //D "WIN32" //D "_WINDOWS" //D "_USRDLL" //D "GRAVITY_EXPORTS" //D "_WINDLL" 
COMPILE_FLAG=//c //MD
COUTPUT_FLAG=//Fo
OUTPUT_FLAG=//OUT:
LINK_FLAGS=//DLL

OS_LIBS = "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" 
OS_SPECIFIC_LIBS = "libprotobuf.lib" "libiniparser.lib" "libzmq.lib" "pthreadVCE2.lib" "ws2_32.lib" $(OS_LIBS)

#This guy needed to be escaped, twice!  
OS_PATH_SEP=\\\\
