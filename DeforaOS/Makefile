SUBDIRS	= System/src/libSystem Apps/Devel/src/configure
PREFIX	= /usr/local



all: subdirs
	@echo
	@echo ========================================================================
	@echo 'The base library (libSystem) and the compilation configuration tool'
	@echo 'compiled correctly. To continue the compilation process you need to run'
	@echo '"$(MAKE) bootstrap". This will install libSystem in $(PREFIX)/lib, and'
	@echo 'replace this Makefile.'
	@echo ========================================================================
	@echo

subdirs:
	(cd System/src/libSystem && $(MAKE))
	(cd Apps/Devel/src/configure && $(MAKE) \
		CPPFLAGS="-I ../../../../../System/src/libSystem/include" \
		LDFLAGSF="../../../../../System/src/libSystem/src/libSystem.a")

bootstrap: all
	(cd System/src/libSystem && $(MAKE) install)
	$(MAKE) clean
	./Apps/Devel/src/configure/src/configure -vp "$(PREFIX)"
	@echo
	@echo ========================================================================
	@echo 'The sources tree is now configured for your environment. You may compile'
	@echo 'it as usual with "$(MAKE) all".'
	@echo ========================================================================
	@echo

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

dist: subdirs
	$(MAKE) dist

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install: all
	$(MAKE) install

uninstall: subdirs
	$(MAKE) uninstall

.PHONY: all subdirs bootstrap clean dist distclean install uninstall
