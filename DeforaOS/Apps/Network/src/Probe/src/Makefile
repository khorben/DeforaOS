TARGETS	= Probe DaMon
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	= cc
CFLAGSF	= -W -Wall -ansi -I /System/Include
CFLAGS	= -g
LDFLAGSF= -L /System/Libraries -l System -l dl -rdynamic
RM	= rm -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

Probe_OBJS = probe.o
Probe_CFLAGS = $(CFLAGSF) $(CFLAGS)
Probe: $(Probe_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Probe $(Probe_OBJS)

DaMon_OBJS = damon.o
DaMon_CFLAGS = $(CFLAGSF) $(CFLAGS)
DaMon: $(DaMon_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o DaMon $(DaMon_OBJS)

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
