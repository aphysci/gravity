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

#Complier Commands
YACC= yacc
LEX= lex
CC= $(CROSS)gcc
LD= $(CROSS)gcc
AR= $(CROSS)ar

#Compiler Flags
CFLAGS+= -O3 -fPIC
INCLUDE+= -I.
CFLAGS+= $(INCLUDE) -U_FORTIFY_SOURCE
LDFLAGS+= -L.
OBJ_DIR=build

PROG=keyvalue_parser
LIB=libkeyvalue_parser.a
SOLIB=libkeyvalue_parser.so

#Sources
SRCS = \
	keyvalue_main.c

LIB_SRCS = \
	y.tab.c \
	lex.yy.c \
	keyvalue_parser.c

OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))
LIB_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))

#For Clean
OBJECTS = $(OBJS) $(LIB_OBJS)

MAKE_INC = $(SRCS:%.c=$(OBJ_DIR)/.deps/%.P)

.PHONY: all clean

all: $(LIB) $(SOLIB) $(PROG)

y.tab.c: keyvalue.y
	$(YACC) -dt keyvalue.y -o y.tab.c

lex.yy.c: keyvalue.l
	$(LEX) -o lex.yy.c keyvalue.l

define cc-command
	@mkdir -p $(@D)
	@$(CC) -c $(CFLAGS) -MD -MF $(OBJ_DIR)/$*.d -o $@ $<
	@echo $(CC) $<
	@mkdir -p $(OBJ_DIR)/.deps/$(*D); \
	cp $(OBJ_DIR)/$*.d $(OBJ_DIR)/.deps/$*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
            -e '/^$$/ d' -e 's/$$/ :/' < $(OBJ_DIR)/$*.d >> $(OBJ_DIR)/.deps/$*.P; \
	rm -f $(OBJ_DIR)/$*.d
endef

$(OBJ_DIR)/%.o:%.c
	$(cc-command)

-include $(MAKE_INC)

$(PROG): $(OBJS) keyvalue.y keyvalue.l
	$(LD) -static -o $@ $(OBJS) $(LDFLAGS) -lkeyvalue_parser

$(LIB): $(LIB_OBJS) keyvalue.y keyvalue.l
	$(AR) rcs $@ $(LIB_OBJS)

$(SOLIB): $(LIB_OBJS) keyvalue.y keyvalue.l
	$(LD) -shared -o $@ $(LIB_OBJS)

clean:
	-rm -rf $(OBJ_DIR) $(PROG) $(LIB) $(SOLIB) ./Map*.txt y.tab.* lex.yy.c
