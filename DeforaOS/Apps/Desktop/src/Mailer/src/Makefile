SUBDIRS	= account
TARGETS	= mailer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem libDesktop`
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

mailer_OBJS = account.o callbacks.o compose.o mailer.o main.o
mailer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mailer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

mailer: $(mailer_OBJS)
	$(CC) -o mailer $(mailer_OBJS) $(mailer_LDFLAGS)

account.o: account.c mailer.h account.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c account.c

callbacks.o: callbacks.c account.h callbacks.h compose.h mailer.h gtkassistant.c
	$(CC) $(mailer_CFLAGS) -c callbacks.c

compose.o: compose.c callbacks.h mailer.h compose.h ../config.h
	$(CC) $(mailer_CFLAGS) -c compose.c

mailer.o: mailer.c account.h callbacks.h compose.h mailer.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c mailer.c

main.o: main.c mailer.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(mailer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(mailer_OBJS)
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- mailer $(DESTDIR)$(BINDIR)/mailer

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/mailer

.PHONY: all subdirs clean distclean install uninstall
