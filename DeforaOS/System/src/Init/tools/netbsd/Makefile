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
	$(INSTALL) -m 0555 Init $(DESTDIR)$(PREFIX)/share/examples/rc.d/Init

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/share/examples/rc.d/Init

.PHONY: all clean distclean install uninstall
