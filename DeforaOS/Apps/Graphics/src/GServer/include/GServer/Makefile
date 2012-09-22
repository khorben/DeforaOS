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
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Graphics/GServer
	$(INSTALL) -m 0644 -- gserver.h $(DESTDIR)$(PREFIX)/include/Graphics/GServer/gserver.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Graphics/GServer
	$(INSTALL) -m 0644 -- video.h $(DESTDIR)$(PREFIX)/include/Graphics/GServer/video.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Graphics/GServer/gserver.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Graphics/GServer/video.h

.PHONY: all clean distclean install uninstall
