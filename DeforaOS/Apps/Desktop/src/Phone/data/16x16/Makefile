PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- phone-dialer.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-dialer.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- phone-drafts.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-drafts.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- phone-inbox.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-inbox.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- phone-sent.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-sent.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-dialer.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-drafts.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-inbox.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/phone-sent.png

.PHONY: all clean distclean install uninstall
