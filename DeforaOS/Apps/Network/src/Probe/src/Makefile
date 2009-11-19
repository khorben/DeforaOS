TARGETS	= Probe DaMon
PREFIX	= /usr/local
DESTDIR	= 
BINDIR	= $(PREFIX)/bin
CC	= cc
CPPFLAGSF= -I ../data
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= -lSystem 
LDFLAGS	= -L $(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -p
INSTALL	= install


all: $(TARGETS)

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

probe.o: probe.c ../data/Probe.h
	$(CC) $(Probe_CFLAGS) -c probe.c

damon.o: damon.c
	$(CC) $(DaMon_CFLAGS) -c damon.c

clean:
	$(RM) $(Probe_OBJS) $(DaMon_OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
