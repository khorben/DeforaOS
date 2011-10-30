TARGETS	= string test
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
LDFLAGS	= -L ../src -Wl,-rpath,$(PWD)/../src -lSystem
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

string_OBJS = string.o
string_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
string_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

string: $(string_OBJS)
	$(CC) -o string $(string_OBJS) $(string_LDFLAGS)

test: string
	./test.sh -P "$(PREFIX)" -- "test"

string.o: string.c ../src/string.c
	$(CC) $(string_CFLAGS) -c string.c

clean:
	$(RM) -- $(string_OBJS) $(test_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
