SUBDIRS	= applets
TARGETS	= panel run
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem gtk+-2.0`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem gtk+-2.0`
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

panel_OBJS = panel.o window.o main.o
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel: $(panel_OBJS)
	$(CC) -o panel $(panel_OBJS) $(panel_LDFLAGS)

run_OBJS = run.o
run_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS)
run_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

run: $(run_OBJS)
	$(CC) -o run $(run_OBJS) $(run_LDFLAGS)

panel.o: panel.c common.h window.h ../include/Panel.h ../config.h
	$(CC) $(panel_CFLAGS) -c panel.c

window.o: window.c common.h window.h
	$(CC) $(panel_CFLAGS) -c window.c

main.o: main.c common.h ../config.h
	$(CC) $(panel_CFLAGS) -c main.c

run.o: run.c
	$(CC) $(run_CFLAGS) -c run.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(panel_OBJS) $(run_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(panel_OBJS) $(run_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panel $(DESTDIR)$(BINDIR)/panel
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- run $(DESTDIR)$(BINDIR)/run

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/panel
	$(RM) -- $(DESTDIR)$(BINDIR)/run

.PHONY: all subdirs clean distclean install uninstall
