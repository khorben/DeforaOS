PACKAGE	= libSystem
VERSION	= 0.0.0
SUBDIRS	= src include
LN	= ln -sf
TAR	= tar cfzv


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist:
	$(RM) $(PACKAGE)-$(VERSION)
	$(LN) . $(PACKAGE)-$(VERSION)
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/include/System.h \
		$(PACKAGE)-$(VERSION)/include/appclient.h \
		$(PACKAGE)-$(VERSION)/include/appserver.h \
		$(PACKAGE)-$(VERSION)/include/array.h \
		$(PACKAGE)-$(VERSION)/include/config.h \
		$(PACKAGE)-$(VERSION)/include/event.h \
		$(PACKAGE)-$(VERSION)/include/hash.h \
		$(PACKAGE)-$(VERSION)/include/string.h \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/src/appclient.c \
		$(PACKAGE)-$(VERSION)/src/appinterface.c \
		$(PACKAGE)-$(VERSION)/src/appserver.c \
		$(PACKAGE)-$(VERSION)/src/array.c \
		$(PACKAGE)-$(VERSION)/src/config.c \
		$(PACKAGE)-$(VERSION)/src/event.c \
		$(PACKAGE)-$(VERSION)/src/hash.c \
		$(PACKAGE)-$(VERSION)/src/string.c \
		$(PACKAGE)-$(VERSION)/src/appinterface.h  \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile
	$(RM) $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
