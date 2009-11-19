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
	$(INSTALL) -m 0644 todo.desktop $(DESTDIR)$(PREFIX)/share/applications/todo.desktop

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/share/applications/todo.desktop

.PHONY: all clean distclean install uninstall
