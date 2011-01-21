PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- browser-view-details.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/browser-view-details.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- browser-view-icons.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/browser-view-icons.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- browser-view-list.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/browser-view-list.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/browser-view-details.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/browser-view-icons.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/browser-view-list.png

.PHONY: all clean distclean install uninstall
