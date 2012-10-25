#CMAKE_EXISTS=$(shell command -v cmake>/dev/null 2>&1)

cppdb:
	@-cd cppdb-trunk; \
	cmake . -G"MSYS Makefiles" #Ignoring errors if we don't have cmake.  CHANGE THIS if you aren't using MinGW
	@cd cppdb-trunk; \
	make -s

iniparser:
	cd iniparser; \
	make -s
	
zmq:
	cd zeromq-3.2.1; \
	./configure; \
	make -s

protobuf:
	cd protobuf-2.4.1; \
	./configure; \
	make -s
	
all: cppdb iniparser zmq protobuf
