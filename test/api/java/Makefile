
JAVASRC_DIR=src
GRAVSRC_DIR=$(JAVASRC_DIR)/com/aphysci/gravity
PROTOGEN_DIR=$(GRAVSRC_DIR)/protobuf
JAVA_BUILD_DIR=build
LIB_NAME=gravitytest.jar
PRODGRAV_DIR=../../../src/api/java
GRAVLIB=$(PRODGRAV_DIR)/gravity.jar
PROTOC=protoc

SRC=$(wildcard $(GRAVSRC_DIR)/*java)
OBJECTS=$(patsubst $(GRAVSRC_DIR)/%.java,$(JAVA_BUILD_DIR)/%.class,$(SRC))

PROTO_SRC=$(wildcard *.proto)
PROTO_CODE=$(patsubst %PB.proto,$(PROTOGEN_DIR)/%Container.java,$(PROTO_SRC))
PROTO_OBJ=$(patsubst $(PROTOGEN_DIR)%.java,$(JAVA_BUILD_DIR)%.class,$(PROTO_CODE))

SYSTEM:=$(strip $(shell uname -s))
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
        CLASSPATH="$(GRAVLIB);build;$(JAVAPROTOBUF_DIR)"
	THIRDPARTY_LIB=../../../Win32/Release/bin
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
        CLASSPATH=$(GRAVLIB):build:$(JAVAPROTOBUF_DIR)
        THIRDPARTY_LIB=$(ZMQ_LIB_DIR)
linux: all;
else
        ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

.PRECIOUS: $(PROTO_CODE)

all: $(LIB_NAME)

$(PROTOGEN_DIR)/%Container.java:%PB.proto
	@echo $<
	@$(PROTOC) --proto_path=. --java_out=$(JAVASRC_DIR) $<

$(JAVA_BUILD_DIR)/%.class:$(PROTOGEN_DIR)/%.java
	@echo $<
	javac -d $(JAVA_BUILD_DIR) -cp $(CLASSPATH) $<

$(JAVA_BUILD_DIR)/%.class:$(GRAVSRC_DIR)/%.java
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(GRAVLIB):$(JAVAPROTOBUF_DIR):build:. -sourcepath src/java $<

$(LIB_NAME): $(PROTO_OBJ) $(OBJECTS) 
	echo $(GRAVSRC_DIR)
	echo $(SRC)
	echo $(OBJECTS)
	jar cf $@ -C $(JAVA_BUILD_DIR) .

test: $(LIB_NAME)
	java -Djava.library.path=$(PRODGRAV_DIR):$(THIRDPARTY_LIB) -cp $(CLASSPATH) com.aphysci.gravity.GravityJavaTest

clean:
	rm -rf $(JAVA_BUILD_DIR)/* *.jar $(PROTOGEN_DIR)/*
