TARGETS	= makepasswd
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGS	= -lcrypt
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

makepasswd_OBJS = makepasswd.o md5c.o
makepasswd_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
makepasswd_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

makepasswd: $(makepasswd_OBJS)
	$(CC) -o makepasswd $(makepasswd_OBJS) $(makepasswd_LDFLAGS)

makepasswd.o: makepasswd.c global.h md5.h ../config.h
	$(CC) $(makepasswd_CFLAGS) -c makepasswd.c

md5c.o: md5c.c global.h md5.h
	$(CC) $(makepasswd_CFLAGS) -c md5c.c

clean:
	$(RM) -- $(makepasswd_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- makepasswd $(DESTDIR)$(BINDIR)/makepasswd

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/makepasswd

.PHONY: all clean distclean install uninstall
