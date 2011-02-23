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
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Mailer
	$(INSTALL) -m 0644 -- account.h $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/account.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Mailer
	$(INSTALL) -m 0644 -- folder.h $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/folder.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Mailer
	$(INSTALL) -m 0644 -- mailer.h $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/mailer.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Mailer
	$(INSTALL) -m 0644 -- message.h $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/message.h
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop/Mailer
	$(INSTALL) -m 0644 -- transport.h $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/transport.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/account.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/folder.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/mailer.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/message.h
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Mailer/transport.h

.PHONY: all clean distclean install uninstall
