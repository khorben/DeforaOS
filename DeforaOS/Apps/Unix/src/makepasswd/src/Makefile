TARGETS	= makepasswd
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W -Wall -ansi -D_GNU_SOURCE
CFLAGS	= -g
LDFLAGS	= -lcrypt
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

makepasswd_OBJS = makepasswd.o md5.o
makepasswd_CFLAGS = $(CFLAGSF) $(CFLAGS)
makepasswd: $(makepasswd_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o makepasswd $(makepasswd_OBJS)

makepasswd.o: makepasswd.c
	$(CC) $(makepasswd_CFLAGS) -c makepasswd.c

md5.o: md5.c
	$(CC) $(makepasswd_CFLAGS) -c md5.c

clean:
	$(RM) $(makepasswd_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 makepasswd $(DESTDIR)$(BINDIR)/makepasswd

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/makepasswd

.PHONY: all clean distclean install uninstall
