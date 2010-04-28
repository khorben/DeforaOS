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
	$(INSTALL) -m 0644 phone.desktop $(DESTDIR)$(PREFIX)/share/applications/phone.desktop

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/share/applications/phone.desktop

.PHONY: all clean distclean install uninstall
