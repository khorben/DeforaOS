PACKAGE	= cpp
VERSION	= 0.0.1
SUBDIRS	= data include src
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
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz -- \
		$(PACKAGE)-$(VERSION)/data/Makefile \
		$(PACKAGE)-$(VERSION)/data/cpp.pc.in \
		$(PACKAGE)-$(VERSION)/data/pkgconfig.sh \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/include/cpp.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/src/cpp.c \
		$(PACKAGE)-$(VERSION)/src/parser.c \
		$(PACKAGE)-$(VERSION)/src/scanner.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/parser.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/config.sh \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
