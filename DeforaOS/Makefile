#$Id$
#Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>
#This file is part of the DeforaOS Project



SUBDIRS	= System/src Apps
PREFIX	= /usr/local


all:
	./build.sh bootstrap

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

dist:
	#XXX hack to bootstrap only configure
	./build.sh bootstrap < /dev/null
	./Apps/Devel/src/configure/src/configure -v
	$(MAKE) dist

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install: all
	$(MAKE) install

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done

.PHONY: all clean dist distclean install uninstall
