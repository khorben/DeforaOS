TARGETS	= xscreensaver.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags x11`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	= ar -rc
RANLIB	= ranlib
LD	= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

xscreensaver_OBJS = xscreensaver.o
xscreensaver_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags glib-2.0`
xscreensaver_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs glib-2.0`

xscreensaver.so: $(xscreensaver_OBJS)
	$(LD) -o xscreensaver.so $(xscreensaver_OBJS) $(xscreensaver_LDFLAGS)

xscreensaver.o: xscreensaver.c ../../include/Locker.h ../../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(xscreensaver_CFLAGS) -c xscreensaver.c

clean:
	$(RM) -- $(xscreensaver_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Locker/demos
	$(INSTALL) -m 0644 -- xscreensaver.so $(DESTDIR)$(LIBDIR)/Locker/demos/xscreensaver.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Locker/demos/xscreensaver.so

.PHONY: all clean distclean install uninstall
