TARGETS	= openmoko.so profiles.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic -fPIC `pkg-config --cflags gtk+-2.0`
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

openmoko_OBJS = openmoko.o
openmoko_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
openmoko_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

openmoko.so: $(openmoko_OBJS)
	$(LD) -o openmoko.so $(openmoko_OBJS)

profiles_OBJS = profiles.o
profiles_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libpulse`
profiles_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libpulse`

profiles.so: $(profiles_OBJS)
	$(LD) -o profiles.so $(profiles_OBJS) `pkg-config --libs libpulse`

openmoko.o: openmoko.c ../../include/Phone.h
	$(CC) $(openmoko_CFLAGS) -c openmoko.c

profiles.o: profiles.c ../../include/Phone.h
	$(CC) $(profiles_CFLAGS) -c profiles.c

clean:
	$(RM) $(openmoko_OBJS) $(profiles_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 openmoko.so $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 profiles.so $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so

.PHONY: all clean distclean install uninstall
