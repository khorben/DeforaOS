TARGETS	= utilbox
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

utilbox_OBJS = utilbox.o utils.o
utilbox_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
utilbox_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

utilbox: $(utilbox_OBJS)
	$(CC) -o utilbox $(utilbox_OBJS) $(utilbox_LDFLAGS)

utilbox.o: utilbox.c utilbox.h
	$(CC) $(utilbox_CFLAGS) -c utilbox.c

utils.o: utils.c utilbox.h
	$(CC) $(utilbox_CFLAGS) -c utils.c

clean:
	$(RM) -- $(utilbox_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
