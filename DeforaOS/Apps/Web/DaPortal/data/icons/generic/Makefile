SUBDIRS	= 16x16
PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic
	$(INSTALL) -m 0644 -- 16x16.css $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/index.php

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/index.php

.PHONY: all subdirs clean distclean install uninstall
