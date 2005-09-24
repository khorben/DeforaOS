TARGETS	= Probe DaMon
CFLAGSF	= -W -Wall -ansi -I /System/Include
CFLAGS	= -g
LDFLAGSF= -L /System/Libraries -l System -l dl -rdynamic
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

Probe_OBJS= probe.o
Probe_CFLAGS=$(CFLAGSF) $(CFLAGS)
Probe: $(Probe_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Probe $(Probe_OBJS)

probe.o: probe.c
	$(CC) $(Probe_CFLAGS) -c probe.c

DaMon_OBJS= damon.o
DaMon_CFLAGS=$(CFLAGSF) $(CFLAGS)
DaMon: $(DaMon_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o DaMon $(DaMon_OBJS)

damon.o: damon.c
	$(CC) $(DaMon_CFLAGS) -c damon.c


clean:
	$(RM) $(Probe_OBJS) $(DaMon_OBJS)

distclean: clean
	$(RM) $(TARGETS)
