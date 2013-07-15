#Tools
CC=cl
LINK=link

#Naming
LIB_EXT=dll
OBJ_EXT=obj

#Compiler Flags
#(NOTE: if I quoted all of the include strings I might be able to use the / separator)
INCLUDES=/I$(GRAV_HOME)\\include /I$(KEYVALUE_PARSER_DIR)\\src /I"$(JAVAINCLUDE_DIR)" /I"$(JAVAINCLUDE_DIR)\\win32"
LIBDIRS=/LIBPATH:"..\..\..\ThirdParty\lib" /LIBPATH:"$(GRAVITY_LIB_PATH)"
#OS_SPECIFIC_FLAGS=/nologo /EHsc /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "GRAVITY_EXPORTS" /D "_WINDLL" /D "_JNI_IMPLEMENTATION_"
OS_SPECIFIC_FLAGS=/nologo /EHsc
COUTPUT_FLAG=/Fo
OUTPUT_FLAG=/OUT:
LINK_FLAGS=/DLL

#OS_SPECIFIC_LIBS = "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" 
ifeq ($(GRAVITY_CONFIG), RELEASE)
	MSVS_LIBS = "gravity.lib" "libprotobuf.lib" $(OS_LIBS)
	COMPILE_FLAG=/c /MD /O2
	LIB_NAME = libgravity_wrap
else
	MSVS_LIBS = "gravity_d.lib" "libprotobuf_d.lib" $(OS_LIBS)
	COMPILE_FLAG=/c /MDd
	LIB_NAME = libgravity_wrap_d
endif
COMPILER_SPECIFIC_LIBS=$(MSVS_LIBS) $(OS_SPECIFIC_LIBS)

#This guy needed to be escaped, twice!  
OS_PATH_SEP=/
