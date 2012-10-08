SYSTEM:=$(strip $(shell uname -s))
ifneq (,$(findstring MINGW32_NT,$(SYSTEM)))
    ADDITIONAL_LIBS= -lcppdb -lodbc32
else ifneq (,$(findstring Linux,$(SYSTEM)))
    ADDITIONAL_LIBS= -lcppdb -lodbc
else
ostype: ; @echo "ERROR UNKNOWN OS: " $(SYSTEM);
endif
export ADDITIONAL_LIBS
-include ../Makefile.in
