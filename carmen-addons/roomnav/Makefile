include ../Makefile.conf

#NO_GRAPHICS = 1  #dbug
NO_DLL = 1

MODULE_NAME = ROOMNAV
MODULE_COMMENT = Room-Based Navigator

SOURCES = roomnav.c roomnav_interface.c mlm.c \
	roomnav_learner.c rlog_add_time.c \ #erni.c erni_planner.c erni_map_modify.c \
	roomnav_guidance.c

CFLAGS +=
IFLAGS += -I$(CARMEN_HOME)/src/navigator
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lglobal \
		-lipc -llocalize_interface -lgeometry -llaser_interface \
		-lant_lite -lnavigator_interface -lrobot_interface -lbase_interface 

ifndef NO_READLINE
LFLAGS += -lreadline
ifndef FOUND_SUSE 
LFLAGS += -ltermcap
endif
endif

ifndef NO_DLL
LFLAGS	+=	-rdynamic
endif

ifndef NO_GRAPHICS
CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lglobal_graphics -lgdk_imlib `$(GTK_CONFIG) --libs gthread`
endif

PUBLIC_INCLUDES = roomnav.h roomnav_messages.h roomnav_interface.h
PUBLIC_LIBRARIES = libroomnav_interface.a
PUBLIC_BINARIES = roomnav mlm roomnav_learner rlog_add_time # erni

TARGETS = roomnav libroomnav_interface.a mlm roomnav_learner rlog_add_time roomnav_guidance #erni

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif

ifndef NO_DLL
TARGETS += liberni.so
PUBLIC_LIBRARIES += liberni.so
CFLAGS += -DHAVE_DLL
endif

all:

roomnav: roomnav.o mlm.o roomnav_interface.o # dbug!

libroomnav_interface.a: roomnav_interface.o

mlm: mlm_main.o mlm.o roomnav_interface.o

roomnav_learner: roomnav_learner.o

rlog_add_time: rlog_add_time.o

erni: erni.o erni_planner.o erni_map_modify.o roomnav_interface.o \
	$(CARMEN_HOME)/src/navigator/navigator.o \
	$(CARMEN_HOME)/src/navigator/navigator_ipc.o \
	$(CARMEN_HOME)/src/navigator/trajectory.o

ifndef NO_DLL
liberni.so: erni.o erni_planner.o erni_map_modify.o roomnav_interface.o \
	$(CARMEN_HOME)/src/navigator/trajectory.o
endif

roomnav_guidance: roomnav_guidance.o roomnav_interface.o

include $(CARMEN_HOME)/src/Makefile.rules
