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
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Locker
	$(INSTALL) -m 0644 -- demo.h $(DESTDIR)$(PREFIX)/include/Desktop/Locker/demo.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Locker
	$(INSTALL) -m 0644 -- locker.h $(DESTDIR)$(PREFIX)/include/Desktop/Locker/locker.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Locker
	$(INSTALL) -m 0644 -- plugin.h $(DESTDIR)$(PREFIX)/include/Desktop/Locker/plugin.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Locker/demo.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Locker/locker.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Locker/plugin.h

.PHONY: all clean distclean install uninstall
