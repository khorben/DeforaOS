PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install
INCLUDEDIR= $(PREFIX)/include


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 -- VFS.h $(DESTDIR)$(INCLUDEDIR)/VFS.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/etc/AppInterface
	$(INSTALL) -m 0644 -- VFS.interface $(DESTDIR)$(PREFIX)/etc/AppInterface/VFS.interface

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/VFS.h
	$(RM) -- $(DESTDIR)$(PREFIX)/etc/AppInterface/VFS.interface

.PHONY: all clean distclean install uninstall
