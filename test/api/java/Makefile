#** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#**
#** Gravity is free software; you can redistribute it and/or modify
#** it under the terms of the GNU Lesser General Public License as published by
#** the Free Software Foundation; either version 3 of the License, or
#** (at your option) any later version.
#**
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#** GNU Lesser General Public License for more details.
#**
#** You should have received a copy of the GNU Lesser General Public
#** License along with this program;
#** If not, see <http://www.gnu.org/licenses/>.
#**


JAVASRC_DIR=src
GRAVSRC_DIR=$(JAVASRC_DIR)/com/aphysci/gravity
PROTOGEN_DIR=$(GRAVSRC_DIR)/protobuf
JAVA_BUILD_DIR=build
LIB_NAME=gravitytest.jar
PRODGRAV_DIR=../../../src/api/java
GRAVLIB=$(PRODGRAV_DIR)/gravity.jar
PROTOC=$(PROTOBUF_INCLUDE_DIR)/protoc

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
