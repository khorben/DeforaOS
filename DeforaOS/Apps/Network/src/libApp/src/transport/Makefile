TARGETS	= openssl.so ssl.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF?=
CPPFLAGS= -I ../../include
CFLAGSF	= -W `pkg-config --cflags libApp`
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

ssl.so: openssl.c
	./symlink.sh -P "$(PREFIX)" -- "ssl.so"

openssl.o: openssl.c ../../config.h
	$(CC) $(openssl_CFLAGS) -c openssl.c

clean:
	$(RM) -- $(openssl_OBJS) $(ssl.so_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/App/transport
	$(INSTALL) -m 0644 -- openssl.so $(DESTDIR)$(LIBDIR)/App/transport/openssl.so
	./symlink.sh -P "$(DESTDIR)$(PREFIX)/lib/App/transport" -i -- "ssl.so"

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/App/transport/openssl.so
	./symlink.sh -P "$(DESTDIR)$(PREFIX)/lib/App/transport" -u -- "ssl.so"

.PHONY: all clean distclean install uninstall
