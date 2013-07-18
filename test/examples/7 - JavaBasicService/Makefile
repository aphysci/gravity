GRAVITY_ROOT_DIR=../../..
 
GRAVITY_JAVA_DIR=$(GRAVITY_ROOT_DIR)/src/api/java
GRAVLIB=$(GRAVITY_JAVA_DIR)/gravity.jar

JAVA_BUILD_DIR=build
SRC_DIR=src
PROTOBUF_SRC_DIR=../protobuf

PROTOC=$(PROTOBUF_INCLUDE_DIR)/protoc
CLASSPATH="$(JAVAPROTOBUF_DIR);$(GRAVITY_JAVA_DIR)/gravity.jar;build;."

JAVA_SRC=$(wildcard $(SRC_DIR)/*.java)
JAVA_CLASS=$(patsubst $(SRC_DIR)/%.java,$(JAVA_BUILD_DIR)/%.class,$(JAVA_SRC))

PROTO_SRC=$(wildcard $(PROTOBUF_SRC_DIR)/*.proto)
PROTO_JAVA=$(patsubst $(PROTOBUF_SRC_DIR)/%.proto,$(PROTOBUF_SRC_DIR)/%.java,$(PROTO_SRC))

OUTPUT_JAR=JavaBasicService.jar

SYSTEM:=$(strip $(shell uname -s))
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
        CLASSPATH="$(GRAVLIB);build;$(JAVAPROTOBUF_DIR)"
        THIRDPARTY_LIB=../../../src/keyvalue_parser/
windows: all;
else ifneq (,$(findstring Linux,$(SYSTEM)))
        CLASSPATH=$(GRAVLIB):build:$(JAVAPROTOBUF_DIR)
        THIRDPARTY_LIB="../../../src/keyvalue_parser/;$(ZMQ_LIB_DIR)"
linux: all;
else
        ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif

.PRECIOUS: %.java $(PROTO_JAVA)

$(PROTOBUF_SRC_DIR)/%.java:$(PROTOBUF_SRC_DIR)/%.proto
	@echo "Building " $<
	@$(PROTOC) --proto_path=$(PROTOBUF_SRC_DIR) --java_out=$(PROTOBUF_SRC_DIR) $<

$(JAVA_BUILD_DIR)/%.class:$(SRC_DIR)/%.java $(PROTO_JAVA)
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(CLASSPATH) -sourcepath $(SRC_DIR) -sourcepath $(PROTOBUF_SRC_DIR) $<

$(OUTPUT_JAR): $(JAVA_CLASS)
	@echo "Building Jar"
	@cd $(JAVA_BUILD_DIR); \
	jar cf ../$(OUTPUT_JAR) *.class

all: $(OUTPUT_JAR)

run: $(OUTPUT_JAR)
	./run.sh

$(JAVA_BUILD_DIR):
	@mkdir -p $(JAVA_BUILD_DIR)

clean:
	rm -rf $(JAVA_BUILD_DIR)/*.class $(OUTPUT_JAR) $(PROTO_JAVA)
