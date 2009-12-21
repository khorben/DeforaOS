#$Id$
#Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>
#This file is part of the DeforaOS Project



SUBDIRS	= System/src/libSystem Apps/Devel/src/configure
PREFIX	= /usr/local


all: subdirs
	./Apps/Devel/src/configure/src/configure -p "$(PREFIX)"
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	@echo
	@echo ========================================================================
	@echo 'The source tree is now configured for your environment. You may compile'
	@echo 'it as usual with "$(MAKE) all".'
	@echo ========================================================================
	@echo

subdirs:
	(cd System/src/libSystem/src && $(MAKE))
	(cd Apps/Devel/src/configure/src && $(MAKE) configure \
		CPPFLAGS="-I ../../../../../System/src/libSystem/include" \
		LDFLAGSF="../../../../../System/src/libSystem/src/libSystem.a")

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

dist: subdirs
	$(MAKE) dist

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install: all
	$(MAKE) install

uninstall: all
	$(MAKE) uninstall

.PHONY: all subdirs bootstrap clean dist distclean install uninstall
