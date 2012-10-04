PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16
	$(INSTALL) -m 0644 -- gtk-cancel.png $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-cancel.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16
	$(INSTALL) -m 0644 -- gtk-no.png $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-no.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16
	$(INSTALL) -m 0644 -- gtk-ok.png $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-ok.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16
	$(INSTALL) -m 0644 -- gtk-yes.png $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-yes.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16
	$(INSTALL) -m 0644 -- index.php $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/index.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-cancel.png
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-no.png
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-ok.png
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/gtk-yes.png
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/data/icons/generic/16x16/index.php

.PHONY: all clean distclean install uninstall
