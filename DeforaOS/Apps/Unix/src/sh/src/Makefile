TARGETS	= sh
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -ansi
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

sh_OBJS = token.o scanner.o parser.o builtin.o job.o sh.o
sh_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

sh: $(sh_OBJS)
	$(CC) -o sh $(sh_OBJS) $(LDFLAGSF) $(LDFLAGS)

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
	$(RM) $(sh_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 sh $(DESTDIR)$(BINDIR)/sh

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/sh

.PHONY: all clean distclean install uninstall
