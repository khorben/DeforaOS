PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome
	$(INSTALL) -m 0644 -- 16x16.css $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/16x16.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome
	$(INSTALL) -m 0644 -- 32x32.css $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/32x32.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome
	$(INSTALL) -m 0644 -- 48x48.css $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/48x48.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/index.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/16x16.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/32x32.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/48x48.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/gnome/index.php

.PHONY: all clean distclean install uninstall
