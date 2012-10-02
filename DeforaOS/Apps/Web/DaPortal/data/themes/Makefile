PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/themes
	$(INSTALL) -m 0644 -- DaPortal.css $(DESTDIR)$(PREFIX)/daportal/data/themes/DaPortal.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/themes
	$(INSTALL) -m 0644 -- DeforaOS.css $(DESTDIR)$(PREFIX)/daportal/data/themes/DeforaOS.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/themes
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/themes/index.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/themes/DaPortal.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/themes/DeforaOS.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/themes/index.php

.PHONY: all clean distclean install uninstall
