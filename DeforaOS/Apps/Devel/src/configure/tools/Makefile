TARGETS	= pkg-config
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

pkg-config_OBJS = pkg-config.o
pkg-config_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pkg-config_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

pkg-config: $(pkg-config_OBJS)
	$(CC) -o pkg-config $(pkg-config_OBJS) $(pkg-config_LDFLAGS)

pkg-config.o: pkg-config.c ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(pkg-config_CFLAGS) -c pkg-config.c

clean:
	$(RM) -- $(pkg-config_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
