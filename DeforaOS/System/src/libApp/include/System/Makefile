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
	$(INSTALL) -m 0644 -- App/appclient.h $(DESTDIR)$(INCLUDEDIR)/System/App/appclient.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System
	$(INSTALL) -m 0644 -- App/appserver.h $(DESTDIR)$(INCLUDEDIR)/System/App/appserver.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/appclient.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/appserver.h

.PHONY: all clean distclean install uninstall
