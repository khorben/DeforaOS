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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System
	$(INSTALL) -m 0644 -- App.h $(DESTDIR)$(INCLUDEDIR)/System/App.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System
	$(INSTALL) -m 0644 -- appclient.h $(DESTDIR)$(INCLUDEDIR)/System/appclient.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System
	$(INSTALL) -m 0644 -- appserver.h $(DESTDIR)$(INCLUDEDIR)/System/appserver.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/appclient.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/appserver.h

.PHONY: all clean distclean install uninstall
