include ../Makefile.conf

SUBDIRS = orclib pioneerlib reflexlib scoutlib xr4000_base

CFLAGS += 
IFLAGS += -I../robot -I../laser
LFLAGS += -llaser -lrobot -lglobal -lserial -lgeometry -lipc -lbase_interface \
	-llaser_interface -lcamera_interface -lparam_interface 

MODULE_NAME = BASE
MODULE_COMMENT = Generic interface to all robot bases

SOURCES = base_interface.c base_test.c base_main.c 
PUBLIC_INCLUDES = base_messages.h base_interface.h base_low_level.h
PUBLIC_LIBRARIES = libbase_interface.a 

TARGETS = libbase_interface.a base_test base_main.o 

TARGETS += $(ROBOT_TYPES)
PUBLIC_BINARIES += $(ROBOT_TYPES)


# rules

libbase_interface.a:	base_interface.o

base_test:		base_test.o libbase_interface.a

scout: base_main.o scoutlib/libscout.a 

pioneer: base_main.o pioneerlib/libpioneer.a 

orc: base_main.o orclib/liborc.a 

rflex: base_main.o rflexlib/librflex.a

segway: 
	$(MAKE) -C segway_base

xr4000: 
	$(MAKE) -C xr4000_base

scoutlib/libscout.a: 
	$(MAKE) -C scoutlib 

pioneerlib/libpioneer.a: 
	$(MAKE) -C pioneerlib

orclib/liborc.a: 
	$(MAKE) -C orclib

rflexlib/librflex.a: 
	$(MAKE) -C rflexlib

include ../Makefile.rules
