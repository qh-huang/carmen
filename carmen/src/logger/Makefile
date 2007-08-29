include ../Makefile.conf

ifdef FOR_XAVIER
INTERFACE_LIBS = -lxavier_interface -lparam_interface -lsimulator_interface
else
INTERFACE_LIBS = -lrobot_interface -llaser_interface -lbase_interface \
	         -lsimulator_interface -lgps_nmea_interface -larm_interface \
		 -limu_interface

endif

LFLAGS += $(INTERFACE_LIBS) -llocalize_interface -lparam_interface \
	  -lglobal -lipc

MODULE_NAME = LOGGER
MODULE_COMMENT = Modules for logging, displaying, and playing back data.

SOURCES = logger.c playback_interface.c playback.c laserint_to_carmenlog.c \
	  writelog.c readlog.c log_timestamp_repair.c log_corrected_laser.c \
	  test_logfileread.c arm_dump_state.c

PUBLIC_INCLUDES = logger.h logger_messages.h playback_messages.h \
		  playback_interface.h writelog.h readlog.h
PUBLIC_LIBRARIES = libplayback_interface.a  libwritelog.a libreadlog.a
PUBLIC_BINARIES = logger playback playback_control logtool

TARGETS = libwritelog.a libreadlog.a logger libplayback_interface.a  playback \
	  laserint_to_carmenlog log_timestamp_repair log_corrected_laser \
	  test_logfileread arm_dump_state logtool

ifndef NO_GRAPHICS
SOURCES += playback_control.c
IFLAGS += `$(GTK_CONFIG) --cflags`
LFLAGS += -lglobal_graphics `$(GTK_CONFIG) --libs`
TARGETS += playback_control
endif

PUBLIC_LIBRARIES_SO = libplayback_interface.so
ifndef NO_PYTHON
TARGETS += libplayback_interface.so.1
endif

logger:			logger.o libwritelog.a

libwritelog.a : 	writelog.o

libreadlog.a : 		readlog.o

libplayback_interface.a:playback_interface.o

libplayback_interface.so.1:playback_interface.o

playback:		playback.o libreadlog.a

playback_control: 	playback_control.o playback_interface.o

laserint_to_carmenlog:	laserint_to_carmenlog.o libwritelog.a

log_timestamp_repair: 	log_timestamp_repair.o

log_corrected_laser: 	log_corrected_laser.o libwritelog.a

test_logfileread:	test_logfileread.o libreadlog.a

logtool:	        logtool.o libreadlog.a libwrite.a

arm_dump_state:		arm_dump_state.o

include ../Makefile.rules
