PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/etc/AppInterface
	$(INSTALL) -m 0644 Directory.interface $(DESTDIR)$(PREFIX)/etc/AppInterface/Directory.interface

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/etc/AppInterface/Directory.interface

.PHONY: all clean distclean install uninstall
