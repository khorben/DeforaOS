SUBDIRS	= Rodent Tango generic gnome
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
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons
	$(INSTALL) -m 0644 -- Rodent.css $(DESTDIR)$(PREFIX)/daportal/data/icons/Rodent.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons
	$(INSTALL) -m 0644 -- Tango.css $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons
	$(INSTALL) -m 0644 -- generic.css $(DESTDIR)$(PREFIX)/daportal/data/icons/generic.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons
	$(INSTALL) -m 0644 -- gnome.css $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/icons/index.php

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Rodent.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/index.php

.PHONY: all subdirs clean distclean install uninstall
