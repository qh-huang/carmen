include ../Makefile.conf

CFLAGS +=
IFLAGS += 
LFLAGS += -lrobot_interface -lipc -lglobal -lparam_interface

MODULE_NAME = JOYSTICK
MODULE_COMMENT = Joystick library and control modules

SOURCES = keyctrl.c
PUBLIC_INCLUDES = keyctrl.h ctrl.h
PUBLIC_LIBRARIES = libkeyctrl.a
TARGETS = libkeyctrl.a 

ifndef NO_JOYSTICK
SOURCES += joystick.c joyrobot.c joyctrl.c wingman_control.c
IFLAGS += $(JOYSTICK_INCLUDE)	
PUBLIC_INCLUDES += joystick.h joyctrl.h
PUBLIC_LIBRARIES += libjoystick.a libjoyctrl.a
PUBLIC_BINARIES += joyrobot wingman_control
TARGETS += libjoystick.a libjoyctrl.a joyrobot wingman_control
endif

all:

libjoystick.a:	joystick.o

libjoyctrl.a:	joyctrl.o

libkeyctrl.a:	keyctrl.o

joyrobot:	joyrobot.o libjoyctrl.a libkeyctrl.a

wingman_control: wingman_control.o joystick.o

man:
	cp ./joyrobot.1.gz /usr/man/man1/

include ../Makefile.rules
