TARGETS	= iconlist
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config gtk+-2.0 --libs`
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

iconlist_OBJS = iconlist.o
iconlist_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

iconlist: $(iconlist_OBJS)
	$(CC) -o iconlist $(iconlist_OBJS) $(LDFLAGSF) $(LDFLAGS)

iconlist.o: iconlist.c
	$(CC) $(iconlist_CFLAGS) -c iconlist.c

clean:
	$(RM) $(iconlist_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 iconlist $(DESTDIR)$(BINDIR)/iconlist

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/iconlist

.PHONY: all clean distclean install uninstall
