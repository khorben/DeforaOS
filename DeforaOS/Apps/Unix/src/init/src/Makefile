TARGETS	= init
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g -D_GNU_SOURCE -DDEBUG
LDFLAGSF= -lutils
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

init: init.o
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $@ init.o


init.o: init.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c init.c


clean:
	$(RM) init.o

distclean: clean
	$(RM) $(TARGETS)
