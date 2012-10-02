PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/js
	$(INSTALL) -m 0644 -- editor.js $(DESTDIR)$(PREFIX)/daportal/data/js/editor.js
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/js
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/js/index.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/js/editor.js
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/js/index.php

.PHONY: all clean distclean install uninstall
