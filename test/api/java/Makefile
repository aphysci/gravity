
THIRDPARTY_DIR=../../../ThirdParty
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
	java -Djava.library.path=$(PRODGRAV_DIR) -cp $(GRAVLIB):build:$(PROTOBUF_DIR)/java/target/protobuf-java-2.4.1.jar com.aphysci.gravity.GravityJavaTest

clean:
	rm -rf $(JAVA_BUILD_DIR)/* *.jar 
