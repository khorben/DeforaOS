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
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- about.h $(DESTDIR)$(PREFIX)/include/Desktop/about.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- accel.h $(DESTDIR)$(PREFIX)/include/Desktop/accel.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- assistant.h $(DESTDIR)$(PREFIX)/include/Desktop/assistant.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- compat.h $(DESTDIR)$(PREFIX)/include/Desktop/compat.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- menubar.h $(DESTDIR)$(PREFIX)/include/Desktop/menubar.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- mime.h $(DESTDIR)$(PREFIX)/include/Desktop/mime.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- toolbar.h $(DESTDIR)$(PREFIX)/include/Desktop/toolbar.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/about.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/accel.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/assistant.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/compat.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/menubar.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/mime.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/toolbar.h

.PHONY: all clean distclean install uninstall
