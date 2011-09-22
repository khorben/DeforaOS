TARGETS	= gdeasm
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0 asm`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs gtk+-2.0 asm`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

gdeasm_OBJS = gdeasm.o
gdeasm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gdeasm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gdeasm: $(gdeasm_OBJS)
	$(CC) -o gdeasm $(gdeasm_OBJS) $(gdeasm_LDFLAGS)

gdeasm.o: gdeasm.c
	$(CC) $(gdeasm_CFLAGS) -c gdeasm.c

clean:
	$(RM) -- $(gdeasm_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
