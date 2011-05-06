SUBDIRS	= account
TARGETS	= compose-callbacks.o compose mailer
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

compose-callbacks.o_OBJS = compose-callbacks.o
compose-callbacks.o_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

compose_OBJS = compose.o compose-main.o
compose_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
compose_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) compose-callbacks.o

compose: $(compose_OBJS) compose-callbacks.o
	$(CC) -o compose $(compose_OBJS) $(compose_LDFLAGS)

mailer_OBJS = account.o callbacks.o folder.o mailer.o main.o message.o
mailer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
mailer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) compose.o compose-callbacks.o

mailer: $(mailer_OBJS) compose-callbacks.o
	$(CC) -o mailer $(mailer_OBJS) $(mailer_LDFLAGS)

compose-callbacks.o: compose-callbacks.c
	$(CC) $(compose-callbacks.o_CFLAGS) -c compose-callbacks.c

compose.o: compose.c callbacks.h mailer.h compose.h ../config.h
	$(CC) $(compose_CFLAGS) -c compose.c

compose-main.o: compose-main.c compose.h
	$(CC) $(compose_CFLAGS) -c compose-main.c

account.o: account.c mailer.h message.h account.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c account.c

callbacks.o: callbacks.c account.h callbacks.h compose.h mailer.h gtkassistant.c
	$(CC) $(mailer_CFLAGS) -c callbacks.c

folder.o: folder.c mailer.h folder.h
	$(CC) $(mailer_CFLAGS) -c folder.c

mailer.o: mailer.c account.h callbacks.h compose.h mailer.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c mailer.c

main.o: main.c mailer.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(mailer_CFLAGS) -c main.c

message.o: message.c
	$(CC) $(mailer_CFLAGS) -c message.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(compose-callbacks.o_OBJS) $(compose_OBJS) $(mailer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(compose-callbacks.o_OBJS) $(compose_OBJS) $(mailer_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- compose $(DESTDIR)$(BINDIR)/compose
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- mailer $(DESTDIR)$(BINDIR)/mailer

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/compose
	$(RM) -- $(DESTDIR)$(BINDIR)/mailer

.PHONY: all subdirs clean distclean install uninstall
