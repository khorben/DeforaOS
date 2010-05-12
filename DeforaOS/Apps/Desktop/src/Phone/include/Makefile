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

install: all
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 Phone.h $(DESTDIR)$(INCLUDEDIR)/Phone.h

uninstall:
	$(RM) $(DESTDIR)$(INCLUDEDIR)/Phone.h

.PHONY: all clean distclean install uninstall
