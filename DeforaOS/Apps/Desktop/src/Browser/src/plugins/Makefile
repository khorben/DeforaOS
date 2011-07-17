TARGETS	= properties.so
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

properties_OBJS = properties.o
properties_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
properties_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

properties.so: $(properties_OBJS)
	$(LD) -o properties.so $(properties_OBJS) $(properties_LDFLAGS)

properties.o: properties.c ../../include/Browser.h
	$(CC) $(properties_CFLAGS) -c properties.c

clean:
	$(RM) -- $(properties_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Browser/plugins
	$(INSTALL) -m 0644 -- properties.so $(DESTDIR)$(LIBDIR)/Browser/plugins/properties.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Browser/plugins/properties.so

.PHONY: all clean distclean install uninstall
