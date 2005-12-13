include $(CARMEN_HOME)/src/Makefile.conf

LFLAGS +=  -lrobot_interface  -lglobal -lipc  -lparam_interface


MODULE_NAME = MAPTOOLS
MODULE_COMMENT = Tools for mapping, logfile conversion, etc.

SOURCES = movement.c clfconvert.c test_movement.c 

PUBLIC_INCLUDES =  movement.h clfconvert.h
PUBLIC_LIBRARIES = libmovement.a libclfconvert.a
PUBLIC_BINARIES = 

TARGETS =  libmovement.a libclfconvert.a test_movement 

all:

libmovement.a: movement.o

libclfconvert.a: clfconvert.o

test_movement: movement.o test_movement.o


include  $(CARMEN_HOME)/src/Makefile.rules

