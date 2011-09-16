PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- player.desktop $(DESTDIR)$(PREFIX)/share/applications/player.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/player.desktop

.PHONY: all clean distclean install uninstall
