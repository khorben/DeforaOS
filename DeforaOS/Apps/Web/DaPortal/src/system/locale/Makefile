PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/system/locale
	$(INSTALL) -m 0644 -- gettext.php $(DESTDIR)$(PREFIX)/daportal/src/system/locale/gettext.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/system/locale/gettext.php

.PHONY: all clean distclean install uninstall
