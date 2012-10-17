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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System/App
	$(INSTALL) -m 0644 -- app.h $(DESTDIR)$(INCLUDEDIR)/System/App/app.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System/App
	$(INSTALL) -m 0644 -- appclient.h $(DESTDIR)$(INCLUDEDIR)/System/App/appclient.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System/App
	$(INSTALL) -m 0644 -- appmessage.h $(DESTDIR)$(INCLUDEDIR)/System/App/appmessage.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System/App
	$(INSTALL) -m 0644 -- appserver.h $(DESTDIR)$(INCLUDEDIR)/System/App/appserver.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/System/App
	$(INSTALL) -m 0644 -- apptransport.h $(DESTDIR)$(INCLUDEDIR)/System/App/apptransport.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/app.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/appclient.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/appmessage.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/appserver.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/System/App/apptransport.h

.PHONY: all clean distclean install uninstall
