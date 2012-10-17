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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/App
	$(INSTALL) -m 0644 -- app.h $(DESTDIR)$(INCLUDEDIR)/App/app.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/App
	$(INSTALL) -m 0644 -- appclient.h $(DESTDIR)$(INCLUDEDIR)/App/appclient.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/App
	$(INSTALL) -m 0644 -- appmessage.h $(DESTDIR)$(INCLUDEDIR)/App/appmessage.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/App
	$(INSTALL) -m 0644 -- appserver.h $(DESTDIR)$(INCLUDEDIR)/App/appserver.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/App
	$(INSTALL) -m 0644 -- apptransport.h $(DESTDIR)$(INCLUDEDIR)/App/apptransport.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/App/app.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/App/appclient.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/App/appmessage.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/App/appserver.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/App/apptransport.h

.PHONY: all clean distclean install uninstall
