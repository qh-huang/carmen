include ../Makefile.conf

MODULE_NAME = "logtools"
MODULE_COMMENT = "Log Tool"

		echo "WAND_CFLAGS     = `$(WAND_CONFIG) --cflags ` -DUSE_WAND" >> $(VARS); \
		echo "WAND_CXXFLAGS   = `$(WAND_CONFIG) --cppflags `" >> $(VARS); \
		echo "WAND_LIBS       = `$(WAND_CONFIG) --libs`" >> $(VARS); \

CFLAGS += 
IFLAGS += 
LFLAGS += `/usr/bin/Wand-config --libs` -L/usr/X11R6/lib -lmap_io -lmap_interface -llogtools -lglobal -lipc

SOURCES = utils.c color.c strings.c mdalloc.c read-script.c read-rec.c \
	  write-file.c parse-lines.c correction.c graphics.c \
	  ll2utm.c utm2ll.c magick-utils.c log2pic.c

PUBLIC_INCLUDES = logtools.h logtools_graphics.h

PUBLIC_LIBRARIES = liblogtools.a

PUBLIC_BINARIES = log2pic

TARGETS = liblogtools.a log2pic

liblogtools.a:	utils.o color.o strings.o mdalloc.o read-script.o \
	read-rec.o write-file.o parse-lines.o correction.o \
	graphics.o ll2utm.o utm2ll.o

log2pic:	magick-utils.o log2pic.o

# rules

include ../Makefile.rules
