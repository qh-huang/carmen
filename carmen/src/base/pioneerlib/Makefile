include ../../Makefile.conf

ifndef CYGWIN
CFLAGS += -fPIC
endif
IFLAGS +=
LFLAGS += -lglobal -lserial -lipc -lparam_interface

MODULE_NAME = "PIONEER"
MODULE_COMMENT = "Program for controlling Pioneer robot"

SOURCES = pioneer_lib2.c
PUBLIC_INCLUDES =  
PUBLIC_LIBRARIES = libpioneer.a 
PUBLIC_BINARIES = pioneer 

TARGETS = libpioneer.a pioneer_test

libpioneer.a: pioneer_lib.o

pioneer_test: pioneer_test.o

include ../../Makefile.rules



