TARGETS	= debug.so hayes.so purple.so sofia.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags glib-2.0`
CFLAGS	= -Wall -g -O2 -pedantic -fPIC
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install
INCLUDEDIR= $(PREFIX)/include


all: $(TARGETS)

debug_OBJS = debug.o
debug_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0 libSystem`
debug_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0 libSystem`

debug.so: $(debug_OBJS)
	$(CCSHARED) -o debug.so $(debug_OBJS) $(debug_LDFLAGS)

hayes_OBJS = hayes.o
hayes_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem`
hayes_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem`

hayes.so: $(hayes_OBJS)
	$(CCSHARED) -o hayes.so $(hayes_OBJS) $(hayes_LDFLAGS)

purple_OBJS = purple.o
purple_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem purple`
purple_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem purple`

purple.so: $(purple_OBJS)
	$(CCSHARED) -o purple.so $(purple_OBJS) $(purple_LDFLAGS)

sofia_OBJS = sofia.o
sofia_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem sofia-sip-ua-glib`
sofia_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem sofia-sip-ua-glib`

sofia.so: $(sofia_OBJS)
	$(CCSHARED) -o sofia.so $(sofia_OBJS) $(sofia_LDFLAGS)

debug.o: debug.c
	$(CC) $(debug_CFLAGS) -c debug.c

hayes.o: hayes.c hayes.h
	$(CC) $(hayes_CFLAGS) -c hayes.c

purple.o: purple.c ../../config.h
	$(CC) $(purple_CFLAGS) -c purple.c

sofia.o: sofia.c
	$(CC) $(sofia_CFLAGS) -c sofia.c

clean:
	$(RM) -- $(debug_OBJS) $(hayes_OBJS) $(purple_OBJS) $(sofia_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/modem
	$(INSTALL) -m 0644 -- debug.so $(DESTDIR)$(LIBDIR)/Phone/modem/debug.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/modem
	$(INSTALL) -m 0644 -- hayes.so $(DESTDIR)$(LIBDIR)/Phone/modem/hayes.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/modem
	$(INSTALL) -m 0644 -- purple.so $(DESTDIR)$(LIBDIR)/Phone/modem/purple.so
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Phone/modem
	$(INSTALL) -m 0644 -- sofia.so $(DESTDIR)$(LIBDIR)/Phone/modem/sofia.so
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/Desktop/Phone/modems
	$(INSTALL) -m 0644 -- hayes.h $(DESTDIR)$(INCLUDEDIR)/Desktop/Phone/modems/hayes.h

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/modem/debug.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/modem/hayes.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/modem/purple.so
	$(RM) -- $(DESTDIR)$(LIBDIR)/Phone/modem/sofia.so
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/Desktop/Phone/modems/hayes.h

.PHONY: all clean distclean install uninstall
