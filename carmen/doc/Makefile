include ../src/Makefile.conf

MODULE_NAME = DOCUMENTATION
MODULE_COMMENT = The CARMEN Documentation

SOURCES = 
PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES =
PUBLIC_BINARIES = 

ifndef NO_DOXYGEN
TARGETS = carmen.doxygen  copy_files
endif

TO_HTML = tree.html carmen-logo-tiny.png doxygen.css 

copy_to_html =   $(SILENT) \
	for i in $(TO_HTML) ; do \
		echo "    ---- Copying $$i to html/" ; \
		$(CP) -f $$i html/; \
	done 


carmen.doxygen:  carmen.dox  

copy_files:  
	$(copy_to_html)





include ../src/Makefile.rules
