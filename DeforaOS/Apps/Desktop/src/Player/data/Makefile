PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- player.desktop $(DESTDIR)$(PREFIX)/share/applications/player.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/Player
	$(INSTALL) -m 0644 -- splash.png $(DESTDIR)$(PREFIX)/share/Player/splash.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/player.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/Player/splash.png

.PHONY: all clean distclean install uninstall
