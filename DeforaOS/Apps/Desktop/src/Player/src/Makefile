TARGETS	= player
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -pedantic
LDFLAGSF= `pkg-config --libs libDesktop`
LDFLAGS	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

player_OBJS = callbacks.o player.o main.o
player_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
player_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

player: $(player_OBJS)
	$(CC) -o player $(player_OBJS) $(player_LDFLAGS)

callbacks.o: callbacks.c callbacks.h player.h
	$(CC) $(player_CFLAGS) -c callbacks.c

player.o: player.c callbacks.h player.h ../config.h
	$(CC) $(player_CFLAGS) -c player.c

main.o: main.c player.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(player_CFLAGS) -c main.c

clean:
	$(RM) -- $(player_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- player $(DESTDIR)$(BINDIR)/player

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/player

.PHONY: all clean distclean install uninstall
