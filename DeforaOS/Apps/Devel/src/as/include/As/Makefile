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
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Devel/As
	$(INSTALL) -m 0644 -- arch.h $(DESTDIR)$(PREFIX)/include/Devel/As/arch.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Devel/As
	$(INSTALL) -m 0644 -- format.h $(DESTDIR)$(PREFIX)/include/Devel/As/format.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Devel/As/arch.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Devel/As/format.h

.PHONY: all clean distclean install uninstall
