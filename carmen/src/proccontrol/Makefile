include ../Makefile.conf

CFLAGS +=
IFLAGS += 
LFLAGS += -lserial -lglobal -lipc

MODULE_NAME = PROCCONTROL
MODULE_COMMENT = Process controller for CARMEN

SOURCES = 		watchdog.c proccontrol_interface.c proccontrol.c \
			proccontrol_ipc.c proccontrol_setmodule.c \
			proccontrol_setgroup.c proccontrol_viewoutput.c

PUBLIC_INCLUDES = 	proccontrol_messages.h proccontrol_interface.h
PUBLIC_LIBRARIES = 	proccontrol_interface.a
PUBLIC_BINARIES = 	proccontrol proccontrol_setmodule \
	                proccontrol_setgroup proccontrol_viewoutput
MAN_PAGES =

TARGETS = 		watchdog libproccontrol_interface.a proccontrol \
			proccontrol_setmodule proccontrol_setgroup \
	 		proccontrol_viewoutput

# rules

watchdog:			watchdog.o

libproccontrol_interface.a:	proccontrol_interface.o

proccontrol:			proccontrol.o proccontrol_ipc.o

proccontrol_setmodule:		proccontrol_setmodule.o \
				libproccontrol_interface.a

proccontrol_setgroup:		proccontrol_setgroup.o \
				libproccontrol_interface.a

proccontrol_viewoutput:		proccontrol_viewoutput.o \
				libproccontrol_interface.a

include ../Makefile.rules
