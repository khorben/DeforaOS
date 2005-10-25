TARGETS	= inetd discard echo
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall -ansi
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

inetd_OBJS = config.o parser.o scanner.o service.o token.o inetd.o
inetd_CFLAGS = $(CFLAGSF) $(CFLAGS)
inetd: $(inetd_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o inetd $(inetd_OBJS)

discard_OBJS = discard.o
discard_CFLAGS = $(CFLAGSF) $(CFLAGS)
discard: $(discard_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o discard $(discard_OBJS)

echo_OBJS = echo.o
echo_CFLAGS = $(CFLAGSF) $(CFLAGS)
echo: $(echo_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o echo $(echo_OBJS)

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
	$(RM) $(inetd_OBJS) $(discard_OBJS) $(echo_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 inetd $(DESTDIR)$(BINDIR)/inetd
	$(INSTALL) -m 0755 discard $(DESTDIR)$(BINDIR)/discard
	$(INSTALL) -m 0755 echo $(DESTDIR)$(BINDIR)/echo

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/inetd
	$(RM) $(DESTDIR)$(BINDIR)/discard
	$(RM) $(DESTDIR)$(BINDIR)/echo
