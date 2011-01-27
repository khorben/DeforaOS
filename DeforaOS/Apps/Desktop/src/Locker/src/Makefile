TARGETS	= locker
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0 xscrnsaver`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0 xscrnsaver`
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

locker_OBJS = locker.o main.o
locker_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
locker_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

locker: $(locker_OBJS)
	$(CC) -o locker $(locker_OBJS) $(locker_LDFLAGS)

locker.o: locker.c
	$(CC) $(locker_CFLAGS) -c locker.c

main.o: main.c
	$(CC) $(locker_CFLAGS) -c main.c

clean:
	$(RM) -- $(locker_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- locker $(DESTDIR)$(BINDIR)/locker

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/locker

.PHONY: all clean distclean install uninstall
