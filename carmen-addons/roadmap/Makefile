include ../Makefile.vars
include $(CARMEN_HOME)/src/Makefile.conf

IFLAGS += `$(CARMEN_HOME)/bin/carmen-config --cflags --graphics navigation`
LFLAGS += `$(CARMEN_HOME)/bin/carmen-config --libs --graphics navigation` 
# CFLAGS += -pg

LFLAGS += -ldot_interface -lsimulator_interface 

# CARMEN_DEBUG = 1

MODULE_NAME = Roadmap Navigator
MODULE_COMMENT = Another motion planner!

SOURCES = roadmap_test.c navigator.c navigator_ipc.c planner.c dynamics.c \
	fwd_lazy_dynamic_roadmap.c dynamic_roadmap.c

PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES = 

PUBLIC_BINARIES = 

TARGETS = roadmap_test navigator libdynamic_roadmap.a 

all:

libdynamic_roadmap.a: dynamic_roadmap.o dynamics.o

libfwd_lazy_dynamic_roadmap.a: fwd_lazy_dynamic_roadmap.o dynamics.o

roadmap_test: roadmap_test.o libdynamic_roadmap.a

navigator: navigator.o navigator_ipc.o planner.o libdynamic_roadmap.a

# rules

include $(CARMEN_HOME)/src/Makefile.rules
