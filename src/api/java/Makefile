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

#Wrapper Source Dirs
#Swig source dir
SWIGSRC_DIR=src/swig
#C++ Wrapper source dir
SWIGGENCPP_DIR=src/cpp
#Java wrapper source dir
SWIGGENJAVA_DIR=src/java/com/aphysci/gravity/swig

SYSTEM:=$(strip $(shell uname -s))
ifneq (,$(findstring Linux,$(SYSTEM)))
   BUILD_VARIANT_DIRECTIVE=-DLINUX
else ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
   BUILD_VARIANT_DIRECTIVE=-DWIN32
else
   BUILD_VARIANT_DIRECTIVE=
endif

all: libgravity_wrap gravity.jar

# Generate source from SWIG files and update AUTO_SRCS to include the generated .java files
# The change to autosrcs.mk will cause the Makefile to be reloaded so the updated AUTO_SRCS will be seen.
swig: $(SWIGSRC_DIR)/gravity.i $(wildcard $(SWIGSRC_DIR)/*.i)
	@echo "Executing swig"
	@swig $(BUILD_VARIANT_DIRECTIVE) -c++ -o $(SWIGGENCPP_DIR)/gravity_wrap.cpp -outdir $(SWIGGENJAVA_DIR) -package com.aphysci.gravity.swig -java $<
	@touch swig

gravity.jar: swig
	make -f java_makefile

libgravity_wrap: swig
	make -f cpp_makefile
	
clean:
	rm -rf $(SWIGGENJAVA_DIR)/* $(SWIGGENCPP_DIR)/gravity_wrap.* ./swig
	make -f cpp_makefile clean
	make -f java_makefile clean
