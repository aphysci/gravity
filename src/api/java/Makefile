#Wrapper Source Dirs
#Swig source dir
SWIGSRC_DIR=src/swig
#C++ Wrapper source dir
SWIGGENCPP_DIR=src/cpp
#Java wrapper source dir
SWIGGENJAVA_DIR=src/java/com/aphysci/gravity/swig

all: $(JAR_NAME) libgravity_wrap

# Generate source from SWIG files and update AUTO_SRCS to include the generated .java files
# The change to autosrcs.mk will cause the Makefile to be reloaded so the updated AUTO_SRCS will be seen.
swig: $(SWIGSRC_DIR)/gravity.i $(wildcard $(SWIGSRC_DIR)/*.i)
	@echo "Executing swig"
	@swig -o $(SWIGGENCPP_DIR)/gravity_wrap.cpp -outdir $(SWIGGENJAVA_DIR) -package com.aphysci.gravity.swig -java $<
	@touch swig
#	echo "AUTO_SRCS=`echo $(SWIGGENJAVA_DIR)/*.java $(GRAVJAVASRC_DIR)/*.java`" > autosrcs.mk
#	echo "CPP_SRC=`echo $(SWIGGENCPP_DIR)/*.cpp`" >> autosrcs.mk

gravity.jar: swig
	make -f java_makefile

libgravity_wrap: swig
	make -f cpp_makefile
	
clean:
	rm -rf $(SWIGGENJAVA_DIR)/* $(SWIGGENCPP_DIR)/gravity_wrap.* ./swig
	make -f cpp_makefile clean
	make -f java_makefile clean
