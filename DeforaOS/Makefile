SUBDIRS	= System/src/libSystem Apps/Devel/src/configure
PREFIX	= /usr/local



all: subdirs
	@echo
	@echo ========================================================================
	@echo 'The base library (libSystem) and the compilation configuration tool'
	@echo 'compiled correctly. To continue the compilation process you need to run'
	@echo '"make bootstrap". This will install libSystem in $(PREFIX)/lib, replace'
	@echo 'this Makefile, and compile the rest of the software.'
	@echo ========================================================================
	@echo

subdirs:
	(cd System/src/libSystem && $(MAKE))
	(cd Apps/Devel/src/configure && $(MAKE) \
		CPPFLAGS="-I ../../../../../System/src/libSystem/include" \
		LDFLAGS="../../../../../System/src/libSystem/src/libSystem.a")

bootstrap:
	(cd System/src/libSystem && $(MAKE) install)
	$(MAKE) clean
	./Apps/Devel/src/configure/src/configure -v
	$(MAKE)

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
