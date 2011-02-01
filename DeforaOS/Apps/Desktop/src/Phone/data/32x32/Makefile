PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps
	$(INSTALL) -m 0644 -- phone-dialer.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/phone-dialer.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/phone-dialer.png

.PHONY: all clean distclean install uninstall
