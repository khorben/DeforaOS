PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 0644 -- phone-n900.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/phone-n900.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 0644 -- phone-openmoko.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/phone-openmoko.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/phone-n900.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/phone-openmoko.png

.PHONY: all clean distclean install uninstall
