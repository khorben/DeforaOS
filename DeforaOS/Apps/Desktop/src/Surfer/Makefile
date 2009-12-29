PACKAGE	= Surfer
VERSION	= 0.0.7
SUBDIRS	= data src
RM	= rm -f
LN	= ln -f
TAR	= tar -czvf


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist:
	$(RM) -r $(PACKAGE)-$(VERSION)
	$(LN) -s . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/data/Makefile \
		$(PACKAGE)-$(VERSION)/data/surfer.desktop \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/src/download.c \
		$(PACKAGE)-$(VERSION)/src/surfer.c \
		$(PACKAGE)-$(VERSION)/src/ghtml.c \
		$(PACKAGE)-$(VERSION)/src/callbacks.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/callbacks.h \
		$(PACKAGE)-$(VERSION)/src/ghtml.h \
		$(PACKAGE)-$(VERSION)/src/surfer.h \
		$(PACKAGE)-$(VERSION)/src/ghtml-gtkhtml.c \
		$(PACKAGE)-$(VERSION)/src/ghtml-gtkmozembed.c \
		$(PACKAGE)-$(VERSION)/src/ghtml-gtktextview.c \
		$(PACKAGE)-$(VERSION)/src/ghtml-webkit.c \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/common/Makefile \
		$(PACKAGE)-$(VERSION)/src/common/conn.c \
		$(PACKAGE)-$(VERSION)/src/common/history.c \
		$(PACKAGE)-$(VERSION)/src/common/url.c \
		$(PACKAGE)-$(VERSION)/src/common/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
