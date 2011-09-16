TARGETS	= test
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -fPIC -g -O2 -pedantic
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

test_OBJS = test.o
test_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
test_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -L../src -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lGToolkit

test: $(test_OBJS)
	$(CC) -o test $(test_OBJS) $(test_LDFLAGS)

test.o: test.c ../include/GToolkit.h
	$(CC) $(test_CFLAGS) -c test.c

clean:
	$(RM) -- $(test_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
