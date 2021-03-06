SUBDIRS	= auth demos plugins
TARGETS	= locker lockerctl
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libDesktop`
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

locker_OBJS = locker.o main.o
locker_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags x11 xscrnsaver`
locker_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs x11 xscrnsaver`

locker: $(locker_OBJS)
	$(CC) -o locker $(locker_OBJS) $(locker_LDFLAGS)

lockerctl_OBJS = lockerctl.o
lockerctl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
lockerctl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

lockerctl: $(lockerctl_OBJS)
	$(CC) -o lockerctl $(lockerctl_OBJS) $(lockerctl_LDFLAGS)

locker.o: locker.c locker.h ../include/Locker.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(locker_CFLAGS) -c locker.c

main.o: main.c locker.h ../include/Locker.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(locker_CFLAGS) -c main.c

lockerctl.o: lockerctl.c ../include/Locker.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(lockerctl_CFLAGS) -c lockerctl.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(locker_OBJS) $(lockerctl_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(locker_OBJS) $(lockerctl_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- locker $(DESTDIR)$(BINDIR)/locker
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- lockerctl $(DESTDIR)$(BINDIR)/lockerctl

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/locker
	$(RM) -- $(DESTDIR)$(BINDIR)/lockerctl

.PHONY: all subdirs clean distclean install uninstall
