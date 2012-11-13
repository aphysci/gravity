JAVASRC_DIR=.
JAVA_BUILD_DIR=build
LIB_NAME=MATLABGravitySubscriber.jar
CLASSPATH=../java/gravity.jar;../../../ThirdParty/guava-13.0.1/guava-13.0.1.jar
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
