TARGETS	= debug.so openmoko.so panel.so systray.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

debug_OBJS = debug.o
debug_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
debug_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

debug.so: $(debug_OBJS)
	$(CCSHARED) -o debug.so $(debug_OBJS) $(debug_LDFLAGS)

openmoko_OBJS = openmoko.o
openmoko_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
openmoko_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

openmoko.so: $(openmoko_OBJS)
	$(CCSHARED) -o openmoko.so $(openmoko_OBJS) $(openmoko_LDFLAGS)

panel_OBJS = panel.o
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags Panel`
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel.so: $(panel_OBJS)
	$(CCSHARED) -o panel.so $(panel_OBJS) $(panel_LDFLAGS)

systray_OBJS = systray.o
systray_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
systray_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

systray.so: $(systray_OBJS)
	$(CCSHARED) -o systray.so $(systray_OBJS) $(systray_LDFLAGS)

debug.o: debug.c ../../include/Locker.h
	$(CC) $(debug_CFLAGS) -c debug.c

openmoko.o: openmoko.c ../../include/Locker.h
	$(CC) $(openmoko_CFLAGS) -c openmoko.c

panel.o: panel.c ../../include/Locker.h
	$(CC) $(panel_CFLAGS) -c panel.c

systray.o: systray.c ../../include/Locker.h
	$(CC) $(systray_CFLAGS) -c systray.c

clean:
	$(RM) -- $(debug_OBJS) $(openmoko_OBJS) $(panel_OBJS) $(systray_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- debug.so $(DESTDIR)$(LIBDIR)/Locker/plugins/debug.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- openmoko.so $(DESTDIR)$(LIBDIR)/Locker/plugins/openmoko.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- panel.so $(DESTDIR)$(LIBDIR)/Locker/plugins/panel.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- systray.so $(DESTDIR)$(LIBDIR)/Locker/plugins/systray.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/debug.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/openmoko.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/panel.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/systray.so

.PHONY: all clean distclean install uninstall
