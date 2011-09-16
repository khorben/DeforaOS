SUBDIRS	= 16x16 22x22 24x24 32x32 48x48
PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-contacts.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-contacts.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-dialer.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-dialer.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-log.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-log.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-messages.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-messages.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- phone-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/phone-settings.desktop

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-contacts.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-dialer.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-log.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-messages.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/phone-settings.desktop

.PHONY: all subdirs clean distclean install uninstall
