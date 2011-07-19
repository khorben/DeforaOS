TARGETS	= devices.so properties.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS= 
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

devices_OBJS = devices.o
devices_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
devices_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

devices.so: $(devices_OBJS)
	$(LD) -o devices.so $(devices_OBJS) $(devices_LDFLAGS)

properties_OBJS = properties.o
properties_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
properties_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

properties.so: $(properties_OBJS)
	$(LD) -o properties.so $(properties_OBJS) $(properties_LDFLAGS)

devices.o: devices.c ../../include/Browser.h
	$(CC) $(devices_CFLAGS) -c devices.c

properties.o: properties.c ../../include/Browser.h ../properties.c
	$(CC) $(properties_CFLAGS) -c properties.c

clean:
	$(RM) -- $(devices_OBJS) $(properties_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- devices.so $(DESTDIR)$(LIBDIR)/Browser/plugins/devices.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- properties.so $(DESTDIR)$(LIBDIR)/Browser/plugins/properties.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/devices.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/properties.so

.PHONY: all clean distclean install uninstall
