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
	$(INSTALL) -m 0644 -- VPN.h $(DESTDIR)$(INCLUDEDIR)/VPN.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/VPN.h

.PHONY: all clean distclean install uninstall
