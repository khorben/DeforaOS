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

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/System/Parser
	$(INSTALL) -m 0644 -- XML.h $(DESTDIR)$(PREFIX)/include/System/Parser/XML.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/System/Parser/XML.h

.PHONY: all clean distclean install uninstall
