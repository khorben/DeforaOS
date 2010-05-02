TARGETS	= gputty
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -Wall `pkg-config --cflags gtk+-2.0`
CFLAGS	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0` -lSystem
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

gputty_OBJS = gputty.o
gputty_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gputty_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gputty: $(gputty_OBJS)
	$(CC) -o gputty $(gputty_OBJS) $(gputty_LDFLAGS)

gputty.o: gputty.c gputty.h ../config.h
	$(CC) $(gputty_CFLAGS) -c gputty.c

clean:
	$(RM) $(gputty_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 gputty $(DESTDIR)$(BINDIR)/gputty

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/gputty

.PHONY: all clean distclean install uninstall
