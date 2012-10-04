
THIRDPARTY_DIR=../../../ThirdParty
ZMQ_DIR=$(THIRDPARTY_DIR)/zeromq-3.2.0
PROTOBUF_DIR=$(THIRDPARTY_DIR)/protobuf-2.4.1
PROTO_DIR=../protobufs
JAVASRC_DIR=src
GRAVSRC_DIR=$(JAVASRC_DIR)/com/aphysci/gravity
JAVA_BUILD_DIR=build
LIB_NAME=gravitytest.jar
PRODGRAV_DIR=../../../src/api/java
GRAVLIB=$(PRODGRAV_DIR)/gravity.jar

SRC=$(wildcard $(GRAVSRC_DIR)/*java)
OBJECTS=$(patsubst $(GRAVSRC_DIR)/%.java,$(JAVA_BUILD_DIR)/%.class,$(SRC))

SYSTEM:=$(strip $(shell uname -s))
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
	CLASSPATH="$(GRAVLIB);build;$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar"
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
	CLASSPATH=$(GRAVLIB):build:$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar
linux: all;
else
        ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

all: test

$(JAVA_BUILD_DIR)/%.class:$(GRAVSRC_DIR)/%.java
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(GRAVLIB):$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar:build:. -sourcepath src/java $<

$(LIB_NAME): $(OBJECTS)
	echo $(GRAVSRC_DIR)
	echo $(SRC)
	echo $(OBJECTS)
	jar cf $@ -C $(JAVA_BUILD_DIR) .

test: $(LIB_NAME)
	java -Djava.library.path=$(PRODGRAV_DIR):$(ZMQ_DIR)/src/.libs -cp $(CLASSPATH) com.aphysci.gravity.GravityJavaTest

clean:
	rm -rf $(JAVA_BUILD_DIR)/* *.jar 
