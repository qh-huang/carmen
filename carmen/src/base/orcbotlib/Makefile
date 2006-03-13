include ../../Makefile.conf

ifndef CYGWIN
CFLAGS += -fPIC 
endif
CFLAGS += -std=gnu99 -g
IFLAGS +=
LFLAGS += -lglobal -lipc -lparam_interface -lorcbot -lbase_interface -lserial

MODULE_NAME = "ORCBOT with orclib v5"
MODULE_COMMENT = "Library for controlling ORC (v5) robots"

SOURCES = orcbot_test.c orc_arm_lib.c orc_drive_lib.c 
PUBLIC_LIBRARIES = liborcbot.a
PUBLIC_BINARIES = orcbot_test
MAN_PAGES = 

TARGETS = liborcbot.a orcbot_test 

liborcbot.a: orc_drive_lib.o orc_arm_lib.o orclib_v5/liborc.a

orcbot_test: orcbot_test.o orc_drive_lib.o orc_arm_lib.o orclib_v5/liborc.a

include ../../Makefile.rules
