TARGETS	= string tests.txt
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2
LDFLAGSF= -L ../src -lSystem
LDFLAGS	= -L$(PREFIX)/lib -Wl,-rpath,"$(PWD)/../src"
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

tests.txt: string
	./tests.sh -P "$(PREFIX)" -- "tests.txt"

string.o: string.c ../src/string.c
	$(CC) $(string_CFLAGS) -c string.c

clean:
	$(RM) -- $(string_OBJS) $(tests.txt_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
