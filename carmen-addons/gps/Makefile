include ../Makefile.conf

CFLAGS +=
IFLAGS +=
LFLAGS += -lgps_interface -lparam_interface -lglobal -lipc -lbase_interface

MODULE_NAME = "GPS"
MODULE_COMMENT = "Module for controlling GARMIN gps according to NMEA protocol"

SOURCES = gps_garmin.c gps_nmea.c gps_interface.c gps_test.c gps_estimate.c \
	gps_main.c

PUBLIC_INCLUDES = gps_interface.h gps_messages.h
PUBLIC_LIBRARIES = libgps_interface.a
PUBLIC_BINARIES = gps 

TARGETS = libgps_interface.a libgps.a gps gps_test

all:

gps:		libgps.a

libgps_interface.a:	gps_interface.o

libgps.a:  gps_garmin.o gps_nmea.o gps_interface.o gps_estimate.o gps_main.o

gps_test:  gps_test.o libgps_interface.a

# rules

include ../Makefile.rules
