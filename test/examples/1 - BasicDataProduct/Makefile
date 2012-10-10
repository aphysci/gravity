CC=g++
THIRDPARTY_DIR=../../../ThirdParty
GRAVLIB_DIR=../../../src/api/cpp
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
PROG_SUB=BasicDataProductSubscriber
PROG_PUB=BasicDataProductPublisher

#OS Specific
SYSTEM:=$(strip $(shell uname -s))
#SYSTEM:=LINUX
#SYSTEM=MINGW32_NT-6.1
#WARNING: DON'T PUT SPACES OR QUOTES IN COMPARISON STRING!!!
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
	OS_SPECIFIC_LIBS = -Wl,-Bdynamic -lzmq -lwsock32 -lpthread
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
	OS_SPECIFIC_LIBS = -lzmq -Wl,-Bdynamic -lrt
linux: all;
else
ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

INCLUDES=-I$(GRAVLIB_DIR) -I$(PROTOBUF_DIR)/src
CFLAGS=-L$(GRAVLIB_DIR) -L$(THIRDPARTY_DIR)/lib
LIBS=-Wl,-Bstatic -lgravity -lprotobuf -liniparser $(OS_SPECIFIC_LIBS)

all: $(PROG_SUB) $(PROG_PUB) 

%.o:%.cpp
	$(CC) -c -o $@ $< $(INCLUDES)

$(PROG_SUB): $(PROG_SUB).o $(GRAVLIB_DIR)/libgravity.a
	$(CC) -o $@ $< $(CFLAGS) $(LIBS)

$(PROG_PUB): $(PROG_PUB).o $(GRAVLIB_DIR)/libgravity.a
	$(CC) -o $@ $< $(CFLAGS) $(LIBS)

clean:
	@rm -rf *.o $(MYPROTO_DIR)/*.o $(MYPROTO_DIR)/*.h $(MYPROTO_DIR)/*.cc $(PROG_NAME)
