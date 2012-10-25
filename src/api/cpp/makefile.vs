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
