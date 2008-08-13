TARGETS	= linuxrc
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -ansi
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

linuxrc_OBJS = linuxrc.o
linuxrc_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

linuxrc: $(linuxrc_OBJS)
	$(CC) -o linuxrc $(linuxrc_OBJS) $(LDFLAGSF) $(LDFLAGS)

linuxrc.o: linuxrc.c
	$(CC) $(linuxrc_CFLAGS) -c linuxrc.c

clean:
	$(RM) $(linuxrc_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
