#$Id$
#Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
#This file is part of the DeforaOS Project



SUBDIRS	= System/src Apps
PREFIX	= /usr/local
DEVNULL	= /dev/null


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

bootstrap:
	./build.sh PREFIX="$(PREFIX)" bootstrap

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

dist:
	#XXX hack to bootstrap only configure
	./build.sh bootstrap < "$(DEVNULL)"
	./Apps/Devel/src/configure/src/configure -v
	$(MAKE) dist

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all subdirs bootstrap clean dist distclean install uninstall
