PREFIX	= /usr/local
DESTDIR	=
MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 -- appbroker.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/appbroker.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 -- docbook.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/docbook.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 -- gettext.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gettext.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 -- gtkdoc.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gtkdoc.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 -- pkgconfig.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/pkgconfig.sh

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/appbroker.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/docbook.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gettext.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gtkdoc.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/pkgconfig.sh

.PHONY: all clean distclean install uninstall
