SUBDIRS	= System
PREFIX	= /usr/local
DESTDIR	= 
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install
INCLUDEDIR= $(PREFIX)/include


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 System.h $(DESTDIR)$(INCLUDEDIR)/System.h
	$(INSTALL) -m 0644 System/appclient.h $(DESTDIR)$(INCLUDEDIR)/System/appclient.h
	$(INSTALL) -m 0644 System/appserver.h $(DESTDIR)$(INCLUDEDIR)/System/appserver.h
	$(INSTALL) -m 0644 System/array.h $(DESTDIR)$(INCLUDEDIR)/System/array.h
	$(INSTALL) -m 0644 System/buffer.h $(DESTDIR)$(INCLUDEDIR)/System/buffer.h
	$(INSTALL) -m 0644 System/config.h $(DESTDIR)$(INCLUDEDIR)/System/config.h
	$(INSTALL) -m 0644 System/error.h $(DESTDIR)$(INCLUDEDIR)/System/error.h
	$(INSTALL) -m 0644 System/event.h $(DESTDIR)$(INCLUDEDIR)/System/event.h
	$(INSTALL) -m 0644 System/file.h $(DESTDIR)$(INCLUDEDIR)/System/file.h
	$(INSTALL) -m 0644 System/hash.h $(DESTDIR)$(INCLUDEDIR)/System/hash.h
	$(INSTALL) -m 0644 System/object.h $(DESTDIR)$(INCLUDEDIR)/System/object.h
	$(INSTALL) -m 0644 System/parser.h $(DESTDIR)$(INCLUDEDIR)/System/parser.h
	$(INSTALL) -m 0644 System/string.h $(DESTDIR)$(INCLUDEDIR)/System/string.h
	$(INSTALL) -m 0644 System/token.h $(DESTDIR)$(INCLUDEDIR)/System/token.h

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/appclient.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/appserver.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/array.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/buffer.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/config.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/error.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/event.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/file.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/hash.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/object.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/parser.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/string.h
	$(RM) $(DESTDIR)$(INCLUDEDIR)/System/token.h

.PHONY: all subdirs clean distclean install uninstall
