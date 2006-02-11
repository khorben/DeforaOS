TARGETS	= gputty
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall `pkg-config --cflags gtk+-2.0`
CFLAGS	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0`
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

gputty_OBJS = array.o config.o gputty.o hash.o
gputty_CFLAGS = $(CFLAGSF) $(CFLAGS)
gputty: $(gputty_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gputty $(gputty_OBJS)

array.o: array.c
	$(CC) $(gputty_CFLAGS) -c array.c

config.o: config.c
	$(CC) $(gputty_CFLAGS) -c config.c

gputty.o: gputty.c
	$(CC) $(gputty_CFLAGS) -c gputty.c

hash.o: hash.c
	$(CC) $(gputty_CFLAGS) -c hash.c

clean:
	$(RM) $(gputty_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 gputty $(DESTDIR)$(BINDIR)/gputty

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/gputty
