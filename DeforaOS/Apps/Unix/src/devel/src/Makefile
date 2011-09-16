TARGETS	= ar
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

ar_OBJS = ar.o
ar_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
ar_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

ar: $(ar_OBJS)
	$(CC) -o ar $(ar_OBJS) $(ar_LDFLAGS)

ar.o: ar.c
	$(CC) $(ar_CFLAGS) -c ar.c

clean:
	$(RM) -- $(ar_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- ar $(DESTDIR)$(BINDIR)/ar

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/ar

.PHONY: all clean distclean install uninstall
