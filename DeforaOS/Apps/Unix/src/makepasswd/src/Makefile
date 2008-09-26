TARGETS	= makepasswd
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
LDFLAGS	= -lcrypt
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

makepasswd_OBJS = makepasswd.o md5.o
makepasswd_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

makepasswd: $(makepasswd_OBJS)
	$(CC) -o makepasswd $(makepasswd_OBJS) $(LDFLAGSF) $(LDFLAGS)

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
