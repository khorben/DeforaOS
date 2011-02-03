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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 -- as.h $(DESTDIR)$(INCLUDEDIR)/as.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/as.h

.PHONY: all clean distclean install uninstall
