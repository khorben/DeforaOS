TARGETS	= gtk-demo.so logo.so template.so xscreensaver.so xterm.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem x11 gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
LDFLAGSF= -W `pkg-config --libs libSystem x11 gtk+-2.0`
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

gtk-demo_OBJS = gtk-demo.o
gtk-demo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
gtk-demo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

gtk-demo.so: $(gtk-demo_OBJS)
	$(CCSHARED) -o gtk-demo.so $(gtk-demo_OBJS) $(gtk-demo_LDFLAGS)

logo_OBJS = logo.o
logo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
logo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

logo.so: $(logo_OBJS)
	$(CCSHARED) -o logo.so $(logo_OBJS) $(logo_LDFLAGS)

template_OBJS = template.o
template_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
template_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

template.so: $(template_OBJS)
	$(CCSHARED) -o template.so $(template_OBJS) $(template_LDFLAGS)

xscreensaver_OBJS = xscreensaver.o
xscreensaver_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
xscreensaver_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

xscreensaver.so: $(xscreensaver_OBJS)
	$(CCSHARED) -o xscreensaver.so $(xscreensaver_OBJS) $(xscreensaver_LDFLAGS)

xterm_OBJS = xterm.o
xterm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
xterm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

xterm.so: $(xterm_OBJS)
	$(CCSHARED) -o xterm.so $(xterm_OBJS) $(xterm_LDFLAGS)

gtk-demo.o: gtk-demo.c ../../include/Locker/demo.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(gtk-demo_CFLAGS) -c gtk-demo.c

logo.o: logo.c ../../include/Locker/demo.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(logo_CFLAGS) -c logo.c

template.o: template.c
	$(CC) $(template_CFLAGS) -c template.c

xscreensaver.o: xscreensaver.c ../../include/Locker.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(xscreensaver_CFLAGS) -c xscreensaver.c

xterm.o: xterm.c ../../include/Locker.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(xterm_CFLAGS) -c xterm.c

clean:
	$(RM) -- $(gtk-demo_OBJS) $(logo_OBJS) $(template_OBJS) $(xscreensaver_OBJS) $(xterm_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- gtk-demo.so $(DESTDIR)$(LIBDIR)/Locker/demos/gtk-demo.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- logo.so $(DESTDIR)$(LIBDIR)/Locker/demos/logo.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- xscreensaver.so $(DESTDIR)$(LIBDIR)/Locker/demos/xscreensaver.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- xterm.so $(DESTDIR)$(LIBDIR)/Locker/demos/xterm.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/gtk-demo.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/logo.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/xscreensaver.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/xterm.so

.PHONY: all clean distclean install uninstall
