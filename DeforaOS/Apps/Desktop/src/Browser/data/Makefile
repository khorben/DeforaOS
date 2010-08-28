PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- browser.desktop $(DESTDIR)$(PREFIX)/share/applications/browser.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- desktop-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/desktop-settings.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/browser.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/desktop-settings.desktop

.PHONY: all clean distclean install uninstall
