TARGETS	= sh
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g
CC	= cc
AR	= ar rc
RANLIB	= ranlib
LD	= ld -shared
RM	= rm -f


all: $(TARGETS)

sh_OBJS= grammar.o parser.o scanner.o sh.o token.o tokencode.o
sh_CFLAGS=$(CFLAGSF) $(CFLAGS)
sh: $(sh_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o sh $(sh_OBJS)

grammar.o: grammar.c
	$(CC) $(sh_CFLAGS) -c grammar.c

parser.o: parser.c
	$(CC) $(sh_CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(sh_CFLAGS) -c scanner.c

sh.o: sh.c
	$(CC) $(sh_CFLAGS) -c sh.c

token.o: token.c
	$(CC) $(sh_CFLAGS) -c token.c

tokencode.o: tokencode.c
	$(CC) $(sh_CFLAGS) -c tokencode.c


clean:
	$(RM) $(sh_OBJS)

distclean: clean
	$(RM) $(TARGETS)
