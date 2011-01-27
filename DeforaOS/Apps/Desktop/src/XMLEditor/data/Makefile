PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- xmleditor.desktop $(DESTDIR)$(PREFIX)/share/applications/xmleditor.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/xmleditor.desktop

.PHONY: all clean distclean install uninstall
