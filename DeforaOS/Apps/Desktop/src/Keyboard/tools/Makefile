TARGETS	= plug
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config --libs gtk+-2.0`
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

plug_OBJS = plug.o
plug_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
plug_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

plug: $(plug_OBJS)
	$(CC) -o plug $(plug_OBJS) $(plug_LDFLAGS)

plug.o: plug.c
	$(CC) $(plug_CFLAGS) -c plug.c

clean:
	$(RM) -- $(plug_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
