PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango
	$(INSTALL) -m 0644 -- 16x16.css $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/16x16.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango
	$(INSTALL) -m 0644 -- 24x24.css $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/24x24.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango
	$(INSTALL) -m 0644 -- 32x32.css $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/32x32.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango
	$(INSTALL) -m 0644 -- 48x48.css $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/48x48.css
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/index.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/16x16.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/24x24.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/32x32.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/48x48.css
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/Tango/index.php

.PHONY: all clean distclean install uninstall
