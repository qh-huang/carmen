include ../Makefile.conf

MODULE_NAME = "GPS"
MODULE_COMMENT = "NMEA GPS Module"

CFLAGS +=
IFLAGS += 
LFLAGS += -lparam_interface -lserial -lipc -lglobal

SOURCES = gps-io.c gps-ipc.c gps-nmea.c gps-main.c \
	  gps_nmea_interface.c gps-test.c

PUBLIC_INCLUDES = gps_nmea_messages.h gps_nmea_interface.h

PUBLIC_LIBRARIES = libgps_nmea_interface.a

PUBLIC_BINARIES = gps-nmea

TARGETS = gps-nmea libgps_nmea_interface.a gps-test 

gps-nmea:			gps-io.o gps-ipc.o gps-nmea.o gps-main.o

gps-test:			gps-test.o gps_nmea_interface.o

libgps_nmea_interface.a:	gps_nmea_interface.o

# rules

include ../Makefile.rules
