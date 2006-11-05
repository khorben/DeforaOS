SUBDIRS	= account
TARGETS	= mailer
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2 -ansi
LDFLAGSF= `pkg-config gtk+-2.0 --libs` -l System
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

mailer_OBJS = account.o callbacks.o common.o compose.o mailer.o main.o
mailer_CFLAGS = $(CFLAGSF) $(CFLAGS)
mailer: $(mailer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o mailer $(mailer_OBJS)

account.o: account.c account/account.h
	$(CC) $(mailer_CFLAGS) -c account.c

callbacks.o: callbacks.c account/account.h callbacks.h compose.h mailer.h
	$(CC) $(mailer_CFLAGS) -c callbacks.c

common.o: common.c
	$(CC) $(mailer_CFLAGS) -c common.c

compose.o: compose.c callbacks.h mailer.h compose.h
	$(CC) $(mailer_CFLAGS) -c compose.c

mailer.o: mailer.c account/account.h callbacks.h compose.h mailer.h
	$(CC) $(mailer_CFLAGS) -c mailer.c

main.o: main.c mailer.h
	$(CC) $(mailer_CFLAGS) -c main.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) $(mailer_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) $(mailer_OBJS)
	$(RM) $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 mailer $(DESTDIR)$(BINDIR)/mailer

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) $(DESTDIR)$(BINDIR)/mailer

.PHONY: all subdirs clean distclean install uninstall
