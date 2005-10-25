TARGETS	= GServer
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

GServer_OBJS = gserver.o
GServer_CFLAGS = $(CFLAGSF) $(CFLAGS)
GServer: $(GServer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o GServer $(GServer_OBJS)

gserver.o: gserver.c
	$(CC) $(GServer_CFLAGS) -c gserver.c

clean:
	$(RM) $(GServer_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 GServer $(DESTDIR)$(BINDIR)/GServer

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/GServer
