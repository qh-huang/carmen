include ../Makefile.conf

ifdef FOR_XAVIER
INTERFACE_LIBS = 	-lxavier_interface -lparam_interface -lsimulator_interface
else
INTERFACE_LIBS = 	-lrobot_interface -llaser_interface -lbase_interface -lsimulator_interface
endif

LFLAGS += $(INTERFACE_LIBS) -llocalize_interface -lparam_interface \
	-lglobal -lipc

MODULE_NAME = LOGGER
MODULE_COMMENT = Modules for logging, displaying, and playing back data.

SOURCES = logger.c playback_interface.c playback.c laserint_to_carmenlog.c \
	logger_io.c log_timestamp_repair.c log_corrected_laser.c

PUBLIC_INCLUDES = logger.h logger_messages.h playback_messages.h \
	playback_interface.h logger_io.h 
PUBLIC_LIBRARIES = libplayback_interface.a liblogger_io.a
PUBLIC_BINARIES = logger playback playback_control

TARGETS = liblogger_io.a logger libplayback_interface.a playback \
	laserint_to_carmenlog log_timestamp_repair log_corrected_laser

ifndef NO_GRAPHICS
SOURCES += playback_control.c
IFLAGS += `$(GTK_CONFIG) --cflags`
LFLAGS += `$(GTK_CONFIG) --libs`
TARGETS += playback_control
endif

logger:	logger.o liblogger_io.a

liblogger_io.a : logger_io.o

libplayback_interface.a:playback_interface.o

playback:		playback.o

playback_control: 	playback_control.o playback_interface.o

laserint_to_carmenlog:	laserint_to_carmenlog.o liblogger_io.a

log_timestamp_repair: log_timestamp_repair.o

log_corrected_laser: log_corrected_laser.o liblogger_io.a

# rules

include ../Makefile.rules
