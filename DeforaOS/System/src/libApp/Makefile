PACKAGE	= libApp
VERSION	= 0.1.5
SUBDIRS	= data doc include src src/transport tools
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
		$(PACKAGE)-$(VERSION)/data/libApp.pc.in \
		$(PACKAGE)-$(VERSION)/data/pkgconfig.sh \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc.sh \
		$(PACKAGE)-$(VERSION)/doc/project.conf \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/libApp-docs.xml \
		$(PACKAGE)-$(VERSION)/doc/gtkdoc/project.conf \
		$(PACKAGE)-$(VERSION)/include/App.h \
		$(PACKAGE)-$(VERSION)/include/Makefile \
		$(PACKAGE)-$(VERSION)/include/project.conf \
		$(PACKAGE)-$(VERSION)/include/App/app.h \
		$(PACKAGE)-$(VERSION)/include/App/appclient.h \
		$(PACKAGE)-$(VERSION)/include/App/appmessage.h \
		$(PACKAGE)-$(VERSION)/include/App/appserver.h \
		$(PACKAGE)-$(VERSION)/include/App/apptransport.h \
		$(PACKAGE)-$(VERSION)/include/App/Makefile \
		$(PACKAGE)-$(VERSION)/include/App/project.conf \
		$(PACKAGE)-$(VERSION)/src/appclient.c \
		$(PACKAGE)-$(VERSION)/src/appinterface.c \
		$(PACKAGE)-$(VERSION)/src/appmessage.c \
		$(PACKAGE)-$(VERSION)/src/appserver.c \
		$(PACKAGE)-$(VERSION)/src/apptransport.c \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/appinterface.h \
		$(PACKAGE)-$(VERSION)/src/apptransport.h \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/transport/tcp.c \
		$(PACKAGE)-$(VERSION)/src/transport/udp.c \
		$(PACKAGE)-$(VERSION)/src/transport/project.conf \
		$(PACKAGE)-$(VERSION)/tools/appbroker.c \
		$(PACKAGE)-$(VERSION)/tools/appclient.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/README \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
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
