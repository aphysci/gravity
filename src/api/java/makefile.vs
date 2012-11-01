#Tools
CC=cl
LINK=link

#Naming
LIB_EXT=dll
OBJ_EXT=obj

#Compiler Flags
#(NOTE: if I quoted all of the include strings I might be able to use the / separator)
INCLUDES=//I$(GRAVLIB_DIR) //I$(THIRDPARTY_DIR)\\include //I$(INIPARSER_DIR)\\src //I$(JAVAINCLUDE_DIR) //I$(JAVAINCLUDE_DIR)\\win32
LIBDIRS=//LIBPATH:"..\..\..\ThirdParty\lib"
OS_SPECIFIC_FLAGS=//nologo //EHsc //D "WIN32" //D "_WINDOWS" //D "_USRDLL" //D "GRAVITY_EXPORTS" //D "_WINDLL" //D "_JNI_IMPLEMENTATION_"
COMPILE_FLAG=//c //MD
COUTPUT_FLAG=//Fo
OUTPUT_FLAG=//OUT:
LINK_FLAGS=//DLL

OS_SPECIFIC_LIBS = "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" 
MSVS_LIBS = "libprotobuf.lib" "libiniparser.lib" "libzmq.lib" "pthreadVCE2.lib" "ws2_32.lib" $(OS_LIBS)
COMPLIER_SPECIFIC_LIBS=$(MSVS_LIBS) $(OS_SPECIFIC_LIBS)

#This guy needed to be escaped, twice!  
OS_PATH_SEP=\\\\
