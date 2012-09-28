TARGETS	= openssl.so tcp.so udp.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF?=
CPPFLAGS= -I ../../include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

openssl_OBJS = openssl.o
openssl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
openssl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

openssl.so: $(openssl_OBJS)
	$(CCSHARED) -o openssl.so $(openssl_OBJS) $(openssl_LDFLAGS)

tcp_OBJS = tcp.o
tcp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
tcp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

tcp.so: $(tcp_OBJS)
	$(CCSHARED) -o tcp.so $(tcp_OBJS) $(tcp_LDFLAGS)

udp_OBJS = udp.o
udp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
udp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

udp.so: $(udp_OBJS)
	$(CCSHARED) -o udp.so $(udp_OBJS) $(udp_LDFLAGS)

openssl.o: openssl.c
	$(CC) $(openssl_CFLAGS) -c openssl.c

tcp.o: tcp.c
	$(CC) $(tcp_CFLAGS) -c tcp.c

udp.o: udp.c
	$(CC) $(udp_CFLAGS) -c udp.c

clean:
	$(RM) -- $(openssl_OBJS) $(tcp_OBJS) $(udp_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
