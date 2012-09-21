PACKAGE	= libDesktop
VERSION	= 0.0.7
SUBDIRS	= data doc include src
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
		$(PACKAGE)-$(VERSION)/data/Makefile \
		$(PACKAGE)-$(VERSION)/data/libDesktop.pc.in \
		$(PACKAGE)-$(VERSION)/data/pkgconfig.sh \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc.sh \
		$(PACKAGE)-$(VERSION)/doc/project.conf \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/libDesktop-docs.xml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/project.conf \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/Desktop.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/about.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/accel.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/assistant.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/compat.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/help.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/libDesktop-unused.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/menubar.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/message.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/mime.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/toolbar.sgml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/tmpl/project.conf \
		$(PACKAGE)-$(VERSION)/include/Desktop.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/Desktop/about.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/accel.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/assistant.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/compat.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/help.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/menubar.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/message.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/mime.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/toolbar.h \
		$(PACKAGE)-$(VERSION)/include/Desktop/Makefile \
		$(PACKAGE)-$(VERSION)/include/Desktop/project.conf \
		$(PACKAGE)-$(VERSION)/src/about.c \
		$(PACKAGE)-$(VERSION)/src/accel.c \
		$(PACKAGE)-$(VERSION)/src/assistant.c \
		$(PACKAGE)-$(VERSION)/src/compat.c \
		$(PACKAGE)-$(VERSION)/src/help.c \
		$(PACKAGE)-$(VERSION)/src/menubar.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/mime.c \
		$(PACKAGE)-$(VERSION)/src/toolbar.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/config.sh \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
