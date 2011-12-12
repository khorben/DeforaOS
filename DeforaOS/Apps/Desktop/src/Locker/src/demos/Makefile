TARGETS	= xscreensaver.so xterm.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags x11`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

xscreensaver_OBJS = xscreensaver.o
xscreensaver_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0`
xscreensaver_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0`

xscreensaver.so: $(xscreensaver_OBJS)
	$(CCSHARED) -o xscreensaver.so $(xscreensaver_OBJS) $(xscreensaver_LDFLAGS)

xterm_OBJS = xterm.o
xterm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0`
xterm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0`

xterm.so: $(xterm_OBJS)
	$(CCSHARED) -o xterm.so $(xterm_OBJS) $(xterm_LDFLAGS)

xscreensaver.o: xscreensaver.c ../../include/Locker.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(xscreensaver_CFLAGS) -c xscreensaver.c

xterm.o: xterm.c ../../include/Locker.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(xterm_CFLAGS) -c xterm.c

clean:
	$(RM) -- $(xscreensaver_OBJS) $(xterm_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- xscreensaver.so $(DESTDIR)$(LIBDIR)/Locker/demos/xscreensaver.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- xterm.so $(DESTDIR)$(LIBDIR)/Locker/demos/xterm.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/xscreensaver.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/xterm.so

.PHONY: all clean distclean install uninstall
