include ../Makefile.conf

SUBDIRS = pioneerlib rflexlib scoutlib xr4000_base


CFLAGS += 
IFLAGS += -I../robot -I../laser
LFLAGS += -lparam_interface -lglobal -lcarmenserial -lgeometry -lipc 

MODULE_NAME = BASE
MODULE_COMMENT = Generic interface to all robot bases

SOURCES = base_interface.c base_test.c drive_main.c arm_main.c base_main.c arm_test.c
PUBLIC_INCLUDES = base_messages.h arm_messages.h base_interface.h \
	arm_interface.h arm_low_level.h drive_low_level.h
PUBLIC_LIBRARIES = libbase_interface.a libarm_interface.a 

TARGETS =  libbase_interface.a libarm_interface.a  base_test arm_test

TARGETS += $(ROBOT_TYPES)
PUBLIC_BINARIES += $(ROBOT_TYPES)

PUBLIC_LIBRARIES_SO = libbase_interface.so libarm_interface.so
TARGETS += libbase_interface.so.1 libarm_interface.so.1

# rules

libbase_interface.a:	base_interface.o

libbase_interface.so.1:	base_interface.o

libarm_interface.a:	arm_interface.o

libarm_interface.so.1:	arm_interface.o

base_test:		base_test.o libbase_interface.a

arm_test:		arm_test.o libarm_interface.a

# mains for each robot
scout: base_main.o scoutlib/libscout.a 

pioneer: base_main.o pioneerlib/libpioneer.a 

orc4_main.o: orcbotlibv4/base_main.c
	$(ECHO) "    ---- Compiling $< to $@ (C)"
	$(SILENT) $(CC) -DBASE_HAS_ARM $(CFLAGS) $(IFLAGS) -c $< -o $@

orc4: orc4_main.o orcbotlibv4/orclib/liborc.a

orc_drive: drive_main.o orcbotlib/orclib_v5/liborc.a orcbotlib/liborcbot.a

orc_arm: arm_main.o orcbotlib/orclib_v5/liborc.a orcbotlib/liborcbot.a

rflex: base_main.o rflexlib/librflex.a

segway: 
	$(MAKE) -C segway_base

xr4000: 
	$(MAKE) -C xr4000_base

# libraries for each robot
scoutlib/libscout.a: 
	$(MAKE) -C scoutlib 

pioneerlib/libpioneer.a: 
	$(MAKE) -C pioneerlib

orcbotlibv4/orclib/liborc.a:
	$(MAKE) -C orcbotlibv4/orclib

orcbotlib/orclib_v5/liborc.a:
	$(MAKE) -C orcbotlib/orclib_v5

orcbotlib/liborcbot.a:
	$(MAKE) -C orcbotlib

rflexlib/librflex.a: 
	$(MAKE) -C rflexlib

include ../Makefile.rules
