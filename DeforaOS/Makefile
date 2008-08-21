SUBDIRS	= System/src/libSystem Apps/Devel/src/configure
PREFIX	= /usr/local



all: subdirs
	./Apps/Devel/src/configure/src/configure -vp "$(PREFIX)"
	$(MAKE) clean
	@echo
	@echo ========================================================================
	@echo 'The sources tree is now configured for your environment. You may compile'
	@echo 'it as usual with "$(MAKE) all".'
	@echo ========================================================================
	@echo

subdirs:
	(cd System/src/libSystem && $(MAKE))
	(cd Apps/Devel/src/configure && $(MAKE) \
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

uninstall: subdirs
	$(MAKE) uninstall

.PHONY: all subdirs bootstrap clean dist distclean install uninstall
