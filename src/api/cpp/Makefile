#CROSS= arm-angstrom-linux-gnueabi-
CXX=$(CROSS)g++
CC=$(CROSS)gcc
LINK=$(CROSS)g++

#Locations
MYPROTO_DIR=../protobufs
MY_PROTOBUF_BUILD_DIR=./protobuf
THIRDPARTY_DIR=../../../ThirdParty

#Libraries
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
PROTOBUF_INCLUDE_DIR=$(PROTOBUF_DIR)/src
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.1
ZMQ_INCLUDE_DIR=$(ZMQ_DIR)/include
EZOPTION_PARSER_DIR=$(THIRDPARTY_DIR)/EZOptionParser
PTHREAD_INCLUDE=$(THIRDPARTY_DIR)/pthread/include
KEYVALUE_PARSER_INCLUDE_DIR=../../keyvalue_parser

#Tools
PROTOC=$(THIRDPARTY_DIR)/bin/protoc

#Compiler Flags
INCLUDES=-I$(ZMQ_INCLUDE_DIR) -I$(PROTOBUF_INCLUDE_DIR)  -I$(EZOPTION_PARSER_DIR) -I$(KEYVALUE_PARSER_INCLUDE_DIR)
LIBDIRS=-L$(THIRDPARTY_DIR)/lib -L$(KEYVALUE_PARSER_INCLUDE_DIR)
LIBS=$(OS_SPECIFIC_LIBS)
CFLAGS=$(INCLUDES) $(OS_SPECIFIC_FLAGS)
COMPILE_FLAG=-c
COUTPUT_FLAG=-o
OUTPUT_FLAG=-o
LINK_FLAGS=-shared

#Naming
LIB_NAME=libgravity
OBJ_EXT=o
OS_PATH_SEP=/

#OS Specific
SYSTEM:=$(strip $(shell uname -s))
#SYSTEM:=LINUX
#SYSTEM=MINGW32_NT-6.1
#WARNING: DON'T PUT SPACES OR QUOTES IN COMPARISON STRING!!!
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
#MinGW
	# LIB_EXT = dll
	#Not including rt and zmq
	# OS_SPECIFIC_LIBS = -Wl,-Bstatic -lprotobuf -Wl,-Bdynamic -lzmq -lpthread -lwsock32
# windows: all;
#Visual Studio
	-include makefile.vs
vs: all
else ifneq (,$(findstring Linux,$(SYSTEM)))
	LIB_EXT = so
	OS_SPECIFIC_LIBS = -lprotobuf -lzmq -lkeyvalue_parser
	OS_SPECIFIC_FLAGS = -fpic
linux: all;
else
ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

#Source/Object/Dependancies
SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.$(OBJ_EXT),$(SRC))
DEPS=$(wildcard *.h $(MY_PROTOBUF_BUILD_DIR)/*.h)

#Protobuf Source/Generated c++/Objects
PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,$(MY_PROTOBUF_BUILD_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(MY_PROTOBUF_BUILD_DIR)/%.cc,$(MY_PROTOBUF_BUILD_DIR)$(OS_PATH_SEP)%.$(OBJ_EXT),$(PROTO_CODE))

.PRECIOUS: $(MY_PROTOBUF_BUILD_DIR)/%.pb.cc

#Just in case
$(shell mkdir protobuf -p)

all: $(LIB_NAME).a $(LIB_NAME).$(LIB_EXT)

#it looks like make automatically deletes these guys after building.  
$(MY_PROTOBUF_BUILD_DIR)/%.pb.cc:$(MYPROTO_DIR)/%.proto
	@echo protoc $<
	@$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=$(MY_PROTOBUF_BUILD_DIR) $<

$(MY_PROTOBUF_BUILD_DIR)$(OS_PATH_SEP)%.pb.$(OBJ_EXT):$(MY_PROTOBUF_BUILD_DIR)/%.pb.cc
	@echo $(CC) proto build $<
	@$(CC) $(COUTPUT_FLAG)$@ $(COMPILE_FLAG) $(CFLAGS) $<

%.$(OBJ_EXT):%.cpp $(DEPS)
	@echo $(CC) $<
	@$(CC) $(COUTPUT_FLAG)$@ $(COMPILE_FLAG) $(CFLAGS) $<

$(LIB_NAME).a: $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).a  
	@ar rcs $(LIB_NAME).a $(OBJECTS) $(PROTO_OBJ)

$(LIB_NAME).$(LIB_EXT): $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).$(LIB_EXT)
	$(LINK) $(LINK_FLAGS) $(LIBDIRS) $(OUTPUT_FLAG)$@ $^ $(LIBS)

clean:
	rm -rf *.a *.$(LIB_EXT) *.$(OBJ_EXT) $(MY_PROTOBUF_BUILD_DIR)/*.*
