include ../Makefile.conf

CFLAGS += -D_REENTRANT
IFLAGS += 
LFLAGS += -lrobot_interface -llogger_io -lcanlib -pthread -lparam_interface -lbase_interface -lglobal -lipc -lz -lm

MODULE_NAME = SEGWAY
MODULE_COMMENT = Segway RMP Base Module

SOURCES = segway.c keystick.c
PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = segway
MAN_PAGES =

TARGETS = libsegwaycore.a segway libsegway_interface.a segway_test segwaylogger keystick

# rules

all:

libsegwaycore.a:	segwaycore.o

libsegway_interface.a:	segway_interface.o

segway:			segway.o segway_ipc.o libsegwaycore.a

segway_test:		segway_test.o libsegway_interface.a

segwaylogger:		segwaylogger.o

keystick:		keystick.o

include $(CARMEN_HOME)/src/Makefile.rules
