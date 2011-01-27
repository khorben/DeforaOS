PREFIX	= /usr/local
DESTDIR	= 
MKDIR	= mkdir -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 -- mixer.desktop $(DESTDIR)$(PREFIX)/share/applications/mixer.desktop

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/mixer.desktop

.PHONY: all clean distclean install uninstall
