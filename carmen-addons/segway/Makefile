include ../Makefile.conf

CFLAGS += -D_REENTRANT
IFLAGS += 
LFLAGS += -lrobot_interface -llogger_io -lcanlib -pthread -lparam_interface -lbase_interface -lglobal -lipc -llaser_interface -lamtec_interface -lz -lm

MODULE_NAME = SEGWAY
MODULE_COMMENT = Segway RMP Base Module

SOURCES = segwaycore.c segway_interface.c segwaylogger.c segway_test.c segway.c keystick.c segwayplayback.c segwayplayback_interface.c
PUBLIC_INCLUDES = segway_messages.h segway_interface.h segwayplayback_message.h segwayplayback_interface.h
PUBLIC_LIBRARIES = libsegway_interface.a libsegwayplayback_interface.a
PUBLIC_BINARIES = segway
MAN_PAGES =

TARGETS = libsegwaycore.a segway libsegway_interface.a segway_test segwaylogger segwayplayback keystick

# rules

all:

libsegwaycore.a:	segwaycore.o

libsegway_interface.a:	segway_interface.o

segway:			segway.o segway_ipc.o libsegwaycore.a

segway_test:		segway_test.o libsegway_interface.a

segwaylogger:		segwaylogger.o libsegway_interface.a

keystick:		keystick.o

libsegwayplayback_interface.a:segwayplayback_interface.o

segwayplayback:segwayplayback.o

segwayplayback_control:segwayplayback_control.o segwayplayback_interface.o

include $(CARMEN_HOME)/src/Makefile.rules
