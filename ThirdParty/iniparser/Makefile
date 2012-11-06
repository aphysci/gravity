#
# iniparser Makefile
#

# Compiler settings
#CROSS= arm-angstrom-linux-gnueabi-
CXX=$(CROSS)g++
CC=$(CROSS)gcc
CFLAGS  = -O2 -fPIC -Wall -ansi -pedantic

# Ar settings to build the library
AR	    = ar
ARFLAGS = rcv

SHLD = ${CC} ${CFLAGS}
LDSHFLAGS = -shared -Wl,-Bsymbolic  -Wl,-rpath -Wl,/usr/lib -Wl,-rpath,/usr/lib
LDFLAGS = -Wl,-rpath -Wl,/usr/lib -Wl,-rpath,/usr/lib

# Set RANLIB to ranlib on systems that require it (Sun OS < 4, Mac OSX)
# RANLIB  = ranlib
RANLIB = true

RM      = rm -f


# Implicit rules

SUFFIXES = .o .obj .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)
%.obj: %.c
	@(echo "compiling $< ...")
	@cl //c //Fo$@ //MD $< //nologo

SRCS = src/iniparser.c \
	   src/dictionary.c
VSSRCS = src\iniparser.c \
	   src\dictionary.c

OBJS = $(SRCS:.c=.o)
VSOBJS = $(VSSRCS:.c=.obj)

default:	libiniparser.a libiniparser.so

libiniparser.a:	$(OBJS)
	@($(AR) $(ARFLAGS) libiniparser.a $(OBJS))
	@($(RANLIB) libiniparser.a)

libiniparser.so:	$(OBJS)
	@$(SHLD) $(LDSHFLAGS) -o $@.0 $(OBJS) $(LDFLAGS) \
		-Wl,-soname=`basename $@`.0

libiniparser.lib: $(VSOBJS)
	lib //OUT:$@ //nologo $(VSOBJS)
		
clean:
	$(RM) $(OBJS) src/*.obj *.a *.lib

veryclean:
	$(RM) $(OBJS) src/*.obj libiniparser.a libiniparser.so*
	rm -rf ./html ; mkdir html
	cd test ; $(MAKE) veryclean

docs:
	@(cd doc ; $(MAKE))
	
check:
	@(cd test ; $(MAKE))
