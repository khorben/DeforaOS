PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install
INCLUDEDIR= $(PREFIX)/include


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/C99
	$(INSTALL) -m 0644 -- c99.h $(DESTDIR)$(INCLUDEDIR)/C99/c99.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/C99
	$(INSTALL) -m 0644 -- target.h $(DESTDIR)$(INCLUDEDIR)/C99/target.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/C99/c99.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/C99/target.h

.PHONY: all clean distclean install uninstall
