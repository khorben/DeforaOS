TARGETS	= init
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

init_OBJS= init.o
init_CFLAGS=$(CFLAGSF) $(CFLAGS)
init: $(init_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o init $(init_OBJS)

init.o: init.c
	$(CC) $(init_CFLAGS) -c init.c


clean:
	$(RM) $(init_OBJS)

distclean: clean
	$(RM) $(TARGETS)
