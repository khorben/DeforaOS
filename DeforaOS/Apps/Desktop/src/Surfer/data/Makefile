PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- surfer.desktop $(DESTDIR)$(PREFIX)/share/applications/surfer.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/surfer.desktop

.PHONY: all clean distclean install uninstall
