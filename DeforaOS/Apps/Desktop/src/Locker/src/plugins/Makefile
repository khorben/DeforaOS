TARGETS	= debug.so openmoko.so panel.so suspend.so systray.so template.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
LDFLAGSF= -W `pkg-config --libs libSystem gtk+-2.0`
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
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags Panel libDesktop`
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs Panel libDesktop`

panel.so: $(panel_OBJS)
	$(CCSHARED) -o panel.so $(panel_OBJS) $(panel_LDFLAGS)

suspend_OBJS = suspend.o
suspend_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
suspend_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

suspend.so: $(suspend_OBJS)
	$(CCSHARED) -o suspend.so $(suspend_OBJS) $(suspend_LDFLAGS)

systray_OBJS = systray.o
systray_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
systray_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

systray.so: $(systray_OBJS)
	$(CCSHARED) -o systray.so $(systray_OBJS) $(systray_LDFLAGS)

template_OBJS = template.o
template_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
template_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

template.so: $(template_OBJS)
	$(CCSHARED) -o template.so $(template_OBJS) $(template_LDFLAGS)

debug.o: debug.c ../../include/Locker.h
	$(CC) $(debug_CFLAGS) -c debug.c

openmoko.o: openmoko.c ../../include/Locker.h
	$(CC) $(openmoko_CFLAGS) -c openmoko.c

panel.o: panel.c ../../include/Locker.h
	$(CC) $(panel_CFLAGS) -c panel.c

suspend.o: suspend.c ../../include/Locker.h
	$(CC) $(suspend_CFLAGS) -c suspend.c

systray.o: systray.c ../../include/Locker.h
	$(CC) $(systray_CFLAGS) -c systray.c

template.o: template.c ../../include/Locker.h
	$(CC) $(template_CFLAGS) -c template.c

clean:
	$(RM) -- $(debug_OBJS) $(openmoko_OBJS) $(panel_OBJS) $(suspend_OBJS) $(systray_OBJS) $(template_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- openmoko.so $(DESTDIR)$(LIBDIR)/Locker/plugins/openmoko.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- panel.so $(DESTDIR)$(LIBDIR)/Locker/plugins/panel.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- suspend.so $(DESTDIR)$(LIBDIR)/Locker/plugins/suspend.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/plugins
	$(INSTALL) -m 0644 -- systray.so $(DESTDIR)$(LIBDIR)/Locker/plugins/systray.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/openmoko.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/panel.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/suspend.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/plugins/systray.so

.PHONY: all clean distclean install uninstall
