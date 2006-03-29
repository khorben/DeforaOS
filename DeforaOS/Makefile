SUBDIRS	= System/src/libSystem Apps/Devel/src/configure


all: subdirs
	$(MAKE)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	configure -v

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
