PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/templates
	$(INSTALL) -m 0644 -- DeforaOS.php $(DESTDIR)$(PREFIX)/daportal/src/templates/DeforaOS.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/templates
	$(INSTALL) -m 0644 -- basic.php $(DESTDIR)$(PREFIX)/daportal/src/templates/basic.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/templates
	$(INSTALL) -m 0644 -- desktop.php $(DESTDIR)$(PREFIX)/daportal/src/templates/desktop.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/templates/DeforaOS.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/templates/basic.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/templates/desktop.php

.PHONY: all clean distclean install uninstall
