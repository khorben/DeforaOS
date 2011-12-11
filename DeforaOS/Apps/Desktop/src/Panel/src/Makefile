SUBDIRS	= applets
TARGETS	= panel panelctl run
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS?=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

panel_OBJS = panel.o window.o main.o
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem libDesktop`
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem libDesktop`

panel: $(panel_OBJS)
	$(CC) -o panel $(panel_OBJS) $(panel_LDFLAGS)

panelctl_OBJS = panelctl.o
panelctl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags gtk+-2.0`
panelctl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs gtk+-2.0`

panelctl: $(panelctl_OBJS)
	$(CC) -o panelctl $(panelctl_OBJS) $(panelctl_LDFLAGS)

run_OBJS = run.o
run_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) -D PREFIX=\"$(PREFIX)\" $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem gtk+-2.0`
run_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem gtk+-2.0`

run: $(run_OBJS)
	$(CC) -o run $(run_OBJS) $(run_LDFLAGS)

panel.o: panel.c panel.h window.h ../include/Panel.h ../config.h
	$(CC) $(panel_CFLAGS) -c panel.c

window.o: window.c panel.h window.h
	$(CC) $(panel_CFLAGS) -c window.c

main.o: main.c panel.h ../config.h
	$(CC) $(panel_CFLAGS) -c main.c

panelctl.o: panelctl.c ../include/Panel.h panel.h ../config.h
	$(CC) $(panelctl_CFLAGS) -c panelctl.c

run.o: run.c
	$(CC) $(run_CFLAGS) -c run.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(panel_OBJS) $(panelctl_OBJS) $(run_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(panel_OBJS) $(panelctl_OBJS) $(run_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panel $(DESTDIR)$(BINDIR)/panel
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- panelctl $(DESTDIR)$(BINDIR)/panelctl
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- run $(DESTDIR)$(BINDIR)/run

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/panel
	$(RM) -- $(DESTDIR)$(BINDIR)/panelctl
	$(RM) -- $(DESTDIR)$(BINDIR)/run

.PHONY: all subdirs clean distclean install uninstall
