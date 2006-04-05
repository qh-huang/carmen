total:	home export phase1 phase2 phase3 phase4

include Makefile.conf


PACKAGES = ipc global param_daemon base laser robot simulator robotgui \
	   maptools mapeditor logger logtools hmap \
	   localize navigator vasco proccontrol \
	   camera joystick pantilt gps 

ifndef NO_PYTHON
PACKAGES += python
endif

ifdef JAVA_HOME
PACKAGES += java 
endif

TAR_PACKAGE_LIST = $(PACKAGES) 
PACKAGES += $(ROBOT_TYPES)
TAR_PACKAGE_LIST += scout pioneer rflex xr4000 b21a b21r segway

RECURSE = $(SILENT) \
	for i in $(PACKAGES) xxxx ; do \
		if [ -d $$i ] ; then \
			if ! $(MAKE) -C $$i $@ ; then \
                                exit -1; \
                        fi; \
		fi \
	done

INSTALL_RECURSE = $(SILENT) \
	for i in $(PACKAGES) xxxx ; do \
		if [ -d $$i ] ; then \
			if ! $(MAKE) install -C $$i $@ ; then \
                                exit -1; \
                        fi; \
		fi \
	done

loud:
	$(SILENT) $(MAKE) MAKEFLAGS= LOUD=1 total

home: 
	$(SILENT) $(MKDIR) $(LIB_DIR)
	$(SILENT) $(MKDIR) $(SHARED_DIR)
	$(SILENT) $(MKDIR) $(BIN_DIR)
	$(SILENT) $(MKDIR) $(INC_DIR)/carmen
	$(SILENT) $(MKDIR) $(INC_DIR)
	$(SILENT) $(MKDIR) $(MAN_DIR)
	if ! test -f carmen.ini ; then \
	   echo "    ---- Copying global/carmen-std.ini to carmen.ini " ; \
	   $(CP) global/carmen-std.ini carmen.ini ; \
	fi

export:
	$(ECHO)
	$(ECHO) "       ***********"
	$(ECHO) "       E X P O R T"
	$(ECHO) "       ***********"
	$(ECHO)
	$(RECURSE)

dep depend:
	$(ECHO)
	$(ECHO) "       ***********"
	$(ECHO) "       D E P E N D"
	$(ECHO) "       ***********"
	$(ECHO)
	$(RECURSE)

phase1:
	$(ECHO)
	$(ECHO) "       *****************"
	$(ECHO) "       L I B R A R I E S"
	$(ECHO) "       *****************"
	$(ECHO)
	$(RECURSE)

phase2:
	$(ECHO)
	$(ECHO) "       ***************"
	$(ECHO) "       B I N A R I E S"
	$(ECHO) "       ***************"
	$(ECHO)
	$(RECURSE)
	$(ECHO) "Done making binaries..."


phase3:
ifndef NO_PYTHON
	$(ECHO)
	$(ECHO) "       *******************************"
	$(ECHO) "       S H A R E D   L I B R A R I E S"
	$(ECHO) "       *******************************"
	$(ECHO)
	$(RECURSE)
endif

phase4:
ifndef NO_PYTHON
	$(ECHO)
	$(ECHO) "       ***********"
	$(ECHO) "       P Y T H O N"
	$(ECHO) "       ***********"
	$(ECHO)
	$(RECURSE)
endif

install:
	$(ECHO)
	$(ECHO) "	*************"
	$(ECHO) "	I N S T A L L"
	$(ECHO) "	*************"
	$(ECHO)
	$(SILENT) $(MKDIR) $(GLOBAL_INC_DIR)/carmen
	$(INSTALL_RECURSE)

tar:
	$(MAKE) distclean
	$(ECHO)
	$(ECHO) "  Adding carmen/data..."
	$(SILENT) tar -cf $(CARMEN_HOME)/../carmen.tar \
		      -C $(CARMEN_HOME)/.. carmen/data
	$(SILENT) tar --delete -f $(CARMEN_HOME)/../carmen.tar \
		      -C $(CARMEN_HOME)/.. carmen/data/CVS
	cd $(CARMEN_HOME)/.. ; \
	if [ -f carmen/data/.cvsignore ] ; then \
		tar --delete -f carmen.tar carmen/data/.cvsignore ; \
	fi
	$(ECHO) "  Adding carmen/doc..."
	$(SILENT) tar -r -f $(CARMEN_HOME)/../carmen.tar \
		      -C $(CARMEN_HOME)/.. carmen/doc
	$(SILENT) tar --delete -f $(CARMEN_HOME)/../carmen.tar \
		      -C $(CARMEN_HOME)/.. carmen/doc/CVS
	cd $(CARMEN_HOME)/.. ; \
	if [ -f carmen/doc/.cvsignore ] ; then \
		tar --delete -f carmen.tar carmen/doc/.cvsignore ; \
	fi
	$(ECHO) "  Adding carmen/src..."
	cd $(CARMEN_HOME)/.. ; \
	for i in $(notdir $(wildcard $(CARMEN_HOME)/src/*)) xxxx ; do \
		if [ -f carmen/src/$$i ] ; then \
			tar -f carmen.tar -r carmen/src/$$i ; \
		fi \
	done
	$(SILENT) tar --delete -f $(CARMEN_HOME)/../carmen.tar \
		      -C $(CARMEN_HOME)/.. carmen/src/carmen.ini
	cd $(CARMEN_HOME)/.. ; \
	for i in $(TAR_PACKAGE_LIST) xxxx ; do \
		if [ -d carmen/src/$$i ] ; then \
			echo "  Adding carmen/src/$$i..." ; \
			tar -r -f carmen.tar carmen/src/$$i ; \
			tar --delete -f carmen.tar carmen/src/$$i/CVS ; \
			if [ -f carmen/src/$$i/.cvsignore ] ; then \
				tar --delete -f carmen.tar \
					carmen/src/$$i/.cvsignore ; \
			fi \
		fi \
	done
	mv $(CARMEN_HOME)/../carmen.tar $(CARMEN_HOME)
	$(SILENT) gzip -f9 $(CARMEN_HOME)/carmen.tar
	$(ECHO)
	$(ECHO) "  --->  $(CARMEN_HOME)/carmen.tar.gz"
	$(ECHO)

clean:
	$(ECHO)
	$(ECHO) "       *********"
	$(ECHO) "       C L E A N"
	$(ECHO) "       *********"
	$(ECHO)
	$(RECURSE)

relink:
	$(ECHO)
	$(ECHO) "       ***********"
	$(ECHO) "       R E L I N K"
	$(ECHO) "       ***********"
	$(ECHO)
	$(RECURSE)

distclean:
	$(ECHO)
	$(ECHO) "       *****************"
	$(ECHO) "       D I S T C L E A N"
	$(ECHO) "       *****************"
	$(ECHO)
	$(RECURSE)
	$(SILENT) $(RM) $(LIB_DIR)
	$(SILENT) $(RM) $(SHARED_DIR)
	$(SILENT) $(RM) $(INC_DIR)/carmen
	$(SILENT) $(RM) $(INC_DIR)
	$(SILENT) $(RM) $(BIN_DIR)
	$(SILENT) $(RM) $(MAN_DIR)
	$(SILENT) $(RM) $(DOC_DIR)/*.log $(DOC_DIR)/*.aux  $(DOC_DIR)/*.dvi
	$(SILENT) $(RM) $(CARMEN_HOME)/src/Makefile.vars
	$(SILENT) $(RM) $(CARMEN_HOME)/src/*~
	$(SILENT) $(RM) $(CARMEN_HOME)/src/.\#*
	$(SILENT) $(RM) $(CARMEN_HOME)/src/\#*
	$(SILENT) $(RM) $(CARMEN_HOME)/src/core
	$(ECHO) "    ---- Cleaning up bin, include, lib, and src dirs"

debug:
	$(SILENT) $(MAKE) CARMEN_DEBUG=1 total

.PHONY:	  debug















