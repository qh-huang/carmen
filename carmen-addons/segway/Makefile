include ../Makefile.conf

CFLAGS += -D_REENTRANT
IFLAGS += 
LFLAGS += -lcanlib -pthread -lparam_interface -lbase_interface -lglobal -lipc -lm

MODULE_NAME = SEGWAY
MODULE_COMMENT = Segway RMP Base Module

SOURCES = segway.c keystick.c
PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = segway
MAN_PAGES =

TARGETS = libsegwaycore.a segway keystick

# rules

all:

libsegwaycore.a:	segwaycore.o

segway:			segway.o libsegwaycore.a

keystick:		keystick.o

include $(CARMEN_HOME)/src/Makefile.rules
