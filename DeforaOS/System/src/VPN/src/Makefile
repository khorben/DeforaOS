TARGETS	= ../include/VPN.h VPN
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libApp`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libApp` -Wl,--export-dynamic
LDFLAGS	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

../include/VPN.h:
	../include/appbroker.sh -P "$(PREFIX)" -- "../include/VPN.h"

VPN_OBJS = vpn.o main.o
VPN_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
VPN_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

VPN: $(VPN_OBJS)
	$(CC) -o VPN $(VPN_OBJS) $(VPN_LDFLAGS)

vpn.o: vpn.c ../include/VPN.h
	$(CC) $(VPN_CFLAGS) -c vpn.c

main.o: main.c
	$(CC) $(VPN_CFLAGS) -c main.c

clean:
	$(RM) -- $(../include/VPN.h_OBJS) $(VPN_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- VPN $(DESTDIR)$(BINDIR)/VPN

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/VPN

.PHONY: all clean distclean install uninstall
