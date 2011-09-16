SUBDIRS	= modems plugins
TARGETS	= phone phonectl
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0`
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

phone_OBJS = callbacks.o main.o modem.o phone.o
phone_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libDesktop`
phone_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libDesktop`

phone: $(phone_OBJS)
	$(CC) -o phone $(phone_OBJS) $(phone_LDFLAGS)

phonectl_OBJS = phonectl.o
phonectl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0`
phonectl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0`

phonectl: $(phonectl_OBJS)
	$(CC) -o phonectl $(phonectl_OBJS) $(phonectl_LDFLAGS)

callbacks.o: callbacks.c ../include/Phone/phone.h phone.h callbacks.h
	$(CC) $(phone_CFLAGS) -c callbacks.c

main.o: main.c ../include/Phone/phone.h phone.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(phone_CFLAGS) -c main.c

modem.o: modem.c modem.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(phone_CFLAGS) -c modem.c

phone.o: phone.c ../include/Phone/phone.h modem.h phone.h callbacks.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(phone_CFLAGS) -c phone.c

phonectl.o: phonectl.c ../include/Phone/phone.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(phonectl_CFLAGS) -c phonectl.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(phone_OBJS) $(phonectl_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(phone_OBJS) $(phonectl_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phone $(DESTDIR)$(BINDIR)/phone
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- phonectl $(DESTDIR)$(BINDIR)/phonectl

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/phone
	$(RM) -- $(DESTDIR)$(BINDIR)/phonectl

.PHONY: all subdirs clean distclean install uninstall
