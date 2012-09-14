#Locations
MYPROTO_DIR=../protobufs
MY_PROTOBUF_BUILD_DIR=./protobuf
THIRDPARTY_DIR=../../../ThirdParty

#Libraries
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
PROTOBUF_INCLUDE_DIR=$(PROTOBUF_DIR)/src
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
ZMQ_INCLUDE_DIR=$(ZMQ_DIR)/include
INIPARSE_DIR=$(THIRDPARTY_DIR)/iniparser
INIPARSE_INCLUDE_DIR=$(THIRDPARTY_DIR)/iniparser/src
EZOPTION_PARSER_DIR=$(THIRDPARTY_DIR)/EZOptionParser

#Tools
PROTOC=$(PROTOBUF_DIR)/src/protoc
CC=g++

#Naming
LIB_NAME=libgravity

#OS Specific
SYSTEM:=$(strip $(shell uname -s))
#SYSTEM:=LINUX
#SYSTEM=MINGW32_NT-6.1
#WARNING: DON'T PUT SPACES OR QUOTES IN COMPARISON STRING!!!
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
	LIB_EXT = dll
	#Not including rt and zmq
	OS_SPECIFIC_LIBS = -Wl,-Bstatic -lprotobuf -liniparser -Wl,-Bdynamic -lzmq -lpthread -lwsock32
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
	LIB_EXT = so
	OS_SPECIFIC_LIBS = -Wl,-Bstatic -lzmq -lprotobuf -liniparser -lrt -Wl,-Bdynamic
	OS_SPECIFIC_FLAGS = -fpic
linux: all;
else
ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

#Compiler Flags
INCLUDES=-I$(ZMQ_INCLUDE_DIR) -I$(PROTOBUF_INCLUDE_DIR) -I$(INIPARSE_INCLUDE_DIR) -I$(EZOPTION_PARSER_DIR)
LIBDIRS=-L$(PROTOBUF_DIR)/src/.libs -L$(ZMQ_DIR)/src/.libs -L$(INIPARSE_DIR) 
LIBS=$(OS_SPECIFIC_LIBS)
CFLAGS=$(INCLUDES) $(OS_SPECIFIC_FLAGS)

#Source/Object/Dependancies
SRC=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SRC))
DEPS=$(wildcard *.h $(MY_PROTOBUF_BUILD_DIR)/*.h)

#Protobuf Source/Generated c++/Objects
PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,$(MY_PROTOBUF_BUILD_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(MY_PROTOBUF_BUILD_DIR)/%.cc,$(MY_PROTOBUF_BUILD_DIR)/%.o,$(PROTO_CODE))

#.PRECIOUS: %.pb.cc

#Just in case
$(shell mkdir protobuf -p)

all: $(LIB_NAME).a $(LIB_NAME).$(LIB_EXT)

#it looks like make automatically deletes these guys after building.  
$(MY_PROTOBUF_BUILD_DIR)/%.pb.cc:$(MYPROTO_DIR)/%.proto
	@echo protoc $<
	@$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=$(MY_PROTOBUF_BUILD_DIR) $<

$(MY_PROTOBUF_BUILD_DIR)/%.pb.o:$(MY_PROTOBUF_BUILD_DIR)/%.pb.cc
	@echo $(CC) proto build $<
	@$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cpp $(DEPS)
	@echo $(CC) $<
	@$(CC) -c -o $@ $< $(CFLAGS)

$(LIB_NAME).a: $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).a  
	@ar rcs $(LIB_NAME).a $(OBJECTS) $(PROTO_OBJ)

$(LIB_NAME).$(LIB_EXT): $(PROTO_OBJ) $(OBJECTS)
	@echo building $(LIB_NAME).$(LIB_EXT)
#	@$(CC) -shared -o $@ $(OBJECTS) $(PROTO_OBJ)
	@$(CC) -shared -o $@ $^ $(LIBDIRS) $(LIBS)

clean:
	rm -rf *.a *.$(LIB_EXT) *.o $(MY_PROTOBUF_BUILD_DIR)/*.*
