PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/engines
	$(INSTALL) -m 0644 -- cli.php $(DESTDIR)$(PREFIX)/daportal/src/engines/cli.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/engines
	$(INSTALL) -m 0644 -- daportal.php $(DESTDIR)$(PREFIX)/daportal/src/engines/daportal.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/engines
	$(INSTALL) -m 0644 -- email.php $(DESTDIR)$(PREFIX)/daportal/src/engines/email.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/engines
	$(INSTALL) -m 0644 -- gtk.php $(DESTDIR)$(PREFIX)/daportal/src/engines/gtk.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/engines
	$(INSTALL) -m 0644 -- http.php $(DESTDIR)$(PREFIX)/daportal/src/engines/http.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/engines
	$(INSTALL) -m 0644 -- httpfriendly.php $(DESTDIR)$(PREFIX)/daportal/src/engines/httpfriendly.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/engines/cli.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/engines/daportal.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/engines/email.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/engines/gtk.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/engines/http.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/engines/httpfriendly.php

.PHONY: all clean distclean install uninstall
