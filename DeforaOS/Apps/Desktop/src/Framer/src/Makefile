TARGETS	= framer
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs gtk+-2.0`
LDFLAGS	= -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

framer_OBJS = framer.o main.o
framer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
framer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

framer: $(framer_OBJS)
	$(CC) -o framer $(framer_OBJS) $(framer_LDFLAGS)

framer.o: framer.c
	$(CC) $(framer_CFLAGS) -c framer.c

main.o: main.c
	$(CC) $(framer_CFLAGS) -c main.c

clean:
	$(RM) -- $(framer_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- framer $(DESTDIR)$(BINDIR)/framer

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/framer

.PHONY: all clean distclean install uninstall
