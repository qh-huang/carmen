include ../Makefile.conf

LFLAGS 	+= 	-lglobal -lipc -lmap_io -lmap_interface 

MODULE_NAME = PARAMDAEMON
MODULE_COMMENT = Module providing central repository for module parameters

SOURCES = param_daemon.c param_test.c param_set.c install_params.c 
PUBLIC_INCLUDES = param_messages.h param_interface.h
PUBLIC_LIBRARIES = libparam_interface.a
PUBLIC_BINARIES = param_daemon param_set install_params 

TARGETS = param_daemon libparam_interface.a param_test param_set install_params

ifndef NO_TCPD
LFLAGS += -lwrap -lnsl
endif

ifndef NO_GRAPHICS
IFLAGS += `$(GTK_CONFIG) --cflags`
LFLAGS += -lglobal_graphics -lgdk_imlib `$(GTK_CONFIG) --libs`
SOURCES += param_edit.c
PUBLIC_BINARIES += param_edit
TARGETS += param_edit
endif

param_daemon: param_daemon.o 

libparam_interface.a:	param_interface.o

param_test: param_test.o libparam_interface.a

param_set: param_set.o libparam_interface.a

param_edit: param_edit.o libparam_interface.a

install_params: install_params.o libparam_interface.a

include ../Makefile.rules


