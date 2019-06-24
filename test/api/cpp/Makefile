TARGET 			:= run_test
 
SOURCES 		:= tests/main.cpp \
							tests/DomainDataKey_tests.cpp \
							tests/GravityDataProduct_tests.cpp \
							tests/GravityLogger_tests.cpp \
							tests/GravityNode_tests.cpp \
							tests/Utility_tests.cpp \
							tests/CommUtil_tests.cpp

CXX      = g++
CC       = gcc
CXXFLAGS = -Ofast -std=c++11

#location of test cpp files
TESTS_DIR = tests

#location of libgravity.a
GRAVITY_DIR=../../../src/api/cpp/

#location of libkeyvalue_parser.cpp
KEYVALUE_PARSER_DIR=../../../src/keyvalue_parser/

#location of proto files
MYPROTO_DIR=protobuf

#location of protobuf files
PROTOBUF_DIR=/opt/protobuf-2.5.0

PROTOC  = /opt/gravity-1.9.4-RELEASE/deps/protoc
LDFLAGS += -L$(GRAVITY_DIR) -L$(KEYVALUE_PARSER_DIR) -L$(PROTOBUF_DIR)/lib
LDLIBS  += -Wl,-Bstatic -lgravity -Wl,-Bdynamic -lzmq -lprotobuf -lkeyvalue_parser -lpthread
IDIRS   += -I$(GRAVITY_DIR) -I$(PROTOBUF_DIR)/include -I$(MYPROTO_DIR)

############## No changes below here ######################

#
# Create .obj directory to store .o files.
#
obj_dir := .obj
$(shell mkdir -p $(obj_dir) >/dev/null)

#
# Create list of object files (obj) from cpp files.
#
obj    := $(patsubst $(TESTS_DIR)/%.cpp, $(obj_dir)/%.o,$(SOURCES)) 

#
# Protobuf files
#
PROTO_SRC=$(wildcard $(MYPROTO_DIR)/*.proto)
PROTO_CODE=$(patsubst $(MYPROTO_DIR)/%.proto,$(MYPROTO_DIR)/%.pb.cc,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(MYPROTO_DIR)/%.cc,$(MYPROTO_DIR)/%.o,$(PROTO_CODE))

# 
# Create targets.
#
.PHONY: test clean libtarget 

test: libtarget $(PROTO_OBJ) $(TARGET) 
	./$(TARGET)

%.pb.cc:%.proto
	$(PROTOC) --proto_path=$(MYPROTO_DIR) --cpp_out=$(MYPROTO_DIR) $<

%.pb.o:%.pb.cc
	$(CC) -c -o $@ $< $(IDIRS)

#build target if any object files are newer than current target
#relink if library is newer than target
$(TARGET): $(obj) $(GRAVITY_DIR)/libgravity.a
	$(CXX) $(CXXFLAGS) -o $@ $(PROTO_OBJ) $^ $(IDIRS) $(LDFLAGS) $(LDLIBS) 
	
#build object files if cpp files are newer 
$(obj_dir)/%.o: $(TESTS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(IDIRS) -c $< -o $@

libtarget:
	make -C $(GRAVITY_DIR) 

clean:
	rm -rf $(TARGET) $(obj_dir) *.log $(MYPROTO_DIR)/*.pb.h $(MYPROTO_DIR)/*.pb.o

