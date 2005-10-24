TARGETS	= GServer
CC	= cc
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
RM	= rm -f


all: $(TARGETS)

GServer_OBJS = gserver.o
GServer_CFLAGS = $(CFLAGSF) $(CFLAGS)
GServer: $(GServer_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o GServer $(GServer_OBJS)

gserver.o: gserver.c
	$(CC) $(GServer_CFLAGS) -c gserver.c

clean:
	$(RM) $(GServer_OBJS)

distclean: clean
	$(RM) $(TARGETS)
