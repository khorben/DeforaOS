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
	$(INSTALL) -m 0644 -- editor.desktop $(DESTDIR)$(PREFIX)/share/applications/editor.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/editor.desktop

.PHONY: all clean distclean install uninstall
