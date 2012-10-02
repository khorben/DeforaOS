PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/css
	$(INSTALL) -m 0644 -- DaPortal.css $(DESTDIR)$(PREFIX)/daportal/data/css/DaPortal.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/css
	$(INSTALL) -m 0644 -- icons.css $(DESTDIR)$(PREFIX)/daportal/data/css/icons.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/css
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/css/index.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/css/DaPortal.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/css/icons.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/css/index.php

.PHONY: all clean distclean install uninstall
