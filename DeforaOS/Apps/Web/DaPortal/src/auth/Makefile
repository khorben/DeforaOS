PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/auth
	$(INSTALL) -m 0644 -- http.php $(DESTDIR)$(PREFIX)/daportal/src/auth/http.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/auth
	$(INSTALL) -m 0644 -- session.php $(DESTDIR)$(PREFIX)/daportal/src/auth/session.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/auth
	$(INSTALL) -m 0644 -- unix.php $(DESTDIR)$(PREFIX)/daportal/src/auth/unix.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/auth/http.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/auth/session.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/auth/unix.php

.PHONY: all clean distclean install uninstall
