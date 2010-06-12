TARGETS	= engineering.so openmoko.so profiles.so smscrypt.so
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

engineering_OBJS = engineering.o
engineering_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
engineering_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

engineering.so: $(engineering_OBJS)
	$(LD) -o engineering.so $(engineering_OBJS)

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

smscrypt_OBJS = smscrypt.o
smscrypt_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
smscrypt_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

smscrypt.so: $(smscrypt_OBJS)
	$(LD) -o smscrypt.so $(smscrypt_OBJS)

engineering.o: engineering.c ../../include/Phone.h
	$(CC) $(engineering_CFLAGS) -c engineering.c

openmoko.o: openmoko.c ../../include/Phone.h
	$(CC) $(openmoko_CFLAGS) -c openmoko.c

profiles.o: profiles.c ../../include/Phone.h
	$(CC) $(profiles_CFLAGS) -c profiles.c

smscrypt.o: smscrypt.c ../../include/Phone.h
	$(CC) $(smscrypt_CFLAGS) -c smscrypt.c

clean:
	$(RM) $(engineering_OBJS) $(openmoko_OBJS) $(profiles_OBJS) $(smscrypt_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 engineering.so $(DESTDIR)$(LIBDIR)/Phone/plugins/engineering.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 openmoko.so $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 profiles.so $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 smscrypt.so $(DESTDIR)$(LIBDIR)/Phone/plugins/smscrypt.so

uninstall:
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/engineering.so
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so
	$(RM) $(DESTDIR)$(LIBDIR)/Phone/plugins/smscrypt.so

.PHONY: all clean distclean install uninstall
