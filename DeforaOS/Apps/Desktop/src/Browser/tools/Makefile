TARGETS	= dirtree iconlist
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config gtk+-2.0 --libs`
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

dirtree_OBJS = dirtree.o
dirtree_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
dirtree_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

dirtree: $(dirtree_OBJS)
	$(CC) -o dirtree $(dirtree_OBJS) $(dirtree_LDFLAGS)

iconlist_OBJS = iconlist.o
iconlist_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
iconlist_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

iconlist: $(iconlist_OBJS)
	$(CC) -o iconlist $(iconlist_OBJS) $(iconlist_LDFLAGS)

dirtree.o: dirtree.c
	$(CC) $(dirtree_CFLAGS) -c dirtree.c

iconlist.o: iconlist.c
	$(CC) $(iconlist_CFLAGS) -c iconlist.c

clean:
	$(RM) -- $(dirtree_OBJS) $(iconlist_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
