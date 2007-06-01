TARGETS	= gputty
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CPPFLAGS=
CFLAGSF	= -W -Wall `pkg-config --cflags gtk+-2.0`
CFLAGS	= -g
LDFLAGSF= `pkg-config --libs gtk+-2.0` -l System -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

gputty_OBJS = gputty.o
gputty_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

gputty: $(gputty_OBJS)
	$(CC) -o gputty $(gputty_OBJS) $(LDFLAGSF) $(LDFLAGS)

gputty.o: gputty.c ../config.h
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
