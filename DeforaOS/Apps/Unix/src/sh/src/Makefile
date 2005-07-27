TARGETS	= sh
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

sh_OBJS= token.o scanner.o parser.o builtin.o sh.o
sh_CFLAGS=$(CFLAGSF) $(CFLAGS)
sh: $(sh_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o sh $(sh_OBJS)

token.o: token.c
	$(CC) $(sh_CFLAGS) -c token.c

scanner.o: scanner.c
	$(CC) $(sh_CFLAGS) -c scanner.c

parser.o: parser.c
	$(CC) $(sh_CFLAGS) -c parser.c

builtin.o: builtin.c
	$(CC) $(sh_CFLAGS) -c builtin.c

sh.o: sh.c
	$(CC) $(sh_CFLAGS) -c sh.c


clean:
	$(RM) $(sh_OBJS)

distclean: clean
	$(RM) $(TARGETS)
