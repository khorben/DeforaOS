PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -- dirstats $(DESTDIR)$(PREFIX)/bin/dirstats
	$(MKDIR) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -- watch $(DESTDIR)$(PREFIX)/bin/watch
	$(MKDIR) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -- which $(DESTDIR)$(PREFIX)/bin/which

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/bin/dirstats
	$(RM) -- $(DESTDIR)$(PREFIX)/bin/watch
	$(RM) -- $(DESTDIR)$(PREFIX)/bin/which

.PHONY: all clean distclean install uninstall
