TARGETS	= sh
CFLAGSF	= -W -Wall -ansi
CFLAGS	= -g -DDEBUG
LDFLAGSF=
LDFLAGS	=
CC	= cc
RM	= rm -f


all: $(TARGETS)

SH_OBJS	= grammar.o parser.o scanner.o sh.o token.o tokencode.o
sh: $(SH_OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $@ $(SH_OBJS)

grammar.o: grammar.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c grammar.c

parser.o: parser.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c parser.c

scanner.o: scanner.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c scanner.c

sh.o: sh.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c sh.c

token.o: token.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c token.c

tokencode.o: tokencode.c
	$(CC) $(CFLAGSF) $(CFLAGS) -c tokencode.c


clean:
	$(RM) $(SH_OBJS)

distclean: clean
	$(RM) $(TARGETS)
