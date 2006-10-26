SUBDIRS	= System/src/libSystem Apps/Devel/src/configure



all: subdirs
	$(MAKE) \
		CFLAGS="$(CFLAGS) -I $(PWD)/System/src/libSystem/include" \
		LDFLAGS="$(LDFLAGS) -L $(PWD)/System/src/libSystem/src"

subdirs:
	(cd System/src/libSystem && $(MAKE)) || exit
	(cd Apps/Devel/src/configure && $(MAKE) \
		CFLAGS="$(CFLAGS) -I ../../../../../System/src/libSystem/include" \
		LDFLAGS="$(LDFLAGS) -L ../../../../../System/src/libSystem/src") || exit
	LD_LIBRARY_PATH=./System/src/libSystem/src \
		./Apps/Devel/src/configure/src/configure -v

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

.PHONY: all subdirs clean dist distclean install uninstall
