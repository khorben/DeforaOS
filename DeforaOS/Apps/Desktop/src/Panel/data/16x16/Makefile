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
	$(INSTALL) -m 0644 -- panel-applet-bluetooth.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/panel-applet-bluetooth.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- panel-settings.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/panel-settings.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/panel-applet-bluetooth.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/panel-settings.png

.PHONY: all clean distclean install uninstall
