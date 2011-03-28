TARGETS	= ../data/Splasher.h Splasher
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS= 
CFLAGSF	= -W `pkg-config --cflags libApp`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libApp` -Wl,--export-dynamic
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

../data/Splasher.h: ../data/Splasher.interface
	./appbroker.sh -P "$(PREFIX)" -- "../data/Splasher.h"

Splasher_OBJS = splasher.o main.o
Splasher_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
Splasher_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

Splasher: $(Splasher_OBJS)
	$(CC) -o Splasher $(Splasher_OBJS) $(Splasher_LDFLAGS)

splasher.o: splasher.c ../data/Splasher.h
	$(CC) $(Splasher_CFLAGS) -c splasher.c

main.o: main.c
	$(CC) $(Splasher_CFLAGS) -c main.c

clean:
	$(RM) -- $(../data/Splasher.h_OBJS) $(Splasher_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- Splasher $(DESTDIR)$(BINDIR)/Splasher

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/Splasher

.PHONY: all clean distclean install uninstall
