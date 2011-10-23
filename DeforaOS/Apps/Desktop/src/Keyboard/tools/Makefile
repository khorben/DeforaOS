TARGETS	= plug xkey
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs gtk+-2.0`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

plug_OBJS = plug.o
plug_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
plug_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

plug: $(plug_OBJS)
	$(CC) -o plug $(plug_OBJS) $(plug_LDFLAGS)

xkey_OBJS = xkey.o
xkey_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
xkey_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -l Xtst

xkey: $(xkey_OBJS)
	$(CC) -o xkey $(xkey_OBJS) $(xkey_LDFLAGS)

plug.o: plug.c
	$(CC) $(plug_CFLAGS) -c plug.c

xkey.o: xkey.c
	$(CC) $(xkey_CFLAGS) -c xkey.c

clean:
	$(RM) -- $(plug_OBJS) $(xkey_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
