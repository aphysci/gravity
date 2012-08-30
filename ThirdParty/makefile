#CMAKE_EXISTS=$(shell command -v cmake>/dev/null 2>&1)

cppdb:
	@-cd cppdb-trunk; \
	cmake . #Ignoring errors if we don't have cmake
	@cd cppdb-trunk; \
	make -s

iniparser:
	cd iniparser; \
	make -s

#TODO: other libraries

all: cppdb iniparser
