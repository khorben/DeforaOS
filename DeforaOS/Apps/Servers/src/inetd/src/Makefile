TARGETS	= inetd discard echo
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

inetd_OBJS = config.o parser.o scanner.o service.o token.o inetd.o
inetd_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

inetd: $(inetd_OBJS)
	$(CC) -o inetd $(inetd_OBJS) $(LDFLAGSF) $(LDFLAGS)

discard_OBJS = discard.o
discard_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

discard: $(discard_OBJS)
	$(CC) -o discard $(discard_OBJS) $(LDFLAGSF) $(LDFLAGS)

echo_OBJS = echo.o
echo_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

echo: $(echo_OBJS)
	$(CC) -o echo $(echo_OBJS) $(LDFLAGSF) $(LDFLAGS)

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

.PHONY: all clean distclean install uninstall
