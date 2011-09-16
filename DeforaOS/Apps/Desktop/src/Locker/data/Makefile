PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- locker-settings.desktop $(DESTDIR)$(PREFIX)/share/applications/locker-settings.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/locker-settings.desktop

.PHONY: all clean distclean install uninstall
