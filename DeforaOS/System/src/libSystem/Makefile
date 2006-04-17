PACKAGE	= libSystem
VERSION	= 0.0.0
SUBDIRS	= src include
LN	= ln -sf
TAR	= tar -czvf


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
		$(PACKAGE)-$(VERSION)/src/appclient.c \
		$(PACKAGE)-$(VERSION)/src/appinterface.c \
		$(PACKAGE)-$(VERSION)/src/appserver.c \
		$(PACKAGE)-$(VERSION)/src/array.c \
		$(PACKAGE)-$(VERSION)/src/buffer.c \
		$(PACKAGE)-$(VERSION)/src/config.c \
		$(PACKAGE)-$(VERSION)/src/event.c \
		$(PACKAGE)-$(VERSION)/src/hash.c \
		$(PACKAGE)-$(VERSION)/src/string.c \
		$(PACKAGE)-$(VERSION)/src/appinterface.h  \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/include/System.h \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/System/System.h \
		$(PACKAGE)-$(VERSION)/include/System/appclient.h \
		$(PACKAGE)-$(VERSION)/include/System/appserver.h \
		$(PACKAGE)-$(VERSION)/include/System/array.h \
		$(PACKAGE)-$(VERSION)/include/System/buffer.h \
		$(PACKAGE)-$(VERSION)/include/System/config.h \
		$(PACKAGE)-$(VERSION)/include/System/event.h \
		$(PACKAGE)-$(VERSION)/include/System/hash.h \
		$(PACKAGE)-$(VERSION)/include/System/string.h \
		$(PACKAGE)-$(VERSION)/include/System/project.conf \
		$(PACKAGE)-$(VERSION)/include/System/Makefile \
		$(PACKAGE)-$(VERSION)/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile
	$(RM) $(PACKAGE)-$(VERSION)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
