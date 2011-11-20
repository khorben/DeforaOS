MKDIR	?= mkdir -p
INSTALL	?= install
RM	?= rm -f
TARGETS	= VPN.h
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install
INCLUDEDIR= $(PREFIX)/include


all: $(TARGETS)

VPN.h: VPN.interface
	./appbroker.sh -P "$(PREFIX)" -- "VPN.h"

clean:
	$(RM) -- $(VPN.h_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 -- VPN.h $(DESTDIR)$(INCLUDEDIR)/VPN.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/etc/AppInterface
	$(INSTALL) -m 0644 -- VPN.interface $(DESTDIR)$(PREFIX)/etc/AppInterface/VPN.interface

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/VPN.h
	$(RM) -- $(DESTDIR)$(PREFIX)/etc/AppInterface/VPN.interface

.PHONY: all clean distclean install uninstall
