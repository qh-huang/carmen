include ../../Makefile.conf

ifndef CYGWIN
CFLAGS += -fPIC 
endif
IFLAGS +=
LFLAGS += -lglobal -lipc -lparam_interface

MODULE_NAME = "ORCBOT with orclib v5"
MODULE_COMMENT = "Library for controlling ORC (v5) robots"

SOURCES = orc_arm_lib.c orc_drive_lib.c 
PUBLIC_LIBRARIES = liborcbot.a
MAN_PAGES = 

TARGETS = liborcbot.a

liborcbot.a: orc_drive_lib.o orc_arm_lib.o

include ../../Makefile.rules
