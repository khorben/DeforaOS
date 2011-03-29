TARGETS	= libApp.a libApp.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../include -I $(PREFIX)/include
CPPFLAGS= 
CFLAGSF	= -W -fPIC `pkg-config --cflags libSystem`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem`
LDFLAGS	= 
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

libApp_OBJS = appclient.o appinterface.o appserver.o
libApp_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libApp_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libApp.a: $(libApp_OBJS)
	$(AR) libApp.a $(libApp_OBJS)
	$(RANLIB) libApp.a

libApp.so: $(libApp_OBJS)
	$(LD) -o libApp.so -Wl,-soname,libApp.so.0 $(libApp_OBJS) $(libApp_LDFLAGS)

appclient.o: appclient.c
	$(CC) $(libApp_CFLAGS) -c appclient.c

appinterface.o: appinterface.c
	$(CC) $(libApp_CFLAGS) -c appinterface.c

appserver.o: appserver.c
	$(CC) $(libApp_CFLAGS) -c appserver.c

clean:
	$(RM) -- $(libApp_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libApp.a $(DESTDIR)$(LIBDIR)/libApp.a
	$(INSTALL) -m 0755 -- libApp.so $(DESTDIR)$(LIBDIR)/libApp.so.0.0
	$(LN) -s -- libApp.so.0.0 $(DESTDIR)$(LIBDIR)/libApp.so.0
	$(LN) -s -- libApp.so.0.0 $(DESTDIR)$(LIBDIR)/libApp.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libApp.so

.PHONY: all clean distclean install uninstall
