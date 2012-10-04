PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/daportal/src/modules/project/scm
	$(INSTALL) -m 0644 -- cvs.php $(DESTDIR)$(PREFIX)/daportal/src/modules/project/scm/cvs.php

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/daportal/src/modules/project/scm/cvs.php

.PHONY: all clean distclean install uninstall
