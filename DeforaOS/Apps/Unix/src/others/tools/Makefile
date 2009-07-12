TARGETS	= linuxrc oinit
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

linuxrc_OBJS = linuxrc.o
linuxrc_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
linuxrc_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

linuxrc: $(linuxrc_OBJS)
	$(CC) -o linuxrc $(linuxrc_OBJS) $(linuxrc_LDFLAGS)

oinit_OBJS = oinit.o
oinit_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
oinit_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

oinit: $(oinit_OBJS)
	$(CC) -o oinit $(oinit_OBJS) $(oinit_LDFLAGS)

linuxrc.o: linuxrc.c
	$(CC) $(linuxrc_CFLAGS) -c linuxrc.c

oinit.o: oinit.c
	$(CC) $(oinit_CFLAGS) -c oinit.c

clean:
	$(RM) $(linuxrc_OBJS) $(oinit_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
