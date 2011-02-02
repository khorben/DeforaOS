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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/c99
	$(INSTALL) -m 0644 -- target.h $(DESTDIR)$(INCLUDEDIR)/c99/target.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/c99/target.h

.PHONY: all clean distclean install uninstall
