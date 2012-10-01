PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -- deforaos-release.sh $(DESTDIR)$(PREFIX)/bin/deforaos-release.sh

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/bin/deforaos-release.sh

.PHONY: all clean distclean install uninstall
