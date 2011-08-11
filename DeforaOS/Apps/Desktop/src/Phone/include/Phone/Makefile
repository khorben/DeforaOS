PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install
INCLUDEDIR= $(PREFIX)/include


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Phone
	$(INSTALL) -m 0644 -- modem.h $(DESTDIR)$(PREFIX)/include/Desktop/Phone/modem.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Phone
	$(INSTALL) -m 0644 -- phone.h $(DESTDIR)$(PREFIX)/include/Desktop/Phone/phone.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Phone
	$(INSTALL) -m 0644 -- plugin.h $(DESTDIR)$(PREFIX)/include/Desktop/Phone/plugin.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Phone/modem.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Phone/phone.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Phone/plugin.h

.PHONY: all clean distclean install uninstall
