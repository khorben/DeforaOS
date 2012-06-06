TARGETS	= gdeasm
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libSystem libDesktop Asm`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem libDesktop Asm`
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

gdeasm.o: gdeasm.c ../config.h
	$(CC) $(gdeasm_CFLAGS) -c gdeasm.c

clean:
	$(RM) -- $(gdeasm_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- gdeasm $(DESTDIR)$(BINDIR)/gdeasm

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/gdeasm

.PHONY: all clean distclean install uninstall
