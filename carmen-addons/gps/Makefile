###############################################################################
#
#  $Revision$
#  $Date$
#  $Author$
#  $Log$
#  Revision 1.3  2003/04/25 20:08:29  nickr
#  New GPS module from Ringo repository
#
#  Revision 1.1  2003/03/24 02:30:37  rtriebel
#  Added ringo_gps.
#
#
###############################################################################

include ../Makefile.conf

CFLAGS +=
IFLAGS += -I $(CARMEN_INC)
LFLAGS += -L $(CARMEN_LIB) -lglobal -lipc -lparam_interface
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

PUBLIC_INCLUDES = ringo_gps_interface.h ringo_gps_messages.h
PUBLIC_LIBRARIES = libringo_gps_interface.a 
PUBLIC_BINARIES = ringo_gps 

TARGETS = libringo_gps_interface.a libringo_gps.a ringo_gps ringo_gps_test

all:

libringo_gps.a: 		$(LIB_OBJS)

libringo_gps_interface.a:	gps_interface.o

ringo_gps:			$(LIB_OBJS) $(TEST_OBJS1)	#libringo_gps.a

ringo_gps_test:  gps_test.o gps_interface.o #libringo_gps_interface.a



# rules

include ../Makefile.rules
