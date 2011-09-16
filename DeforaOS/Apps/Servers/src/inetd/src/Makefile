TARGETS	= inetd discard echo
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

inetd_OBJS = config.o parser.o scanner.o service.o token.o inetd.o
inetd_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
inetd_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) 

inetd: $(inetd_OBJS)
	$(CC) -o inetd $(inetd_OBJS) $(inetd_LDFLAGS)

discard_OBJS = discard.o
discard_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
discard_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

discard: $(discard_OBJS)
	$(CC) -o discard $(discard_OBJS) $(discard_LDFLAGS)

echo_OBJS = echo.o
echo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
echo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

echo: $(echo_OBJS)
	$(CC) -o echo $(echo_OBJS) $(echo_LDFLAGS)

config.o: config.c
	$(CC) $(inetd_CFLAGS) -c config.c

parser.o: parser.c
	$(CC) $(inetd_CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(inetd_CFLAGS) -c scanner.c

service.o: service.c
	$(CC) $(inetd_CFLAGS) -c service.c

token.o: token.c
	$(CC) $(inetd_CFLAGS) -c token.c

inetd.o: inetd.c
	$(CC) $(inetd_CFLAGS) -c inetd.c

discard.o: discard.c
	$(CC) $(discard_CFLAGS) -c discard.c

echo.o: echo.c
	$(CC) $(echo_CFLAGS) -c echo.c

clean:
	$(RM) -- $(inetd_OBJS) $(discard_OBJS) $(echo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- inetd $(DESTDIR)$(BINDIR)/inetd

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/inetd

.PHONY: all clean distclean install uninstall
