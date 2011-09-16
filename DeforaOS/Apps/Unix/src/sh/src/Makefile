TARGETS	= sh
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

sh_OBJS = token.o scanner.o parser.o builtin.o job.o sh.o
sh_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
sh_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

sh: $(sh_OBJS)
	$(CC) -o sh $(sh_OBJS) $(sh_LDFLAGS)

token.o: token.c
	$(CC) $(sh_CFLAGS) -c token.c

scanner.o: scanner.c
	$(CC) $(sh_CFLAGS) -c scanner.c

parser.o: parser.c
	$(CC) $(sh_CFLAGS) -c parser.c

builtin.o: builtin.c
	$(CC) $(sh_CFLAGS) -c builtin.c

job.o: job.c
	$(CC) $(sh_CFLAGS) -c job.c

sh.o: sh.c
	$(CC) $(sh_CFLAGS) -c sh.c

clean:
	$(RM) -- $(sh_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- sh $(DESTDIR)$(BINDIR)/sh

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/sh

.PHONY: all clean distclean install uninstall
