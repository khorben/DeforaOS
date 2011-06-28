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
	$(INSTALL) -m 0644 -- mailer.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- mailer-accounts.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-accounts.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- mailer-drafts.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-drafts.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- mailer-inbox.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-inbox.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- mailer-outbox.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-outbox.png
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 0644 -- mailer-sent.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-sent.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-accounts.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-drafts.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-inbox.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-outbox.png
	$(RM) -- $(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/mailer-sent.png

.PHONY: all clean distclean install uninstall
