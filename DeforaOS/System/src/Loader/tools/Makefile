TARGETS	= ldd
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

ldd_OBJS = ldd.o
ldd_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
ldd_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

ldd: $(ldd_OBJS)
	$(CC) -o ldd $(ldd_OBJS) $(ldd_LDFLAGS)

ldd.o: ldd.c elf.c
	$(CC) $(ldd_CFLAGS) -c ldd.c

clean:
	$(RM) -- $(ldd_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -- ldd $(DESTDIR)$(PREFIX)/bin/ldd

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/bin/ldd

.PHONY: all clean distclean install uninstall
