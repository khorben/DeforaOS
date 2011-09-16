PACKAGE	= inetd
VERSION	= 0.0.0
SUBDIRS	= src
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
		$(PACKAGE)-$(VERSION)/src/config.c \
		$(PACKAGE)-$(VERSION)/src/parser.c \
		$(PACKAGE)-$(VERSION)/src/scanner.c \
		$(PACKAGE)-$(VERSION)/src/service.c \
		$(PACKAGE)-$(VERSION)/src/token.c \
		$(PACKAGE)-$(VERSION)/src/inetd.c \
		$(PACKAGE)-$(VERSION)/src/discard.c \
		$(PACKAGE)-$(VERSION)/src/echo.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/config.h \
		$(PACKAGE)-$(VERSION)/src/inetd.h \
		$(PACKAGE)-$(VERSION)/src/parser.h \
		$(PACKAGE)-$(VERSION)/src/scanner.h \
		$(PACKAGE)-$(VERSION)/src/service.h \
		$(PACKAGE)-$(VERSION)/src/token.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs clean distclean dist install uninstall
