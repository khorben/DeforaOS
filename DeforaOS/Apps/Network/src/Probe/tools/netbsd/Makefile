PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/examples/rc.d
	$(INSTALL) -m 0555 DaMon $(DESTDIR)$(PREFIX)/share/examples/rc.d/DaMon
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/examples/rc.d
	$(INSTALL) -m 0555 Probe $(DESTDIR)$(PREFIX)/share/examples/rc.d/Probe

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/share/examples/rc.d/DaMon
	$(RM) $(DESTDIR)$(PREFIX)/share/examples/rc.d/Probe

.PHONY: all clean distclean install uninstall
