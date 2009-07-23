TARGETS	= panel
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config gtk+-2.0 --cflags`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config gtk+-2.0 --libs` -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -lSystem
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

panel_OBJS = panel.o main.o
panel_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
panel_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

panel: $(panel_OBJS)
	$(CC) -o panel $(panel_OBJS) $(panel_LDFLAGS)

panel.o: panel.c panel.h
	$(CC) $(panel_CFLAGS) -c panel.c

main.o: main.c panel.h
	$(CC) $(panel_CFLAGS) -c main.c

clean:
	$(RM) $(panel_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 panel $(DESTDIR)$(BINDIR)/panel

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/panel

.PHONY: all clean distclean install uninstall
