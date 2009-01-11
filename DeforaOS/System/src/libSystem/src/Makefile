TARGETS	= libSystem.a libSystem.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS= -D WITH_SSL -I ../include
CFLAGSF	= -W -Wall -ansi -fPIC
CFLAGS	= -g
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libSystem_OBJS = appclient.o appinterface.o appserver.o array.o buffer.o config.o error.o event.o hash.o object.o parser.o plugin.o string.o token.o
libSystem_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libSystem_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -l ssl

libSystem.a: $(libSystem_OBJS)
	$(AR) libSystem.a $(libSystem_OBJS)
	$(RANLIB) libSystem.a

libSystem.so: $(libSystem_OBJS)
	$(LD) -o libSystem.so $(libSystem_OBJS) -l ssl

appclient.o: appclient.c appinterface.h
	$(CC) $(libSystem_CFLAGS) -c appclient.c

appinterface.o: appinterface.c
	$(CC) $(libSystem_CFLAGS) -c appinterface.c

appserver.o: appserver.c appinterface.h ../config.h
	$(CC) $(libSystem_CFLAGS) -c appserver.c

array.o: array.c
	$(CC) $(libSystem_CFLAGS) -c array.c

buffer.o: buffer.c
	$(CC) $(libSystem_CFLAGS) -c buffer.c

config.o: config.c
	$(CC) $(libSystem_CFLAGS) -c config.c

error.o: error.c
	$(CC) $(libSystem_CFLAGS) -c error.c

event.o: event.c
	$(CC) $(libSystem_CFLAGS) -c event.c

hash.o: hash.c
	$(CC) $(libSystem_CFLAGS) -c hash.c

object.o: object.c
	$(CC) $(libSystem_CFLAGS) -c object.c

parser.o: parser.c token.h
	$(CC) $(libSystem_CFLAGS) -c parser.c

plugin.o: plugin.c
	$(CC) $(libSystem_CFLAGS) -c plugin.c

string.o: string.c
	$(CC) $(libSystem_CFLAGS) -c string.c

token.o: token.c token.h
	$(CC) $(libSystem_CFLAGS) -c token.c

clean:
	$(RM) $(libSystem_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 libSystem.a $(DESTDIR)$(LIBDIR)/libSystem.a
	$(INSTALL) -m 0755 libSystem.so $(DESTDIR)$(LIBDIR)/libSystem.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/libSystem.a
	$(RM) $(DESTDIR)$(LIBDIR)/libSystem.so

.PHONY: all clean distclean install uninstall
