TARGETS	= player
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2 -ansi
LDFLAGSF= `pkg-config gtk+-2.0 --libs` -l System
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

player_OBJS = callbacks.o player.o main.o
player_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

player: $(player_OBJS)
	$(CC) -o player $(player_OBJS) $(LDFLAGSF) $(LDFLAGS)

callbacks.o: callbacks.c callbacks.h player.h
	$(CC) $(player_CFLAGS) -c callbacks.c

player.o: player.c callbacks.h player.h
	$(CC) $(player_CFLAGS) -c player.c

main.o: main.c player.h
	$(CC) $(player_CFLAGS) -c main.c

clean:
	$(RM) $(player_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 player $(DESTDIR)$(BINDIR)/player

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/player

.PHONY: all clean distclean install uninstall
