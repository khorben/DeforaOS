PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/modules/browser
	$(INSTALL) -m 0644 -- module.php $(DESTDIR)$(PREFIX)/daportal/src/modules/browser/module.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/modules/browser/module.php

.PHONY: all clean distclean install uninstall
