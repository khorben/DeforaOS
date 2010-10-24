PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 0644 -- mailer.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 0644 -- mailer-inbox.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer-inbox.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 0644 -- mailer-outbox.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer-outbox.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 0644 -- mailer-sent.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer-sent.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer-inbox.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer-outbox.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps/mailer-sent.png

.PHONY: all clean distclean install uninstall
