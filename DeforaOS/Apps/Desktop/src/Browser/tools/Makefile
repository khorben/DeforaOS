TARGETS	= dirtree iconlist
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config gtk+-2.0 --libs`
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

dirtree_OBJS = dirtree.o
dirtree_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

dirtree: $(dirtree_OBJS)
	$(CC) -o dirtree $(dirtree_OBJS) $(LDFLAGSF) $(LDFLAGS)

iconlist_OBJS = iconlist.o
iconlist_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

iconlist: $(iconlist_OBJS)
	$(CC) -o iconlist $(iconlist_OBJS) $(LDFLAGSF) $(LDFLAGS)

dirtree.o: dirtree.c
	$(CC) $(dirtree_CFLAGS) -c dirtree.c

iconlist.o: iconlist.c
	$(CC) $(iconlist_CFLAGS) -c iconlist.c

clean:
	$(RM) $(dirtree_OBJS) $(iconlist_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 dirtree $(DESTDIR)$(BINDIR)/dirtree
	$(INSTALL) -m 0755 iconlist $(DESTDIR)$(BINDIR)/iconlist

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/dirtree
	$(RM) $(DESTDIR)$(BINDIR)/iconlist

.PHONY: all clean distclean install uninstall
