###############################################################################
#
#  $Revision$
#  $Date$
#  $Author$
#  $Log$
#  Revision 1.4  2003/05/22 14:29:38  nickr
#  Started modifying gps.
#
#  Revision 1.3  2003/04/25 20:08:29  nickr
#  New GPS module 
#
#  Revision 1.1  2003/03/24 02:30:37  rtriebel
#  Added gps.
#
#
###############################################################################

include $(CARMEN_HOME)/src/Makefile.conf

CFLAGS +=
LFLAGS += -lglobal -lipc -lparam_interface
#LFLAGS += -lgps_interface -lparam_interface -lglobal -lipc -lbase_interface \
#	  -lcompass_interface

MODULE_NAME = "GPS"
MODULE_COMMENT = "Module for controlling GARMIN gps according to NMEA protocol"

LIB_SRCS =	gps_garmin.c gps_nmea.c gps_interface.c gps_estimate.c
LIB_OBJS =	$(LIB_SRCS:.c=.o)

TEST_SRCS1 =	gps_main.c
TEST_OBJS1 = 	$(TEST_SRCS1:.c=.o)

TEST_SRCS2 =	gps_test.c
TEST_OBJS2 = 	$(TEST_SRCS1:.c=.o)

SOURCES = $(LIB_SRCS) $(TEST_SRCS1) $(TEST_SRCS2)

#gps_garmin.c gps_nmea.c gps_interface.c gps_test.c gps_estimate.c \
#	gps_main.c

PUBLIC_INCLUDES = gps_interface.h gps_messages.h
PUBLIC_LIBRARIES = libgps_interface.a 
PUBLIC_BINARIES = gps 

TARGETS = libgps_interface.a libgps.a gps gps_test

all:

libgps.a: 		$(LIB_OBJS)

libgps_interface.a:	gps_interface.o

gps:			$(LIB_OBJS) $(TEST_OBJS1)	#libgps.a

gps_test:  gps_test.o gps_interface.o #libgps_interface.a



# rules

include $(CARMEN_HOME)/src/Makefile.rules
