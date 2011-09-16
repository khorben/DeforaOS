TARGETS	= ../data/Probe.h Probe DaMon
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libApp`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libApp`
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

../data/Probe.h: ../data/Probe.interface
	./appbroker.sh -P "$(PREFIX)" -- "../data/Probe.h"

Probe_OBJS = probe.o
Probe_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
Probe_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) -Wl,--export-dynamic

Probe: $(Probe_OBJS)
	$(CC) -o Probe $(Probe_OBJS) $(Probe_LDFLAGS)

DaMon_OBJS = damon.o
DaMon_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
DaMon_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

DaMon: $(DaMon_OBJS)
	$(CC) -o DaMon $(DaMon_OBJS) $(DaMon_LDFLAGS)

probe.o: probe.c ../data/Probe.h ../config.h
	$(CC) $(Probe_CFLAGS) -c probe.c

damon.o: damon.c ../config.h
	$(CC) $(DaMon_CFLAGS) -c damon.c

clean:
	$(RM) -- $(../data/Probe.h_OBJS) $(Probe_OBJS) $(DaMon_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- Probe $(DESTDIR)$(BINDIR)/Probe
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- DaMon $(DESTDIR)$(BINDIR)/DaMon

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/Probe
	$(RM) -- $(DESTDIR)$(BINDIR)/DaMon

.PHONY: all clean distclean install uninstall
