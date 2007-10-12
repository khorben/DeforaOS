TARGETS	= libSystem.a libSystem.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGS=
CFLAGSF	= -W -Wall -ansi -fPIC -I ../include
CFLAGS	= -g -O2
AR	= ar -rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libSystem_OBJS = appclient.o appinterface.o appserver.o array.o buffer.o config.o event.o hash.o string.o
libSystem_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

libSystem.a: $(libSystem_OBJS)
	$(AR) libSystem.a $(libSystem_OBJS)
	$(RANLIB) libSystem.a

libSystem.so: $(libSystem_OBJS)
	$(LD) -o libSystem.so $(libSystem_OBJS)

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

event.o: event.c
	$(CC) $(libSystem_CFLAGS) -c event.c

hash.o: hash.c
	$(CC) $(libSystem_CFLAGS) -c hash.c

string.o: string.c
	$(CC) $(libSystem_CFLAGS) -c string.c

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
