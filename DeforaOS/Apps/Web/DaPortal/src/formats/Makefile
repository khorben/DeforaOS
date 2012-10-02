PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- atom.php $(DESTDIR)$(PREFIX)/daportal/src/formats/atom.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- html.php $(DESTDIR)$(PREFIX)/daportal/src/formats/html.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- html5.php $(DESTDIR)$(PREFIX)/daportal/src/formats/html5.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- pdf.php $(DESTDIR)$(PREFIX)/daportal/src/formats/pdf.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- plain.php $(DESTDIR)$(PREFIX)/daportal/src/formats/plain.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- xhtml1.php $(DESTDIR)$(PREFIX)/daportal/src/formats/xhtml1.php
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/formats
	$(INSTALL) -m 0644 -- xhtml11.php $(DESTDIR)$(PREFIX)/daportal/src/formats/xhtml11.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/atom.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/html.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/html5.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/pdf.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/plain.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/xhtml1.php
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/formats/xhtml11.php

.PHONY: all clean distclean install uninstall
