include ../Makefile.conf

CFLAGS += 
IFLAGS += -I../robot -I../laser
LFLAGS += -rdynamic
LFLAGS += -llaser -lrobot -lglobal -lserial -lgeometry -lipc -lbase_interface \
	-llaser_interface -lparam_interface -ldl

MODULE_NAME = BASE
MODULE_COMMENT = Generic interface to all robot bases

SOURCES = base_interface.c base_test.c base_services.c base_services2.c \
	base_main.c 
PUBLIC_INCLUDES = base_messages.h base_interface.h base_low_level.h
PUBLIC_LIBRARIES = libbase_interface.a 
PUBLIC_BINARIES = base_services

TARGETS = libbase_interface.a libbase.a base_test base_services base_main.o \
	base_services2

# rules

all:

libbase_interface.a:	base_interface.o

base_test:		base_test.o libbase_interface.a

base_services: 		base_services.o

base_services2:	 	base_services2.o 

include ../Makefile.rules
