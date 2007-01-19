TARGETS	= Probe DaMon
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
INCLUDEDIR= $(PREFIX)/include
CC	= cc
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -ansi
LDFLAGSF= -l System  -rdynamic
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Probe_OBJS = probe.o
Probe_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

Probe: $(Probe_OBJS)
	$(CC) -o Probe $(Probe_OBJS) $(LDFLAGSF) $(LDFLAGS)

DaMon_OBJS = damon.o
DaMon_CFLAGS = $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

DaMon: $(DaMon_OBJS)
	$(CC) -o DaMon $(DaMon_OBJS) $(LDFLAGSF) $(LDFLAGS)

probe.o: probe.c
	$(CC) $(Probe_CFLAGS) -c probe.c

damon.o: damon.c
	$(CC) $(DaMon_CFLAGS) -c damon.c

clean:
	$(RM) $(Probe_OBJS) $(DaMon_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 Probe $(DESTDIR)$(BINDIR)/Probe
	$(INSTALL) -m 0755 DaMon $(DESTDIR)$(BINDIR)/DaMon

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Probe
	$(RM) $(DESTDIR)$(BINDIR)/DaMon

.PHONY: all clean distclean install uninstall
