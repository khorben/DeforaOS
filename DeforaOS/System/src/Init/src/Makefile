TARGETS	= Init Hello
CFLAGSF	= -W -Wall -ansi -I /System/Include
CFLAGS	= -g
LDFLAGSF= -L /System/Libraries -l System -rdynamic
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

Hello_OBJS= hello.o
Hello_CFLAGS=$(CFLAGSF) $(CFLAGS)
Hello: $(Hello_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Hello $(Hello_OBJS)

hello.o: hello.c
	$(CC) $(Hello_CFLAGS) -c hello.c


clean:
	$(RM) $(Init_OBJS) $(Hello_OBJS)

distclean: clean
	$(RM) $(TARGETS)
