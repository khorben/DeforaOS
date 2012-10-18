TARGETS	= tests.log transport
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS= -I ../include
CFLAGSF	= -W `pkg-config --cflags libSystem`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs libSystem` -lApp
LDFLAGS	= -L../src -Wl,-rpath,../src
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

tests.log: transport
	./tests.sh -P "$(PREFIX)" -- "tests.log"

transport_OBJS = transport.o
transport_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
transport_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

transport: $(transport_OBJS)
	$(CC) -o transport $(transport_OBJS) $(transport_LDFLAGS)

transport.o: transport.c ../src/libApp.a
	$(CC) $(transport_CFLAGS) -c transport.c

clean:
	$(RM) -- $(tests.log_OBJS) $(transport_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
