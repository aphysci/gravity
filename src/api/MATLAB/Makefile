JAVASRC_DIR=.
JAVA_BUILD_DIR=build
LIB_NAME=MATLABGravitySubscriber.jar

CLASSPATH=../java/gravity.jar:$(GUAVAJAR_DIR)
SOURCES=MATLABGravitySubscriber.java
CLASSES=$(SOURCES:.java=.class)

all: $(LIB_NAME)

$(LIB_NAME): $(CLASSES)

%.class: %.java
	@echo $<
	@mkdir -p $(JAVA_BUILD_DIR)
	@javac -d $(JAVA_BUILD_DIR) -cp $(CLASSPATH) $(JAVASRC_DIR)/$<
	@jar cf $(LIB_NAME) -C $(JAVA_BUILD_DIR) $(JAVASRC_DIR)

clean:
	rm -rf $(JAVA_BUILD_DIR) *.jar 
