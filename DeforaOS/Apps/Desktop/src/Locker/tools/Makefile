TARGETS	= locker-test
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

locker-test_OBJS = test.o
locker-test_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
locker-test_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

locker-test: $(locker-test_OBJS)
	$(CC) -o locker-test $(locker-test_OBJS) $(locker-test_LDFLAGS)

test.o: test.c ../src/locker.h ../config.h
	$(CC) $(locker-test_CFLAGS) -c test.c

clean:
	$(RM) -- $(locker-test_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
