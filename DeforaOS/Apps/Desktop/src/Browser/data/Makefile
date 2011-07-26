SUBDIRS	= 16x16
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f
TARGETS	= Browser.pc
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

Browser.pc: Browser.pc.in ../config.sh
	./pkgconfig.sh -P "$(PREFIX)" -- "Browser.pc"

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(Browser.pc_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(Browser.pc_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -i -- "Browser.pc"
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- browser.desktop $(DESTDIR)$(PREFIX)/share/applications/browser.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- desktop-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/desktop-settings.desktop

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	./pkgconfig.sh -P "$(DESTDIR)$(PREFIX)" -u -- "Browser.pc"
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/browser.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/desktop-settings.desktop

.PHONY: all subdirs clean distclean install uninstall
