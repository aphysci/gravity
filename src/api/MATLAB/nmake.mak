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

JAVASRC_DIR=.
JAVA_BUILD_DIR=build
LIB_NAME=MATLABGravitySubscriber.jar

CLASSPATH=../java/gravity.jar;$(GUAVAJAR_DIR)
SOURCES=MATLABGravitySubscriber.java
CLASSES=MATLABGravitySubscriber.class

.SUFFIXES: .java

all: pre $(LIB_NAME)

pre:
	@mkdir $(JAVA_BUILD_DIR)

$(LIB_NAME): $(CLASSES)
	@jar cf $(LIB_NAME) -C $(JAVA_BUILD_DIR) $(JAVASRC_DIR)

.java.class:
	@echo $<
	@javac -d $(JAVA_BUILD_DIR) -cp $(CLASSPATH) $(JAVASRC_DIR)/$<

clean:
	rm -rf $(JAVA_BUILD_DIR) *.jar
