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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 -- GWidget.h $(DESTDIR)$(INCLUDEDIR)/GWidget.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 -- GWindow.h $(DESTDIR)$(INCLUDEDIR)/GWindow.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/GWidget.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/GWindow.h

.PHONY: all clean distclean install uninstall
