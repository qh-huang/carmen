include ../Makefile.conf

#NO_GRAPHICS = 1  #dbug

MODULE_NAME = ROOMNAV
MODULE_COMMENT = Room-Based Navigator

SOURCES = roomnav.c roomnav_interface.c erni.c erni_planner.c mlm.c \
		roomnav_learner.c rlog_add_time.c

CFLAGS +=
IFLAGS 	+=
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lglobal \
		-lipc -llocalize_interface -llaser_interface \
		-lnavigator_interface -lrobot_interface -rdynamic

ifndef NO_GRAPHICS
CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lgdk_imlib -lglobal_graphics `$(GTK_CONFIG) --libs gthread`
endif

PUBLIC_INCLUDES = roomnav.h roomnav_messages.h roomnav_interface.h
PUBLIC_LIBRARIES = libroomnav_interface.a
PUBLIC_BINARIES = roomnav mlm roomnav_learner rlog_add_time

TARGETS = roomnav libroomnav_interface.a mlm roomnav_learner rlog_add_time

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif

ifndef NO_DLL
TARGETS += liberni.so
PUBLIC_LIBRARIES += liberni.so
CFLAGS += -DHAVE_DLL
IFLAGS += -I$(CARMEN_HOME)/src/navigator
endif

all:

roomnav: roomnav.o mlm.o roomnav_interface.o # dbug!

libroomnav_interface.a: roomnav_interface.o

mlm: mlm_main.o mlm.o roomnav_interface.o

roomnav_learner: roomnav_learner.o

rlog_add_time: rlog_add_time.o

ifndef NO_DLL
liberni.so: erni.o erni_planner.o roomnav_interface.o \
	$(CARMEN_HOME)/src/navigator/trajectory.o \
	$(CARMEN_HOME)/src/navigator/map_modify.o
endif

ifndef NO_DLL
liberni.so: erni.o erni_planner.o roomnav_interface.o \
	$(CARMEN_HOME)/src/navigator/trajectory.o \
	$(CARMEN_HOME)/src/navigator/map_modify.o
endif

include $(CARMEN_HOME)/src/Makefile.rules
