TARGETS	= libApp.a libApp.so.0.0 libApp.so.0 libApp.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS= -I $(PREFIX)/include -D WITH_SSL
CFLAGSF	= -W -fPIC `pkg-config --cflags libSystem`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem`
LDFLAGS	= -lssl
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libApp_OBJS = appclient.o appinterface.o appserver.o common.o
libApp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libApp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs openssl` -lssl  

libApp.a: $(libApp_OBJS)
	$(AR) -rc libApp.a $(libApp_OBJS)
	$(RANLIB) libApp.a

libApp.so.0.0 libApp.so.0 libApp.so: $(libApp_OBJS)
	$(CCSHARED) -o libApp.so.0.0 -Wl,-soname,libApp.so.0 $(libApp_OBJS) $(libApp_LDFLAGS)
	$(LN) -s -- libApp.so.0.0 libApp.so.0
	$(LN) -s -- libApp.so.0.0 libApp.so

appclient.o: appclient.c appinterface.h ../include/System/App/appclient.h common.h
	$(CC) $(libApp_CFLAGS) -c appclient.c

appinterface.o: appinterface.c ../include/System/App/appserver.h ../config.h
	$(CC) $(libApp_CFLAGS) -c appinterface.c

appserver.o: appserver.c appinterface.h ../include/System/App/appserver.h ../config.h
	$(CC) $(libApp_CFLAGS) -c appserver.c

common.o: common.c
	$(CC) $(libApp_CFLAGS) -c common.c

clean:
	$(RM) -- $(libApp_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libApp.a $(DESTDIR)$(LIBDIR)/libApp.a
	$(INSTALL) -m 0755 -- libApp.so.0.0 $(DESTDIR)$(LIBDIR)/libApp.so.0.0
	$(LN) -s -- libApp.so.0.0 $(DESTDIR)$(LIBDIR)/libApp.so.0
	$(LN) -s -- libApp.so.0.0 $(DESTDIR)$(LIBDIR)/libApp.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.so

.PHONY: all clean distclean install uninstall
