TARGETS	= blacklist.so debug.so engineering.so openmoko.so oss.so panel.so profiles.so smscrypt.so
PREFIX	= /usr/local
DESTDIR	= 
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS= -I $(PREFIX)/include
CFLAGSF	= -W `pkg-config --cflags gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

blacklist_OBJS = blacklist.o
blacklist_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
blacklist_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

blacklist.so: $(blacklist_OBJS)
	$(LD) -o blacklist.so $(blacklist_OBJS)

debug_OBJS = debug.o
debug_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
debug_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

debug.so: $(debug_OBJS)
	$(LD) -o debug.so $(debug_OBJS)

engineering_OBJS = engineering.o
engineering_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
engineering_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

engineering.so: $(engineering_OBJS)
	$(LD) -o engineering.so $(engineering_OBJS)

openmoko_OBJS = openmoko.o
openmoko_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags alsa`
openmoko_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs alsa`

openmoko.so: $(openmoko_OBJS)
	$(LD) -o openmoko.so $(openmoko_OBJS) `pkg-config --libs alsa`

oss_OBJS = oss.o
oss_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
oss_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

oss.so: $(oss_OBJS)
	$(LD) -o oss.so $(oss_OBJS)

panel_OBJS = panel.o
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel.so: $(panel_OBJS)
	$(LD) -o panel.so $(panel_OBJS)

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

blacklist.o: blacklist.c ../../include/Phone.h
	$(CC) $(blacklist_CFLAGS) -c blacklist.c

debug.o: debug.c ../../include/Phone.h
	$(CC) $(debug_CFLAGS) -c debug.c

engineering.o: engineering.c ../../include/Phone.h
	$(CC) $(engineering_CFLAGS) -c engineering.c

openmoko.o: openmoko.c ../../include/Phone.h
	$(CC) $(openmoko_CFLAGS) -c openmoko.c

oss.o: oss.c ../../include/Phone.h
	$(CC) $(oss_CFLAGS) -c oss.c

panel.o: panel.c ../../include/Phone.h
	$(CC) $(panel_CFLAGS) -c panel.c

profiles.o: profiles.c ../../include/Phone.h
	$(CC) $(profiles_CFLAGS) -c profiles.c

smscrypt.o: smscrypt.c ../../include/Phone.h
	$(CC) $(smscrypt_CFLAGS) -c smscrypt.c

clean:
	$(RM) -- $(blacklist_OBJS) $(debug_OBJS) $(engineering_OBJS) $(openmoko_OBJS) $(oss_OBJS) $(panel_OBJS) $(profiles_OBJS) $(smscrypt_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- blacklist.so $(DESTDIR)$(LIBDIR)/Phone/plugins/blacklist.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- debug.so $(DESTDIR)$(LIBDIR)/Phone/plugins/debug.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- engineering.so $(DESTDIR)$(LIBDIR)/Phone/plugins/engineering.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- openmoko.so $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- panel.so $(DESTDIR)$(LIBDIR)/Phone/plugins/panel.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- profiles.so $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- smscrypt.so $(DESTDIR)$(LIBDIR)/Phone/plugins/smscrypt.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/blacklist.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/debug.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/engineering.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/panel.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/smscrypt.so

.PHONY: all clean distclean install uninstall
