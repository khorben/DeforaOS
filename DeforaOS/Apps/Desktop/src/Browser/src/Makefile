TARGETS	= browser delete desktop open view
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2
LDFLAGSF= `pkg-config gtk+-2.0 --libs` -l System
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

browser_OBJS = browser.o callbacks.o mime.o main.o
browser_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

browser: $(browser_OBJS)
	$(CC) -o browser $(browser_OBJS) $(LDFLAGSF) $(LDFLAGS)

delete_OBJS = delete.o
delete_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

delete: $(delete_OBJS)
	$(CC) -o delete $(delete_OBJS) $(LDFLAGSF) $(LDFLAGS)

desktop_OBJS = desktop.o
desktop_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

desktop: $(desktop_OBJS)
	$(CC) -o desktop $(desktop_OBJS) $(LDFLAGSF) $(LDFLAGS) mime.o

open_OBJS = open.o
open_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

open: $(open_OBJS)
	$(CC) -o open $(open_OBJS) $(LDFLAGSF) $(LDFLAGS) mime.o

view_OBJS = view.o
view_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

view: $(view_OBJS)
	$(CC) -o view $(view_OBJS) $(LDFLAGSF) $(LDFLAGS) mime.o

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

desktop.o: desktop.c mime.o
	$(CC) $(desktop_CFLAGS) -c desktop.c

open.o: open.c mime.o
	$(CC) $(open_CFLAGS) -c open.c

view.o: view.c mime.o
	$(CC) $(view_CFLAGS) -c view.c

clean:
	$(RM) $(browser_OBJS) $(delete_OBJS) $(desktop_OBJS) $(open_OBJS) $(view_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 browser $(DESTDIR)$(BINDIR)/browser
	$(INSTALL) -m 0755 delete $(DESTDIR)$(BINDIR)/delete
	$(INSTALL) -m 0755 desktop $(DESTDIR)$(BINDIR)/desktop
	$(INSTALL) -m 0755 open $(DESTDIR)$(BINDIR)/open
	$(INSTALL) -m 0755 view $(DESTDIR)$(BINDIR)/view

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/browser
	$(RM) $(DESTDIR)$(BINDIR)/delete
	$(RM) $(DESTDIR)$(BINDIR)/desktop
	$(RM) $(DESTDIR)$(BINDIR)/open
	$(RM) $(DESTDIR)$(BINDIR)/view

.PHONY: all clean distclean install uninstall
