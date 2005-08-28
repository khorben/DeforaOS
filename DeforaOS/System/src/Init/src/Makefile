TARGETS	= Init
CFLAGSF	= -W -Wall -ansi -I /System/Include
CFLAGS	= -g
LDFLAGSF= -L /System/Libraries -l System
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

Init_OBJS= init.o
Init_CFLAGS=$(CFLAGSF) $(CFLAGS)
Init: $(Init_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Init $(Init_OBJS)

init.o: init.c
	$(CC) $(Init_CFLAGS) -c init.c


clean:
	$(RM) $(Init_OBJS)

distclean: clean
	$(RM) $(TARGETS)
