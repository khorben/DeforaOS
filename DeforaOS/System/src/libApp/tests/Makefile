TARGETS	= tests.log udp
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS= -I ../include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
LDFLAGSF= -lApp
LDFLAGS	= -L../src -Wl,-rpath,../src
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

tests.log: udp
	./tests.sh -P "$(PREFIX)" -- "tests.log"

udp_OBJS = udp.o
udp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
udp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

udp: $(udp_OBJS)
	$(CC) -o udp $(udp_OBJS) $(udp_LDFLAGS)

udp.o: udp.c ../src/libApp.a
	$(CC) $(udp_CFLAGS) -c udp.c

clean:
	$(RM) -- $(tests.log_OBJS) $(udp_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
