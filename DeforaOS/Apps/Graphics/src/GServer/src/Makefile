TARGETS	= GServer
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g -DDEBUG
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

GServer_OBJS= gserver.o
GServer: $(GServer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $@ $(GServer_OBJS)

gserver.o: gserver.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c gserver.c


clean:
	$(RM) $(GServer_OBJS)

distclean: clean
	$(RM) $(TARGETS)
