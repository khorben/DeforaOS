TARGETS	= browser delete open
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


all: $(TARGETS)

browser_OBJS = browser.o callbacks.o mime.o main.o
browser_CFLAGS = $(CFLAGSF) $(CFLAGS)

browser: $(browser_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o browser $(browser_OBJS)

delete_OBJS = delete.o
delete_CFLAGS = $(CFLAGSF) $(CFLAGS)

delete: $(delete_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o delete $(delete_OBJS)

open_OBJS = open.o mime.o
open_CFLAGS = $(CFLAGSF) $(CFLAGS)

open: $(open_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o open $(open_OBJS)

browser.o: browser.c mime.h callbacks.h browser.h
	$(CC) $(browser_CFLAGS) -c browser.c

callbacks.o: callbacks.c browser.h callbacks.h
	$(CC) $(browser_CFLAGS) -c callbacks.c

mime.o: mime.c browser.h mime.h ../config.h
	$(CC) $(browser_CFLAGS) -c mime.c

main.o: main.c browser.h mime.h
	$(CC) $(browser_CFLAGS) -c main.c

delete.o: delete.c
	$(CC) $(delete_CFLAGS) -c delete.c

open.o: open.c
	$(CC) $(open_CFLAGS) -c open.c

mime.o: mime.c browser.h mime.h ../config.h
	$(CC) $(open_CFLAGS) -c mime.c

clean:
	$(RM) $(browser_OBJS) $(delete_OBJS) $(open_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 browser $(DESTDIR)$(BINDIR)/browser
	$(INSTALL) -m 0755 delete $(DESTDIR)$(BINDIR)/delete
	$(INSTALL) -m 0755 open $(DESTDIR)$(BINDIR)/open

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/browser
	$(RM) $(DESTDIR)$(BINDIR)/delete
	$(RM) $(DESTDIR)$(BINDIR)/open

.PHONY: all clean distclean install uninstall
