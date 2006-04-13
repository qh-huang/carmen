#include ../Makefile.conf

SWIG      = swig
SWIGOPT   = -I../../include/carmen 
CXXSRCS    = pyCarmen.cxx pyCarmenMessages.cxx
TARGET     = pyCarmen
INTERFACE  = pyCarmen.i
LIBS       = -lm
SWIG      = swig

relink::

export::

phase1::

phase2::

phase3::

phase4:
	$(ECHO) $(MAKE) -f Makefile.conf $(SWIGLIB) CXXSRCS='$(CXXSRCS)' SWIG='$(SWIG)' \
	   SWIGOPT='$(SWIGOPT)' TARGET='$(TARGET)' INTERFACE='$(INTERFACE)' python_cpp	

all:
	$(ECHO) $(MAKE) -f Makefile.conf $(SWIGLIB) CXXSRCS='$(CXXSRCS)' SWIG='$(SWIG)' \
	   SWIGOPT='$(SWIGOPT)' TARGET='$(TARGET)' INTERFACE='$(INTERFACE)' python_cpp	

static::
	$(MAKE) -f Makefile.conf $(SWIGLIB) CXXSRCS='$(CXXSRCS)' SWIG='$(SWIG)' \
		SWIGOPT='$(SWIGOPT)' TARGET='mypython' INTERFACE='$(INTERFACE)' python_cpp_static
clean::
	$(MAKE) -f Makefile.conf python_clean
	rm -f $(TARGET).py

distclean::
	$(MAKE) -f Makefile.conf python_clean
	rm -f $(TARGET).py

check: all
