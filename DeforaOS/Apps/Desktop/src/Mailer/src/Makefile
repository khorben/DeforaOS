SUBDIRS	= account plugins
TARGETS	= libMailer.a libMailer.so.0.0 libMailer.so.0 libMailer.so compose mailer
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGS	= 
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
BINDIR	= $(PREFIX)/bin
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

libMailer_OBJS = account.o callbacks.o compose.o folder.o mailer.o message.o
libMailer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -fPIC
libMailer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libDesktop`

libMailer.a: $(libMailer_OBJS)
	$(AR) -rc libMailer.a $(libMailer_OBJS)
	$(RANLIB) libMailer.a

libMailer.so.0.0 libMailer.so.0 libMailer.so: $(libMailer_OBJS)
	$(CCSHARED) -o libMailer.so.0.0 -Wl,-soname,libMailer.so.0 $(libMailer_OBJS) $(libMailer_LDFLAGS)
	$(LN) -s -- libMailer.so.0.0 libMailer.so.0
	$(LN) -s -- libMailer.so.0.0 libMailer.so

compose_OBJS = compose-main.o
compose_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
compose_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0` -L . -Wl,-rpath,$(PREFIX)/lib -lMailer

compose: $(compose_OBJS) libMailer.so
	$(CC) -o compose $(compose_OBJS) $(compose_LDFLAGS)

mailer_OBJS = main.o
mailer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mailer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0` -L . -Wl,-rpath,$(PREFIX)/lib -lMailer

mailer: $(mailer_OBJS) libMailer.so
	$(CC) -o mailer $(mailer_OBJS) $(mailer_LDFLAGS)

account.o: account.c mailer.h message.h account.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(libMailer_CFLAGS) -c account.c

callbacks.o: callbacks.c account.h callbacks.h compose.h mailer.h gtkassistant.c
	$(CC) $(libMailer_CFLAGS) -c callbacks.c

compose.o: compose.c callbacks.h mailer.h compose.h ../config.h
	$(CC) $(libMailer_CFLAGS) -c compose.c

folder.o: folder.c mailer.h folder.h
	$(CC) $(libMailer_CFLAGS) -c folder.c

mailer.o: mailer.c account.h callbacks.h compose.h mailer.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(libMailer_CFLAGS) -c mailer.c

message.o: message.c
	$(CC) $(libMailer_CFLAGS) -c message.c

compose-main.o: compose-main.c compose.h
	$(CC) $(compose_CFLAGS) -c compose-main.c

main.o: main.c mailer.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(libMailer_OBJS) $(compose_OBJS) $(mailer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(libMailer_OBJS) $(compose_OBJS) $(mailer_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libMailer.a $(DESTDIR)$(LIBDIR)/libMailer.a
	$(INSTALL) -m 0755 -- libMailer.so.0.0 $(DESTDIR)$(LIBDIR)/libMailer.so.0.0
	$(LN) -s -- libMailer.so.0.0 $(DESTDIR)$(LIBDIR)/libMailer.so.0
	$(LN) -s -- libMailer.so.0.0 $(DESTDIR)$(LIBDIR)/libMailer.so
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- compose $(DESTDIR)$(BINDIR)/compose
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- mailer $(DESTDIR)$(BINDIR)/mailer

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(LIBDIR)/libMailer.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libMailer.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libMailer.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libMailer.so
	$(RM) -- $(DESTDIR)$(BINDIR)/compose
	$(RM) -- $(DESTDIR)$(BINDIR)/mailer

.PHONY: all subdirs clean distclean install uninstall
