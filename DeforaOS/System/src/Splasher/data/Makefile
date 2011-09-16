PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/etc/AppInterface
	$(INSTALL) -m 0644 -- Splasher.interface $(DESTDIR)$(PREFIX)/etc/AppInterface/Splasher.interface

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/etc/AppInterface/Splasher.interface

.PHONY: all clean distclean install uninstall
