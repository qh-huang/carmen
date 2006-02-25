include ../../Makefile.conf

LFLAGS += -lglobal -lipc -lparam_interface -lserial

MODULE_NAME = "ORC"
MODULE_COMMENT = "Program for controlling an Orc robot"

SOURCES = orc_lib.c orc_test.c 

TARGETS = liborc.a orc_test

orc_arm:	arm_main.o orc_arm_lib.o

liborc.a:  	orc_lib.o

include ../../Makefile.rules
