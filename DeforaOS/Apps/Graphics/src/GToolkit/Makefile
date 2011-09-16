PACKAGE	= GToolkit
VERSION	= 0.0.0
SUBDIRS	= include src tools
RM	?= rm -f
LN	?= ln -f
TAR	?= tar -czvf


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist:
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz -- \
		$(PACKAGE)-$(VERSION)/include/GToolkit.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/GToolkit/GWidget.h \
		$(PACKAGE)-$(VERSION)/include/GToolkit/GWindow.h \
		$(PACKAGE)-$(VERSION)/include/GToolkit/Makefile \
		$(PACKAGE)-$(VERSION)/include/GToolkit/project.conf \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/gwindow.c \
		$(PACKAGE)-$(VERSION)/src/gtoolkit.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/gwidget.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/tools/test.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
