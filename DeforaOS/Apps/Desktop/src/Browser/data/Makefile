SUBDIRS	= 16x16
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
	$(INSTALL) -m 0644 -- browser.desktop $(DESTDIR)$(PREFIX)/share/applications/browser.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- desktop-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/desktop-settings.desktop

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/browser.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/desktop-settings.desktop

.PHONY: all subdirs clean distclean install uninstall
