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
	$(INSTALL) -m 0644 -- phone-contacts.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-contacts.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-dialer.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-dialer.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-messages.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-messages.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-settings.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps
	$(INSTALL) -m 0644 -- phone-signal-00.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-00.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps
	$(INSTALL) -m 0644 -- phone-signal-25.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-25.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps
	$(INSTALL) -m 0644 -- phone-signal-50.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-50.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps
	$(INSTALL) -m 0644 -- phone-signal-75.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-75.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps
	$(INSTALL) -m 0644 -- phone-signal-100.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-100.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-contacts.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-dialer.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-messages.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-settings.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-00.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-25.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-50.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-75.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/22x22/apps/phone-signal-100.png

.PHONY: all clean distclean install uninstall
