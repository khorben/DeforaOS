TARGETS	= libVPN.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF?=
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W -fPIC `pkg-config --cflags libApp`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libApp`
LDFLAGS	= 
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libVPN_OBJS = libvpn.o
libVPN_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libVPN_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libVPN.so: $(libVPN_OBJS)
	$(CCSHARED) -o libVPN.so $(libVPN_OBJS) $(libVPN_LDFLAGS)

libvpn.o: libvpn.c ../include/VPN.h ../src/common.c
	$(CC) $(libVPN_CFLAGS) -c libvpn.c

clean:
	$(RM) -- $(libVPN_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(PREFIX)/lib/AppWrapper
	$(INSTALL) -m 0644 -- libVPN.so $(DESTDIR)$(PREFIX)/lib/AppWrapper/libVPN.so

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/AppWrapper/libVPN.so

.PHONY: all clean distclean install uninstall
