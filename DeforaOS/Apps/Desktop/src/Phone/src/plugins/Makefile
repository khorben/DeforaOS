SUBDIRS	= 16x16 24x24 32x32 48x48
TARGETS	= blacklist.so debug.so engineering.so gprs.so locker.so n900.so openmoko.so oss.so panel.so profiles.so smscrypt.so
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


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

blacklist_OBJS = blacklist.o
blacklist_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
blacklist_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

blacklist.so: $(blacklist_OBJS)
	$(LD) -o blacklist.so $(blacklist_OBJS) $(blacklist_LDFLAGS)

debug_OBJS = debug.o
debug_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
debug_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

debug.so: $(debug_OBJS)
	$(LD) -o debug.so $(debug_OBJS) $(debug_LDFLAGS)

engineering_OBJS = engineering.o
engineering_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
engineering_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

engineering.so: $(engineering_OBJS)
	$(LD) -o engineering.so $(engineering_OBJS) $(engineering_LDFLAGS)

gprs_OBJS = gprs.o
gprs_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gprs_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gprs.so: $(gprs_OBJS)
	$(LD) -o gprs.so $(gprs_OBJS) $(gprs_LDFLAGS)

locker_OBJS = locker.o
locker_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
locker_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

locker.so: $(locker_OBJS)
	$(LD) -o locker.so $(locker_OBJS) $(locker_LDFLAGS)

n900_OBJS = n900.o
n900_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
n900_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

n900.so: $(n900_OBJS)
	$(LD) -o n900.so $(n900_OBJS) $(n900_LDFLAGS)

openmoko_OBJS = openmoko.o
openmoko_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -I../modems $(CFLAGSF) $(CFLAGS) `pkg-config --cflags alsa`
openmoko_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs alsa`

openmoko.so: $(openmoko_OBJS)
	$(LD) -o openmoko.so $(openmoko_OBJS) $(openmoko_LDFLAGS)

oss_OBJS = oss.o
oss_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
oss_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

oss.so: $(oss_OBJS)
	$(LD) -o oss.so $(oss_OBJS) $(oss_LDFLAGS)

panel_OBJS = panel.o
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel.so: $(panel_OBJS)
	$(LD) -o panel.so $(panel_OBJS) $(panel_LDFLAGS)

profiles_OBJS = profiles.o
profiles_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libpulse`
profiles_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libpulse`

profiles.so: $(profiles_OBJS)
	$(LD) -o profiles.so $(profiles_OBJS) $(profiles_LDFLAGS)

smscrypt_OBJS = smscrypt.o
smscrypt_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
smscrypt_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

smscrypt.so: $(smscrypt_OBJS)
	$(LD) -o smscrypt.so $(smscrypt_OBJS) $(smscrypt_LDFLAGS)

blacklist.o: blacklist.c ../../include/Phone.h
	$(CC) $(blacklist_CFLAGS) -c blacklist.c

debug.o: debug.c ../../include/Phone.h
	$(CC) $(debug_CFLAGS) -c debug.c

engineering.o: engineering.c ../../include/Phone.h
	$(CC) $(engineering_CFLAGS) -c engineering.c

gprs.o: gprs.c ../../include/Phone.h
	$(CC) $(gprs_CFLAGS) -c gprs.c

locker.o: locker.c ../../include/Phone.h
	$(CC) $(locker_CFLAGS) -c locker.c

n900.o: n900.c ../../include/Phone.h
	$(CC) $(n900_CFLAGS) -c n900.c

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
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(blacklist_OBJS) $(debug_OBJS) $(engineering_OBJS) $(gprs_OBJS) $(locker_OBJS) $(n900_OBJS) $(openmoko_OBJS) $(oss_OBJS) $(panel_OBJS) $(profiles_OBJS) $(smscrypt_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(blacklist_OBJS) $(debug_OBJS) $(engineering_OBJS) $(gprs_OBJS) $(locker_OBJS) $(n900_OBJS) $(openmoko_OBJS) $(oss_OBJS) $(panel_OBJS) $(profiles_OBJS) $(smscrypt_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- blacklist.so $(DESTDIR)$(LIBDIR)/Phone/plugins/blacklist.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- debug.so $(DESTDIR)$(LIBDIR)/Phone/plugins/debug.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- engineering.so $(DESTDIR)$(LIBDIR)/Phone/plugins/engineering.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- gprs.so $(DESTDIR)$(LIBDIR)/Phone/plugins/gprs.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- locker.so $(DESTDIR)$(LIBDIR)/Phone/plugins/locker.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- n900.so $(DESTDIR)$(LIBDIR)/Phone/plugins/n900.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- openmoko.so $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- panel.so $(DESTDIR)$(LIBDIR)/Phone/plugins/panel.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- profiles.so $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/plugins
	$(INSTALL) -m 0644 -- smscrypt.so $(DESTDIR)$(LIBDIR)/Phone/plugins/smscrypt.so

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/blacklist.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/debug.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/engineering.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/gprs.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/locker.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/n900.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/openmoko.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/panel.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/profiles.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/plugins/smscrypt.so

.PHONY: all subdirs clean distclean install uninstall
