PACKAGE	= libSystem
VERSION	= 0.0.0
SUBDIRS	= src include
TAR	= tar cfzv


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist: distclean
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		include/System.h \
		include/appclient.h \
		include/appserver.h \
		include/array.h \
		include/config.h \
		include/event.h \
		include/hash.h \
		include/string.h \
		include/project.conf \
		include/Makefile \
		src/appclient.c \
		src/appinterface.c \
		src/appserver.c \
		src/array.c \
		src/config.c \
		src/event.c \
		src/hash.c \
		src/string.c \
		src/appinterface.h  \
		src/project.conf \
		src/Makefile \
		project.conf \
		Makefile

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
