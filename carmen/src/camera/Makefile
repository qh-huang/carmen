include ../Makefile.conf

MODULE_NAME = "CAMERA"
MODULE_COMMENT = "General Camera Module"

LFLAGS += -lparam_interface -lipc -lglobal -L/usr/X11R6/lib -lXpm
SOURCES = camera_main.c camera_view.c camera_interface.c
PUBLIC_INCLUDES = camera_messages.h camera_interface.h
PUBLIC_LIBRARIES = libcamera_interface.a
PUBLIC_BINARIES = camera_fakecam
TARGETS = camera_fakecam libcamera_interface.a 

ifdef CYGWIN
error:
	$(ECHO) "QuickCam must be run in Linux; you are using Cygwin!"
else
PUBLIC_BINARIES += camera_quickcam
TARGETS += camera_quickcam
endif

ifndef NO_GRAPHICS
IFLAGS += `$(GTK_CONFIG) --cflags` 
LFLAGS += -lparam_interface -lipc -lglobal -lglobal_graphics -lgdk_imlib \
	`$(GTK_CONFIG) --libs` 
SOURCES += camera_view.c
PUBLIC_BINARIES += camera_view
TARGETS += camera_view
endif

fakecam/libfakecam.a:
	$(MAKE) -C fakecam

camera_fakecam:		camera_main.o fakecam/libfakecam.a

ifndef CYGWIN
quickcam/libquickcam.a:
	$(MAKE) -C quickcam

camera_quickcam:	camera_main.o quickcam/libquickcam.a quickcam/libccvt.a
endif

libcamera_interface.a:	camera_interface.o

camera_view:		camera_view.o libcamera_interface.a

# rules

include ../Makefile.rules
